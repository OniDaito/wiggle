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
    std::string image_path = ".";       // Path to the source images
    std::string output_path = ".";      // Output path
    std::string annotation_path = ".";  // Path to the annotations
    std::string prefix = "";            // Prefix for the new files if renamed
    std::string psf_path = "./images/PSF_born_wolf_3d.tif"; // The path to the deconvolution kernel.
    std::string base_path = "";         // Used in stack/mask as the part to replace
    bool noprocess = false;
    bool noroi = false;
    bool rename = false;            // Rename the files
    bool flatten = false;           // Flatten and create 2D images, instead of 3D
    bool threeclass = false;        // Forget 1 and 2 and just go with ASI, ASJ or background.
    int offset_number = 0;          // Offset to start the new filenames
    bool subpixel = true;           // Use Subpixel sampling in augmentation
    bool interz = true;             // Interpolate along Z in augmentation
    bool otsu = false;              // Use Otsu's method to process the images instead of normal pipeline
    bool bottom = true;             // Are we looking at the bottom channels of the two? (always true in our data at the moment)
    bool drop_last = false;         // Drop the last Z slice to make an even stack
    bool autoback = false;          // Automatic background detection
    bool safeaug = false;           // Safe aug moves the image slightly when cropping
    bool deconv = false;            // Do we deconvolve?
    bool max_intensity = false;     // If flattening, use max intensity
    bool contrast = false;          // Apply a contrast function
    int channels = 2;               // 2 Channels initially in these images
    int stacksize = 51;             // How many stacks in our input 2D image
    int final_depth = 51;           // number of z-slices - TODO - should be set automatically along with width and height
    int final_width = 200;          // The input dimensions of each slice
    int final_height = 200;
    int deconv_rounds = 5; 
    size_t roi_xy = 200;            // Square across this dimension
    size_t roi_depth = 51;          // Default is to keep the same depth in the ROI
    uint16_t cutoff = 270;          // Background value
    float depth_scale = 6.2;        // Ratio of Z/Depth to XY
    int num_augs = 1;
} Options;


#endif