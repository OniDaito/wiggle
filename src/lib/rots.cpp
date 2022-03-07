/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file rots.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 25/02/2022
 * @brief Useful data finding functions
 * 
 *
 */

#include "rots.hpp"

using namespace masamune;

glm::quat RandRot() {
    srand(time(NULL));
    float u1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float u2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float u3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    glm::quat q = glm::quat(
            static_cast <float>(sqrt(1.0 - u1) * sin(2.0 * M_PI * u2)),
            static_cast <float>(sqrt(1.0 - u1) * cos(2.0 * M_PI * u2)),
            static_cast <float>(sqrt(u1) * sin(2.0 * M_PI * u3)),
            static_cast <float>(sqrt(u1) * cos(2.0 * M_PI * u3))
    );
    q = glm::normalize(q);
    return q;
}

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
 */

vkn::ImageU16L3D Augment(vkn::ImageU16L3D &image, uint32_t final_xy, uint32_t final_depth, glm::quat rot, float zscale) {
    vkn::ImageU16L3D augmented;
    assert(image.width == image.height);
    
    augmented.width = final_xy;
    augmented.height = final_xy;
    augmented.depth = final_depth;

    float aug_ratio_xy = augmented.width / image.width;
    float aug_ratio_z = augmented.depth / image.depth;

    vkn::Alloc(augmented);
    // We expand along the Z depth, rotate, then contract on Z, then we
    // resample and return.
    glm::mat4 rotmat = glm::toMat4(rot);
    glm::mat4 expand = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, zscale));
    glm::mat4 contract = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0 / zscale));
    glm::mat4 finalmat = contract * rotmat * expand;

    float zyratio = static_cast <float>(augmented.depth) * zscale / static_cast <float>(augmented.height);

    // now do the sampling
    for (int z = 0; z < augmented.depth; z++) {
        for (int y = 0; y < augmented.height; y++) {
            for (int x = 0; x < augmented.width; x++) {
                float fx = static_cast <float>(x);
                float fy = static_cast <float>(y);
                float fz = static_cast <float>(z);

                fx = ((fx / static_cast <float>(augmented.width) * 2.0) - 1.0);
                fy = (fy / static_cast <float>(augmented.height) * 2.0) - 1.0;
                fz = ((fz / static_cast <float>(augmented.depth) * 2.0) - 1.0) * zyratio;

                fx = fx * aug_ratio_xy;
                fy = fy * aug_ratio_xy;
                fz = fz * aug_ratio_z;

                glm::vec4 v = glm::vec4(fx, fy, fz, 1.0);
                v = finalmat * v;

                int nx = static_cast <int>(((v.x) + 1.0) / 2.0 * image.width);
                int ny = static_cast <int>((v.y + 1.0) / 2.0 * image.height);
                int nz = static_cast <int>(((v.z / zyratio) + 1.0) / 2.0 * image.depth);

                if (nx >= 0 && nx < image.width &&
                    ny >= 0 && ny < image.height &&
                    nz >= 0 && nz < image.depth) {
                    augmented.image_data[z][y][x] = image.image_data[nz][ny][nx];
                }
            }
        }
    }

    return augmented;
}

// TODO - we should template this I think
vkn::ImageU8L3D Augment(vkn::ImageU8L3D &image, glm::quat rot, float zscale) {
    vkn::ImageU8L3D augmented;
    augmented.width = image.width;
    augmented.height = image.height;
    augmented.depth = image.depth;
    vkn::Alloc(augmented);
    // We expand along the Z depth, rotate, then contract on Z, then we
    // resample and return.
    glm::mat4 rotmat = glm::toMat4(rot);
    glm::mat4 expand = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, zscale));
    glm::mat4 contract = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0 / zscale));
    glm::mat4 finalmat = contract * rotmat * expand;

    float xyratio = static_cast <float>(image.width) / static_cast <float>(image.height);
    float zyratio = static_cast <float>(image.depth) * zscale / static_cast <float>(image.height);

    // now do the sampling
    for (int z = 0; z < image.depth; z++) {
        for (int y = 0; y < image.height; y++) {
            for (int x = 0; x < image.width; x++) {
                float fx = static_cast <float>(x);
                float fy = static_cast <float>(y);
                float fz = static_cast <float>(z);

                fx = ((fx / static_cast <float>(image.width) * 2.0) - 1.0) * xyratio;
                fy = (fy / static_cast <float>(image.height) * 2.0) - 1.0;
                fz = ((fz / static_cast <float>(image.depth) * 2.0) - 1.0) * zyratio;

                glm::vec4 v = glm::vec4(fx, fy, fz, 1.0);
                v = finalmat * v;

                int nx = static_cast <int>(((v.x / xyratio) + 1.0) / 2.0 * image.width);
                int ny = static_cast <int>((v.y + 1.0) / 2.0 * image.height);
                int nz = static_cast <int>(((v.z / zyratio) + 1.0) / 2.0 * image.depth);

                if (nx >= 0 && nx < image.width &&
                    ny >= 0 && ny < image.height &&
                    nz >= 0 && nz < image.depth) {
                    augmented.image_data[z][y][x] = image.image_data[nz][ny][nx];
                }
            }
        }
    }

    return augmented;
}
