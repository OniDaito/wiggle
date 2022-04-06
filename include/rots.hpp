
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
#include <libsee/string.hpp>
#include <libsee/file.hpp>
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
T Augment(T const &image, glm::quat rot, size_t cube_dim, float zscale) {
    assert(image.width == image.height);
    assert(cube_dim < image.width);
    assert(cube_dim / zscale < image.depth);

    T resampled(image.width, image.height, image.width);

    // Essentially, we want a cube, smaller than the input image.
    // Z is a special case and requires scaling.
    T augmented(cube_dim, cube_dim, cube_dim);

    float aug_ratio = static_cast<float>(cube_dim) / static_cast<float>(image.width);

    // Create our sampler
    for (size_t z = 0; z < resampled.depth; z++) {
        for (size_t y = 0; y < resampled.height; y++) {
            for (size_t x = 0; x < resampled.width; x++) {
                size_t sample_z = static_cast<size_t>(floor(static_cast<float>(z) / zscale));
                resampled.data[z][y][x] = image.data[sample_z][y][x];
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

                augmented.data[z][y][x] = val;
            }
        }
    }

    return augmented;
}

glm::quat RandRot();

#endif