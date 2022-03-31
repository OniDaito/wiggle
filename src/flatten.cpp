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
#include <algorithm>
#include <libsee/string.hpp>
#include <libsee/file.hpp>
#include <imagine/imagine.hpp>

using namespace imagine;

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    size_t num_layers = 51;
    bool max_intensity = false;
    bool flatten = false;
    bool rename = false;
    bool bottom = false;
    int channels = 2; // 2 Channels initially in these images
    int offset_number = 0;
    int width = 640; // The desired dimensions
    int height = 300;
} Options;


/**
 * Given a tiff file, return the maximum intensity projection
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 *
 * @return bool if success or not
 */

bool MaximumIntensity(Options &options, std::string &tiff_path) {
    static int idx = 0;
    ImageU16L image = LoadTiff<ImageU16L>(tiff_path);
    ImageU16L flattened (image.width, image.height / (options.num_layers * options.channels));
    uint coff = 0;

    if (options.bottom) {
        coff = flattened.height;
    }

    for (uint32_t d = 0; d < options.num_layers; d++) {

        for (uint32_t y = 0; y < flattened.height; y++) {

            for (uint32_t x = 0; x < flattened.width; x++) {
                uint16_t val = image.data[(d * flattened.height * options.channels) + coff + y][x];
                uint16_t ext = flattened.data[y][x];
                if (val > ext){
                    flattened.data[y][x] = val;
                }
            }
        }
    }

    std::vector<std::string> tokens_log = libsee::SplitStringChars(libsee::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = libsee::StringRemove(image_id, "0xAutoStack");
    std::string output_path_png = options.output_path + "/" + image_id + "_mip.png";

    if (options.rename == true) {
        image_id  = libsee::IntToStringLeadingZeroes(options.offset_number + idx, 5);
        output_path_png = options.output_path + "/" + image_id + "_mip.png";
        std::cout << "Renaming " << tiff_path << " to " << output_path_png << std::endl;
    }

    // std::string output_path = options.output_path + "/" + image_id + "_mip.tiff";
    // SaveTiff(output_path, flattened);
    if (flattened.width != options.width || flattened.height != options.height) {
        std::cout << "Resizing " << output_path_png << std::endl;
        Resize(flattened, options.width, options.height);
    }

    Save(output_path_png, flattened);
    idx += 1;
    return true;
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

bool TiffToLayers(Options &options, std::string &tiff_path) {
    ImageU16L image = LoadTiff<ImageU16L>(tiff_path);
    ImageU16L3D flattened(image.width, image.height / (flattened.depth * options.channels), options.num_layers);
    uint coff = 0;

    if (options.bottom) {
        coff = flattened.height;
    }

    for (uint32_t d = 0; d < flattened.depth; d++) {

        for (uint32_t y = 0; y < flattened.height; y++) {

            for (uint32_t x = 0; x < flattened.width; x++) {
                uint16_t val = image.data[(d * flattened.height * options.channels) + coff + y][x];
                flattened.data[d][y][x] = val;
            }
        }
    }

    std::vector<std::string> tokens_log = libsee::SplitStringChars(libsee::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = libsee::StringRemove(image_id, "0xAutoStack");
    std::string output_path = options.output_path + "/" + image_id + "_layered.tiff";
    SaveTiff(output_path, flattened);

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

    while ((c = getopt_long(argc, (char **)argv, "i:o:l:mrn:bw:h:?", long_options, &option_index)) != -1) {
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
                options.num_layers = libsee::FromString<uint32_t>(std::string(optarg));
                break;
            case 'm' :
                options.max_intensity = true;
                break;
            case 'r' :
                options.rename = true;
                break;
            case 'n':
                options.offset_number = libsee::FromString<int>(optarg);
                break;
            case 'b':
                options.bottom = true;
                break;
            case 'w':
                options.width = libsee::FromString<int>(optarg);
                break;
            case 'h':
                options.height = libsee::FromString<int>(optarg);
                break;
        }
    }
   
    //return EXIT_FAILURE;
    std::cout << "Loading images from " << options.image_path << std::endl;

    std::cout << "Options: max intensity " << options.max_intensity << ", bottom: " << options.bottom << " width: " << options.width
        << " height: " << options.height << " Z layers: " << options.num_layers << std::endl;
    // Browse the directory looking for files
    std::vector<std::string> files = libsee::ListFiles(options.image_path);
    std::vector<std::string> tiff_files;

    for (std::string filename : files) {
        if (libsee::StringContains(filename, ".tif") && libsee::StringContains(filename, "AutoStack")) {
            tiff_files.push_back(filename);
        }
    }

      // We use a sort based on the last ID number - keeps it inline with the flatten program
    struct {
        bool operator()(std::string a, std::string b) const {
            std::vector<std::string> tokens1 = libsee::SplitStringChars(libsee::FilenameFromPath(a), "_.-");
            int idx = 0;
            for (std::string t : tokens1) {
                if (libsee::StringContains(t, "AutoStack")){
                    break;
                }
                idx += 1;
            }
            int ida = libsee::FromString<int>(libsee::StringRemove(tokens1[idx], "0xAutoStack"));
            std::vector<std::string> tokens2 = libsee::SplitStringChars(libsee::FilenameFromPath(b), "_.-");
            int idb = libsee::FromString<int>(libsee::StringRemove(tokens2[idx], "0xAutoStack"));
            return ida < idb;
        }
    } SortOrder;

    std::sort(tiff_files.begin(), tiff_files.end(), SortOrder);

    // Apparently we are missing one for some reason. Oh well
    // assert(tiff_files.size() == log_files.size());
    for (std::string tiff : tiff_files) {
        // Bottom channel
        if (options.max_intensity) {
            std::cout << "MIP-ing: " << tiff << std::endl;
            MaximumIntensity(options, tiff);
        } else {
            std::cout << "Flattening: " << tiff << std::endl;
            TiffToLayers(options, tiff);
        }
    }

    return EXIT_SUCCESS;

}