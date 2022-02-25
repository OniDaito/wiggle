#ifndef __DATA_H__
#define __DATA_H__

/**
 * @file data.h
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/02/2022
 * @brief Common data related functions for Wiggle
 *
 */

#include <getopt.h>
#include <fitsio.h>
#include <masamune/masamune_prog.hpp>
#include <masamune/util/string.hpp>
#include <masamune/util/file.hpp>
#include <masamune/image/tiff.hpp>
#include <masamune/image/basic.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdlib>
#include <thread>

int GetOffetNumber(std::string output_path);
std::vector<std::string> FindLogFiles(std::string annotation_path);
std::vector<std::string> FindAnnotations(std::string annotation_path);
std::vector<std::string> FindInputFiles(std::string image_path);

#endif