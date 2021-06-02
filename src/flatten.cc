/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file flatten.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 01/06/2021
 * @brief Given QueeLim style tiff stacks, flatten them
 * 
 */

#include <getopt.h>
#include <masamune/masamune_prog.h>
#include <masamune/util/string.h>
#include <masamune/util/file.h>
#include <masamune/image/tiff.h>

using namespace masamune;

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    size_t num_layers = 51;
    bool max_intensity = false;
} Options;


/**
 * Given a tiff file, return the maximum intensity projection
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 * @param top_channel - are we spitting out the top channel or bottom one?
 *
 * @return bool if success or not
 */

bool MaximumIntensity(Options &options, std::string &tiff_path, bool top_channel) {
    vkn::ImageU16L image;
    vkn::ImageU16L flattened;
    image::LoadTiff<vkn::ImageU16L>(tiff_path, image);
    flattened.width = image.width;
    flattened.height = image.height / (options.num_layers * 2); // Two channels
    vkn::Alloc(flattened);

    uint coff = 0;

    if (!top_channel) {
        coff = flattened.height;
    }

    for (uint32_t d = 0; d < options.num_layers; d++) {

        for (uint32_t y = 0; y < flattened.height; y++) {

            for (uint32_t x = 0; x < flattened.width; x++) {
                uint16_t val = image.image_data[(d * flattened.height * 2) + coff + y][x];
                uint16_t ext = flattened.image_data[y][x];
                if (val > ext){
                    flattened.image_data[y][x] = val;
                }
            }
        }
    }

    std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = util::StringRemove(image_id, "0xAutoStack");
    std::string output_path = options.output_path + "/" + image_id + "_mip.tiff";
    image::SaveTiff(output_path, flattened);

    return true;
}

/**
 * Given a tiff file return the same file but with one channel and 
 * as a series of layers
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 * @param top_channel - are we spitting out the top channel or bottom one?
 *
 * @return bool if success or not
 */

bool TiffToLayers(Options &options, std::string &tiff_path, bool top_channel) {
    vkn::ImageU16L image;
    vkn::ImageU16L3D flattened;
    image::LoadTiff<vkn::ImageU16L>(tiff_path, image);
    flattened.width = image.width;
    flattened.depth = options.num_layers;
    flattened.height = image.height / (flattened.depth * 2); // Two channels
    vkn::Alloc(flattened);

    uint coff = 0;
    if (!top_channel) {
        coff = flattened.height;
    }

    for (uint32_t d = 0; d < flattened.depth; d++) {

        for (uint32_t y = 0; y < flattened.height; y++) {

            for (uint32_t x = 0; x < flattened.width; x++) {
                uint16_t val = image.image_data[(d * flattened.height * 2) + coff + y][x];
                flattened.image_data[d][y][x] = val;
            }
        }
    }

    std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = util::StringRemove(image_id, "0xAutoStack");
    std::string output_path = options.output_path + "/" + image_id + "_flat.tiff";
    image::SaveTiff(output_path, flattened);

    return true;
}

int main (int argc, char ** argv) {
    // Parse command line options
    Options options;
    int c;
    static struct option long_options[] = {
        {"image-path", 1, 0, 0},
        {"output-path", 1, 0, 0},
        {NULL, 0, NULL, 0}
    };

    int option_index = 0;

    while ((c = getopt_long(argc, (char **)argv, "i:o:l:m?", long_options, &option_index)) != -1) {
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
                options.num_layers = util::FromString<uint32_t>(std::string(optarg));
                break;
            case 'm' :
                options.max_intensity = true;
                break;
        }
    }
   
    //return EXIT_FAILURE;
    std::cout<< "Loading images from " << options.image_path << std::endl;

    // Browse the directory looking for files
    std::vector<std::string> files = util::ListFiles(options.image_path);
    std::vector<std::string> tiff_files;

    for (std::string filename : files) {
        if (util::StringContains(filename, ".tif") && util::StringContains(filename, "AutoStack")) {
            std::cout << filename << std::endl;
            tiff_files.push_back(filename);
        }
    }

    // Apparently we are missing one for some reason. Oh well
    // assert(tiff_files.size() == log_files.size());
    for (std::string tiff : tiff_files) {
        // Bottom channel
        if (options.max_intensity) {
            std::cout << "MIP-ing: " << tiff << std::endl;
            MaximumIntensity(options, tiff, false);
        } else {
            std::cout << "Flattening: " << tiff << std::endl;
            TiffToLayers(options, tiff, false);
        }
    }

    return EXIT_SUCCESS;

}