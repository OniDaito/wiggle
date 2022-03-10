#ifndef __MULTISET_H__
#define __MULTISET_H__

/**
 * @file multiset.h
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 07/02/2021
 * @brief The multiset header - mostly used for testing.
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

typedef struct {
    size_t x;
    size_t y;
    size_t z;
    size_t xy_dim;
    size_t depth;
    double sum;
} ROI;

ROI FindROI(masamune::vkn::ImageU16L3D &input, size_t xy, size_t depth);
ROI FindROICentred(masamune::vkn::ImageU16L3D &input, size_t xy, size_t depth);

#endif