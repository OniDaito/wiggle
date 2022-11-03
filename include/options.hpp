#ifndef __OPTIONS_H__
#define __OPTIONS_H__

/**
 * @file options.h
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 03/11/2022
 * @brief The options struct
 *
 */

#include <string>

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string annotation_path = ".";
    std::string prefix = "";
    std::string output_log_path = "";
    std::string psf_path = "./images/PSF_born_wolf_3d.tif";
    bool rename = false;
    bool flatten = false;
    bool threeclass = false;    // Forget 1 and 2 and just go with ASI, ASJ or background.
    int offset_number = 0;
    bool subpixel = true;
    bool interz = true;
    bool otsu = false;
    bool bottom = false;
    bool drop_last = false;
    bool autoback = false;      // Automatic background detection
    bool deconv = false;         // Do we deconvolve and all that?
    bool max_intensity = false;    // If flattening, use max intensity
    int channels = 2;           // 2 Channels initially in these images
    int stacksize = 51;         // How many stacks in our input 2D image
    int final_depth = 51;             // number of z-slices - TODO - should be set automatically along with width and height
    int final_width = 200;            // The input dimensions of each slice
    int final_height = 200;
    int deconv_rounds = 5; 
    size_t roi_xy = 200;           // Square across this dimension
    size_t roi_depth = 21;         // Multiplied by the depth scale later
    uint16_t cutoff = 270;
    float depth_scale = 6.2;    // Ratio of Z/Depth to XY
    int num_augs = 1;
} Options;


#endif