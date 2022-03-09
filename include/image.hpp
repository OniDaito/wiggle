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
#include <masamune/masamune_prog.hpp>
#include <masamune/util/string.hpp>
#include <masamune/util/file.hpp>
#include <masamune/image/tiff.hpp>
#include <masamune/image/basic.hpp>
#include <masamune/image/convert.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdlib>
#include <thread>

void SetNeuron(masamune::vkn::ImageU16L &image_in, masamune::vkn::ImageU8L3D &image_out,
    std::vector<std::vector<size_t>> &neurons, int neuron_id, bool flip_depth, int id_to_write);

masamune::vkn::ImageU8L Flatten(masamune::vkn::ImageU8L3D &mask);
void WriteFITS(std::string filename, masamune::vkn::ImageU16L3D flattened);
void WriteFITS(std::string filename, masamune::vkn::ImageF32L3D flattened);
void WriteFITS(std::string filename, masamune::vkn::ImageU8L3D flattened);
void WriteFITS(std::string filename, masamune::vkn::ImageU16L flattened);
void WriteFITS(std::string filename, masamune::vkn::ImageF32L flattened);
bool non_zero(masamune::vkn::ImageU8L3D &image);
void printerror( int status);

#endif