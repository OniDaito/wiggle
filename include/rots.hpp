
#ifndef __ROTS_H__
#define __ROTS_H__

/**
 * @file volume.h
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/02/2022
 * @brief The volume header for the volume program
 *
 */

#include <getopt.h>
#include <fitsio.h>
#include <libcee/string.hpp>
#include <libcee/file.hpp>
#include <imagine/imagine.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <time.h>
#include <cstdlib>
#include <random>
#include <thread>
#include <math.h>
#include <cmath>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "roi.hpp"

extern std::default_random_engine RANDROT_GENERATOR;

/**
 * Augment the input images 
 * 
 * @param image - the starting image
 * @param rot - a random rotation
 * @param zscale - the scale on Z depth
 * @param final_xy - the final width and height of the image
 * @param final_depth - the final depth of the image
 * @return the augmented image
 * 
 * Augmentation works by rotating the volume around the origin. We scale 
 * the volume based on the image dimensions and the zscale (as z pixels 
 * often count for 6 times x and y), then we rotate and scale back.
 * 
 * The final image is smaller than the input as we rotate a bigger
 * volume in order to get a more complete rotated final image.
 * 
 */


template<typename T>
T Augment(T const &image, glm::quat rot, size_t cube_dim, float zscale, bool subpixel, bool iterz) {
    assert(image.width == image.height);
    assert(cube_dim < image.width);
    assert(cube_dim / zscale < image.depth);

    T resampled(image.width, image.width, image.width);

    // Essentially, we want a cube, smaller than the input image.
    // Z is a special case and requires scaling.
    T augmented(cube_dim, cube_dim, cube_dim);

    float aug_ratio = static_cast<float>(cube_dim) / static_cast<float>(image.width);

    // Create our sampler
    for (size_t z = 0; z < resampled.depth; z++) {
        for (size_t y = 0; y < resampled.height; y++) {
            for (size_t x = 0; x < resampled.width; x++) {
                size_t it =  static_cast<size_t>(floor(static_cast<float>(z) / zscale));

                if (iterz){
                    float tb = floor(static_cast<float>(z) / zscale);
                    float tt = ceil(static_cast<float>(z) / zscale);
                    float tv = static_cast<float>(z) / zscale;
                    size_t ic = it + 1;
                    
                    if (ic >= image.depth) {
                         ic = it;
                    }
                    
                    float mix_n = 1.0 - (2.0 * (tt - tv));
                    float mix_og = (2.0 * (tt - tv));

                    if ((tt - tv) >= 0.5) {
                        // Closer to the bottom end
                        ic = it - 1;
                        
                        if (ic < 0) {
                            ic = 0;
                        }
                        
                        float mix_n = 1.0 - (2.0 * (tv - tb));
                        float mix_og = (2.0 * (tv - tb));
                    }

                    float interped = mix_og * static_cast<float>(image.data[it][y][x]) + mix_n * static_cast<float>(image.data[ic][y][x]); 
                    resampled.data[z][y][x] = interped; // TODO - this is naughty as we can't assume the float goes to the proper type of resampled

                } else {
                    resampled.data[z][y][x] = image.data[it][y][x];
                }
            }
        }
    }

    glm::mat4 rotmat = glm::toMat4(rot);
    
    // now do the sampling
    for (size_t z = 0; z < augmented.depth; z++) {
        for (size_t y = 0; y < augmented.height; y++) {
            for (size_t x = 0; x < augmented.width; x++) {
                float fx = static_cast <float>(x);
                float fy = static_cast <float>(y);
                float fz = static_cast <float>(z);

                fx = (fx / static_cast <float>(augmented.width) * 2.0) - 1.0;
                fy = (fy / static_cast <float>(augmented.height) * 2.0) - 1.0;
                fz = (fz / static_cast <float>(augmented.depth) * 2.0) - 1.0;

                glm::vec4 v = glm::vec4(fx * aug_ratio, fy * aug_ratio, fz * aug_ratio, 1.0);
                v = rotmat * v;

                // Now perform subpixel sampling
              
                float ffx = floor(v.x);
                float ffy = floor(v.y);
                float ffz = floor(v.z);

                if (subpixel) {
                    float gx = v.x - ffx;
                    float gy = v.y - ffy;
                    float gz = v.z - ffz;

                    int cx = static_cast <int>((v.x + 1.0) / 2.0 * resampled.width);
                    int cy = static_cast <int>((v.y + 1.0) / 2.0 * resampled.height);
                    int cz = static_cast <int>((v.z + 1.0) / 2.0 * resampled.depth);

                    // 27 samples so get values for all - left to right, top to bottom, front to back
                    float val = 0;

                    for (int dz = -1; dz < 2; dz++) {
                        for (int dy = -1; dy < 2; dy++) {
                            for (int dx = -1; dx < 2; dx++) {
                                
                                int rx = cx + dx;
                                int ry = cy + dy;
                                int rz = cz + dz;

                                float ddx = 0.5 + static_cast<float>(dx) - gx;
                                float ddy = 0.5 + static_cast<float>(dy) - gy;
                                float ddz = 0.5 + static_cast<float>(dz) - gz;
                                
                                float dist = sqrt(ddx * ddx + ddy * ddy + ddz * ddz);

                                if (dist < 1.0) {
                                    if (rx >= 0 && rx < resampled.width &&
                                    ry >= 0 && ry < resampled.height &&
                                    rz >= 0 && rz < resampled.depth) {
                                        val += static_cast<float>(resampled.data[rz][ry][rx]) * (1.0 - dist);
                                    }
                                }
                            }
                        }
                    }

                    augmented.data[z][y][x] = val; // TODO - are we being naughty here as val is a float and we can't be sure augmented.data is a float
                } else {
                    int cx = static_cast <int>((v.x + 1.0) / 2.0 * resampled.width);
                    int cy = static_cast <int>((v.y + 1.0) / 2.0 * resampled.height);
                    int cz = static_cast <int>((v.z + 1.0) / 2.0 * resampled.depth);
                    if (cx >= 0 && cy >= 0 && cz >= 0 
                        && cx < resampled.width && cy < resampled.height && cz < resampled.depth ) {
                        augmented.data[z][y][x] = resampled.data[cz][cy][cx];
                    }
                }
            }
        }
    }

    return augmented;
}

