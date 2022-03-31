#ifndef __ROI_H__
#define __ROI_H__

/**
 * @file multiset.h
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 07/02/2021
 * @brief The multiset header - mostly used for testing.
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

ROI FindROI(imagine::ImageU16L3D &input, size_t xy, size_t depth);
ROI FindROICentred(imagine::ImageU16L3D &input, size_t xy, size_t depth);

#endif