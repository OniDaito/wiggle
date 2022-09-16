#ifndef __IMAGE_H__
#define __IMAGE_H__

/**
 * @file image.h
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/02/2022
 * @brief Common image related functions for Wiggle
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
#include <cstdlib>
#include <thread>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

bool SetNeuron(imagine::ImageU16L &image_in, imagine::ImageU8L3D &image_out, std::vector<std::vector<size_t>> &neurons, int neuron_id, bool flip_depth, bool flip_height, int id_to_write);
imagine::ImageU8L3D StackMask(imagine::ImageU16L &image_in, size_t width, size_t height, size_t stacksize);
imagine::ImageU8L Flatten(imagine::ImageU8L3D &mask);
bool non_zero(imagine::ImageU8L3D &image);

#endif