// Same as above but for the graph
// Returns the graph but in augmented co-ordinates to match the images

void AugmentGraph(std::vector<glm::vec4> const &graph, std::vector<glm::vec4> &rgraph, glm::quat rot, size_t image_dim, size_t final_dim, float zscale) {
    assert(graph.size() == 4);
    assert(rgraph.size() == 0);

    for (int i = 0; i < 4; i++) {
        float fz = static_cast <float>(graph[i].z * zscale);
        float fx = static_cast <float>(graph[i].x);
        float fy = static_cast <float>(graph[i].y);

        fx = (fx / static_cast <float>(image_dim) * 2.0) - 1.0;
        fy = (fy / static_cast <float>(image_dim) * 2.0) - 1.0;
        fz = (fz / static_cast <float>(image_dim) * 2.0) - 1.0;

        glm::mat4 rotmat = glm::toMat4(rot);
        glm::vec4 v = glm::vec4(fx, fy, fz, 1.0);
        v = rotmat * v;

        glm::vec4 tg ( static_cast<float>((v.x + 1.0) / 2.0 * image_dim),
            static_cast<float>((v.y + 1.0) / 2.0 * image_dim),
           static_cast<float>( (v.z + 1.0) / 2.0 * image_dim), 1.0f);

        // Augmenting cuts down the input image a bit so we must adjust 
        // ROI accordingly
        float roi_shift = ( image_dim - final_dim ) / 2.0f;
  
        tg.x = tg.x - roi_shift;
        tg.y = tg.y - roi_shift;
        tg.z = tg.z - roi_shift;

        rgraph.push_back(tg);

        // Note we don't undo Zscale, as augimage above does not flatten the Z back down either.
    }

}

glm::quat RandRot();

#endif