#ifndef __PIPE_H__
#define __PIPE_H__

/**
 * @file pipe.h
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 03/11/2022
 * @brief image processing parts for the source files and the masks.
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
#include <map>
#include "rots.hpp"
#include "data.hpp"
#include "options.hpp"
#include "image.hpp"

typedef struct {
    ROI roi;
    glm::quat rot;
} Transform;

imagine::ImageF32L3D ProcessPipe(imagine::ImageU16L3D const &image_in, bool autoback, float noise, bool deconv, const std::string &psf_path, int deconv_rounds);
int TiffToFits(const Options &options, const Transform &master_t, const std::vector<Transform> &transforms, std::string &tiff_path, int image_idx);
bool ProcessMask(Options &options, std::string &tiff_path, std::string &log_path, std::string &coord_path, int image_idx, Transform &master_t, std::vector<Transform> &transforms);

#endif