
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
#include <masamune/masamune_prog.hpp>
#include <masamune/util/string.hpp>
#include <masamune/util/file.hpp>
#include <masamune/image/tiff.hpp>
#include <masamune/image/basic.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <time.h>
#include <cstdlib>
#include <thread>
#include <math.h>
#include <cmath>
#include <glm/vec4.hpp>
#include "multiset.hpp"

masamune::vkn::ImageU16L3D Augment(masamune::vkn::ImageU16L3D &image, glm::quat rot, float zscale);
masamune::vkn::ImageU8L3D Augment(masamune::vkn::ImageU8L3D &image, glm::quat rot, float zscale);
glm::quat RandRot();

#endif