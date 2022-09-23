/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file stack.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 23/09/2022
 * @brief Stack a source tiff
 *
 * Given a directory, stack the source tiffs into fits 
 *
 */

#include <libcee/string.hpp>
#include <libcee/file.hpp>
#include <libcee/threadpool.hpp>
#include <imagine/imagine.hpp>
#include <glm/gtc/quaternion.hpp>
#include <sys/stat.h>
#include "volume.hpp"
#include "image.hpp"
#include "data.hpp"
#include "rots.hpp"

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string base_path = ".";
    bool rename = false;
    bool flatten = false;
    bool threeclass = false;    // Forget 1 and 2 and just go with ASI, ASJ or background.
    int offset_number = 0;
    bool bottom = false;
    bool deconv = false;         // Do we deconvolve and all that?
    bool max_intensity = false;    // If flattening, use max intensity
    int channels = 2;           // 2 Channels initially in these images
    int stacksize = 51;         // How many stacks in our input 2D image
    int final_depth = 51;             // number of z-slices - TODO - should be set automatically along with width and height
    int final_width = 640;            // The input dimensions of each slice
    int final_height = 300;

} Options;

using namespace imagine;

bool folder_exists(std::string foldername)  {
    struct stat st;
    stat(foldername.c_str(), &st);
    return st.st_mode & S_IFDIR;
}


/**
 * Given a tiff file return the same file but with one channel and 
 * as a series of layers
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 *
 * @return bool if success or not
 */

bool TiffToFits(Options &options, std::string &tiff_path) {
    ImageU16L image = LoadTiff<ImageU16L>(tiff_path); 
    ImageU16L3D stacked(image.width, (image.height / (options.stacksize * options.channels)), options.stacksize);
    uint coff = 0;

    if (options.bottom) {
        coff = stacked.height;
    }

    for (uint32_t d = 0; d < stacked.depth; d++) {
        for (uint32_t y = 0; y < stacked.height; y++) {
            for (uint32_t x = 0; x < stacked.width; x++) {
                uint16_t val = image.data[(d * stacked.height * options.channels) + coff + y][x];
                stacked.data[d][y][x] = val;
            }
        }
    }

    std::vector<std::string> end = libcee::SplitStringString(tiff_path, options.output_path);
    std::string new_path =   tiff_path.replace(0, options.base_path.length(), options.output_path);
    std::string output_path = new_path.replace(new_path.length() - 5, 5, ".fits");
    std::string new_dir = libcee::PathFromPath(output_path);

    if (!folder_exists(new_dir)) {
        mkdir(new_dir.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
   
    std::cout << "Saving new fits to " << output_path << std::endl;

    FlipVerticalI(stacked);

    if (options.final_width != stacked.width || options.final_height != stacked.height || options.final_depth != stacked.depth) {
        if (stacked.depth % 2 == 1) {
            stacked.data.pop_back();
        }
        ImageU16L3D resized = Resize(stacked, options.final_width, options.final_height, options.final_depth);
        SaveFITS(output_path, resized);
    } else {
        SaveFITS(output_path, stacked);   
    }

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

    while ((c = getopt_long(argc, (char **)argv, "i:o:l:p:tbz:w:h:s:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                options.image_path = std::string(optarg);
                break;
            case 'o' :
                options.output_path = std::string(optarg);
                break;
            case 'l' :
                options.base_path = std::string(optarg);
                break;
            case 'b':
                options.bottom = true;
                break;
            case 't' :
                options.threeclass = true;
                break;
            case 'z':
                options.final_depth = libcee::FromString<int>(optarg);
                break;
            case 'w':
                options.final_width = libcee::FromString<int>(optarg);
                break;
            case 'h':
                options.final_height = libcee::FromString<int>(optarg);
                break;
            case 's':
                options.stacksize = libcee::FromString<int>(optarg);
                break;
        }
    }

    std::vector<std::string> tiff_input_files = FindInputFiles(options.image_path);

    for (std::string tiff_input : tiff_input_files) {
   
        if (libcee::StringContains(libcee::FilenameFromPath(tiff_input), "_.-"), "AutoStack") {
            TiffToFits(options, tiff_input);
        }
                
    }

    return EXIT_SUCCESS;

}
