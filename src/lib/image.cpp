/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file image.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/02/2022
 * @brief Useful image functions.
 * 
 *
 */

#include "image.hpp"

using namespace imagine;


/**
 * Create a mask stack
 */
ImageU8L3D StackMask(ImageU16L &image_in, size_t width, size_t height, size_t stacksize) {
    ImageU8L3D image_out(width, height, stacksize);

    for (uint32_t d = 0; d < image_out.depth; d++) {
        
        for (uint32_t y = 0; y < image_out.height; y++) {

            for (uint32_t x = 0; x < image_out.width; x++) {
                size_t channel = d * image_out.height;
                uint16_t val = image_in.data[channel + y][x];
                
                if (val != 0) {
                    image_out.data[d][y][x] = val;
                }
            }
        }
    }
    return image_out;
}


/**
 * Check the area id against the neuron list, setting it to what it claims to be.
 * Look at one channel only though, top or bottom
 */
bool SetNeuron(ImageU16L &image_in, ImageU8L3D &image_out, std::vector<std::vector<size_t>> &neurons, int neuron_id, bool flip_depth, int id_to_write) {
    bool neuron_set = false;

    for (uint32_t d = 0; d < image_out.depth; d++) {

        for (uint32_t y = 0; y < image_out.height; y++) {

            for (uint32_t x = 0; x < image_out.width; x++) {

                size_t channel = d * image_out.height;
                uint16_t val = image_in.data[channel + y][x];
                
                if (val != 0) {
                    std::vector<size_t>::iterator it = std::find(neurons[neuron_id].begin(),
                        neurons[neuron_id].end(), static_cast<size_t>(val));
                    if (it != neurons[neuron_id].end()) {
                        neuron_set = true;

                        if (flip_depth) {
                            image_out.data[image_out.depth - d - 1][image_out.height - y - 1][x] = static_cast<uint8_t>(id_to_write);
                        } else {
                            image_out.data[d][image_out.height - y - 1][x] = static_cast<uint8_t>(id_to_write);
                        }
                    } 
                }
            }
        }
    }
    return neuron_set;
}

/**
 * Given a 3D image, flatten it.
 * 
 * @param mask - a 3D image
 * 
 * @return a 2D vkn image
 */
ImageU8L Flatten(ImageU8L3D &mask) {
    ImageU8L flat (mask.width, mask.height);

    for (uint32_t d = 0; d < mask.depth; d++) {
        for (uint32_t y = 0; y < flat.height; y++) {
            for (uint32_t x = 0; x < flat.width; x++) {
                uint16_t val = mask.data[d][y][x];
                uint16_t ext = flat.data[y][x];
                if (ext == 0) {
                    flat.data[y][x] = val;
                }
            }
        }
    }
    return flat;
}


/**
 * Given a 3D image, flatten it.
 * 
 * @param mask - a 3D image
 * 
 * @return a 2D vkn image
 */
ImageF32L Flatten(ImageF32L3D &image) {
    ImageF32L flat(image.width, image.height);

    for (uint32_t d = 0; d < image.depth; d++) {
        for (uint32_t y = 0; y < flat.height; y++) {
            for (uint32_t x = 0; x < flat.width; x++) {
                uint16_t val = image.data[d][y][x];
                uint16_t ext = flat.data[y][x];
                if (ext == 0) {
                    flat.data[y][x] = val;
                }
            }
        }
    }
    return flat;
}


/**
 * Return false if all elements are zero
 *
 * @param image - ImageU8L3D
 *
 * @return bool
 */

bool non_zero(ImageU8L3D &image) {

    for (uint32_t d = 0; d < image.depth; d++) {
        for (uint32_t y = 0; y < image.height; y++) {
            for (uint32_t x = 0; x < image.width; x++) {
                uint16_t val = image.data[d][y][x];
       
                if (val != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}