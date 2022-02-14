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
#include <masamune/masamune_prog.h>
#include <masamune/util/string.h>
#include <masamune/util/file.h>
#include <masamune/image/tiff.h>
#include <masamune/image/basic.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdlib>
#include <thread>

typedef struct {
    size_t x;
    size_t y;
    size_t z;
    double sum;
} ROI;

ROI FindROI(masamune::vkn::ImageU16L3D &input, size_t width, size_t height, size_t depth);

#endif