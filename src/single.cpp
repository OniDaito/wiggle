/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file single.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/02/2022
 * @brief Process a single pair of images.
 * 
 *
 */

#include <libcee/string.hpp>
#include <libcee/file.hpp>
#include <libcee/threadpool.hpp>
#include <imagine/imagine.hpp>
#include "volume.hpp"
#include "image.hpp"
#include "data.hpp"
#include "rots.hpp"

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string annotation_path = ".";
    std::string watershed_path = ".";
    std::string prefix = "";
    bool rename = false;
    bool threeclass = false;    // Forget 1 and 2 and just go with ASI, ASJ or background.
    int offset_number = 0;
    bool bottom = true;
    int channels = 2;           // 2 Channels initially in these images
    int depth = 51;             // number of z-slices - TODO - should be set automatically along with width and height
    int width = 640;            // The desired dimensions
    int height = 300;
    int stacksize = 51;         // How many stacks in our input 2D image
    size_t roi_xy = 128;           // Square across this dimension
    size_t roi_depth = 21;         // Multiplied by the depth scale later
    uint16_t cutoff = 270;
    float depth_scale = 6.2;    // Ratio of Z/Depth to XY
    int num_augs = 1;
} Options;

using namespace imagine;

// A fixed set of augmentation directions
std::vector<glm::quat> ROTS;

/**
 * Given a tiff file return the same file but with one channel and 
 * as a series of layers. Save a FITS and a JPG.
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 *
 * @return bool if success or not
 */

bool TiffToFits(Options &options, std::string &tiff_path, int image_idx, ROI &roi) {
    ImageU16L image = LoadTiff<ImageU16L>(tiff_path);
    ImageU16L3D stacked(image.width, image.height / (options.stacksize * options.channels), options.stacksize);
    uint coff = 0;
    stacked.depth -= 1;
    stacked.data.pop_back();

    if (options.bottom) {
        coff = stacked.height;
    }

    for (uint32_t d = 0; d < stacked.depth; d++) {
        for (uint32_t y = 0; y < stacked.height; y++) {
            // To get the FITS to match, we have to flip/mirror in the Y axis, unlike for PNG flatten.
            for (uint32_t x = 0; x < stacked.width; x++) {
                uint16_t val = image.data[(d * stacked.height * options.channels) + coff + y][x];
                // val = std::max(val - options.cutoff, 0);
                stacked.data[d][stacked.height - y - 1][x] = val;
                //stacked.data[d][y][x] = val;
            }
        }
    }

    std::string output_path = options.output_path + "/mipped.fits";
    SaveFITS(output_path, stacked);
    ImageU8L jpeged = Convert<ImageU8L>(Project(stacked, ProjectionType::MAX_INTENSITY));
    std::string output_path_jpg = options.output_path + "/mipped.jpg";
    SaveJPG(output_path_jpg, jpeged);
    return true;
}

/**
 * Given a tiff file and a log file, create a set of 
 * images for each neuron we are interested in.
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 * @param log_path - the file path to the corresponding .log file
 *
 * @return bool if success or not
 */

bool ProcessMask(Options &options, std::string &tiff_path, std::string &log_path, int image_idx, ROI &roi) {
    std::vector<std::string> lines = libcee::ReadFileLines(log_path);
    ImageU16L image_in = LoadTiff<ImageU16L>(tiff_path);
    size_t idx = 0;
    std::vector<std::vector<size_t>> neurons; // 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2

    // Read the log file and extract the neuron numbers
    for (int i = 0; i < 5; i++) {
        neurons.push_back(std::vector<size_t>());
    }

    for (std::string line : lines) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        if (libcee::ToLower(tokens[0]) == "associate") {
            idx += 1;
        } else {
            neurons[idx].push_back(libcee::FromString<size_t>(tokens[0]));
        }
    }

    // Join all our neurons
    ImageU8L3D neuron_mask(image_in.width, image_in.height / options.depth, options.stacksize);
    bool n1 = false, n2 = false, n3 = false, n4 = false;

    n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, true, 1);
    n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, true, 2);
    n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, true, 3);
    n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, true, 4);

    if (!n1 && !n2 && !n3 && !n4) {
        std::cout << "Failed to find neurons in mask." << std::endl;
        return false;
    }

  
    std::string output_path = options.output_path  + "/mask.fits";
    SaveFITS(output_path , neuron_mask);

    ImageU8L jpeged = Convert<ImageU8L>(Convert<ImageF32L>(Project(neuron_mask, ProjectionType::MAX_INTENSITY)));
    uint8_t min, max;
    MinMax(jpeged, min, max); 
    std::cout << "Max Val JPG: " << static_cast<int>(min) << ", " << static_cast<int>(max) << std::endl; 

    std::string output_path_jpg = options.output_path + "/mask_mipped.jpg";
    SaveJPG(output_path_jpg, jpeged);

    return true;
}


int main (int argc, char ** argv) {
    // Parse command line options
    Options options;
    int c;
    static struct option long_options[] = {
        {"image-path", 1, 0, 0},
        {"output-path", 1, 0, 0},
        {"prefix", 1, 0, 0},
        {NULL, 0, NULL, 0}
    };

    int option_index = 0;
    int image_idx = 0;

    while ((c = getopt_long(argc, (char **)argv, "i:o:a:w:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                options.image_path = std::string(optarg);
                break;
            case 'w' :
                options.watershed_path = std::string(optarg);
                break;
            case 'a' :
                options.annotation_path = std::string(optarg);
                break;
            case 'o' :
                options.output_path = std::string(optarg);
                break;

        }
    }

    ROI roi;
    ProcessMask(options, options.watershed_path, options.annotation_path, 0, roi);
    TiffToFits(options, options.image_path, 0, roi);
    
    return EXIT_SUCCESS;

}