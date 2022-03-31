/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file dataset.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 19/05/2021
 * @brief Parse our tiff files and create our masks
 * 
 * Given a directory of worm information, create the
 * masks and anything else we need for the dataset.
 * 
 * Internally we use the following codes for the neurons
 * 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2
 *
 */

#include <getopt.h>
#include <fitsio.h>
#include <vector>
#include "data.hpp"
#include "image.hpp"

using namespace imagine;

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string prefix = "";
    bool flatten = false;
    bool rename = false;
    int offset_number = 0;
    int image_slices = 51; // number of z-slices
    int width = 640; // The desired dimensions
    int height = 300;
} Options;


/**
 * Check the area id against the neuron list, setting it to what it claims to be.
 * Look at one channel only though, top or bottom
 */
void SetNeuron(ImageU16L &image_in, ImageU8L3D &image_out,
    std::vector<std::vector<size_t>> &neurons, int neuron_id, bool flip_depth) {

    for (uint32_t d = 0; d < image_out.depth; d++) {

        for (uint32_t y = 0; y < image_out.height; y++) {

            for (uint32_t x = 0; x < image_out.width; x++) {

                size_t channel = d * image_out.height;
                uint16_t val = image_in.data[channel + y][x];
                
                if (val != 0) {
                    std::vector<size_t>::iterator it = std::find(neurons[neuron_id].begin(),
                        neurons[neuron_id].end(), static_cast<size_t>(val));
                    if (it != neurons[neuron_id].end()) {
                        uint8_t nval = neuron_id;
                        if (flip_depth) {
                            image_out.data[image_out.depth - d - 1][y][x] = val;
                        } else {
                            image_out.data[d][y][x] = val;
                        }
                    } 
                }
            }
        }
    }
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

bool ProcessTiff(Options &options, std::string &tiff_path, std::string &log_path, int image_idx) {
    std::vector<std::string> lines = libsee::ReadFileLines(log_path);
    ImageU16L image_in = LoadTiff<ImageU16L>(tiff_path);
    size_t idx = 0;
    std::vector<std::vector<size_t>> neurons; // 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2

    // Read the log file and extract the     
    for (int i = 0; i < 5; i++) {
        neurons.push_back(std::vector<size_t>());
    }

    for (std::string line : lines) {
        std::vector<std::string> tokens = libsee::SplitStringWhitespace(line);
        if (libsee::ToLower(tokens[0]) == "associate") {
            idx += 1;
        } else {
            neurons[idx].push_back(libsee::FromString<size_t>(tokens[0]));
        }
    }

    // Export ASI first. We split the tiffs so they have layers
    ImageU8L3D asi(image_in.width, image_in.height / asi.depth, options.image_slices);

    SetNeuron(image_in, asi, neurons, 1, !options.flatten);
    SetNeuron(image_in, asi, neurons, 2, !options.flatten);

    std::vector<std::string> tokens_log = libsee::SplitStringChars(libsee::FilenameFromPath(log_path), "_.");
    std::string image_id = libsee::StringRemove(tokens_log[0], "ID");

    if (options.rename == true) {
        image_id = libsee::IntToStringLeadingZeroes(image_idx, 5);
        std::cout << "Renaming " <<  tiff_path << " to " << image_id << std::endl;
    }

    std::string output_path = options.output_path + "/" + options.prefix + image_id + "_asi.fits";
    std::string output_path_png = options.output_path + "/" + options.prefix + image_id + "_asi.png";

    if (non_zero(asi)) {

        //SaveTiff(output_path, asi);
        if (options.flatten){
            ImageU8L asi_flat = Flatten(asi);
            if (asi_flat.width != options.width || asi_flat.height != options.height) {
                Resize(asi_flat, options.width, options.height);
            }
            //SaveTiff(output_path, asi_flat);
            ImageU8L asi_flip = MirrorVertical(asi_flat);
            Save(output_path_png, asi_flip);

        } else {
            // SaveTiff(output_path, asi);
            WriteFITS(output_path, asi);
        } 
    }

    // Now look at ASJ    
    ImageU8L3D asj(image_in.width, image_in.height / asj.depth, options.image_slices);
    SetNeuron(image_in, asj, neurons, 3, !options.flatten);
    SetNeuron(image_in, asj, neurons, 4, !options.flatten);

    output_path = options.output_path + "/" + options.prefix + image_id + "_asj.fits";
    output_path_png = options.output_path + "/" + options.prefix + image_id + "_asj.png";

    if (non_zero(asj)) {

        if (options.flatten){
            ImageU8L asj_flat = Flatten(asj);
            if (asj_flat.width != options.width || asj_flat.height != options.height) {
                Resize(asj_flat, options.width, options.height);
            }
            //SaveTiff(output_path, asj_flat);
            ImageU8L asj_flip = MirrorVertical(asj_flat);
            Save(output_path_png, asj_flip);
        } else {
            // SaveTiff(output_path, asj);
            WriteFITS(output_path, asj);
        }
    }
    image_idx +=1;

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

    while ((c = getopt_long(argc, (char **)argv, "i:o:p:frn:zw:h:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                options.image_path = std::string(optarg);
                break;
            case 'o' :
                options.output_path = std::string(optarg);
                break;
            case 'p' :
                options.prefix = std::string(optarg);
                break;
            case 'f' :
                options.flatten = true;
                break;
            case 'r' :
                options.rename = true;
                break;
            case 'n':
                options.offset_number = libsee::FromString<int>(optarg);
                break;
            case 'z':
                options.image_slices = libsee::FromString<int>(optarg);
                break;
            case 'w':
                options.width = libsee::FromString<int>(optarg);
                break;
            case 'h':
                options.height = libsee::FromString<int>(optarg);
                break;
        }
    }

    int image_idx = options.offset_number;
   
    //return EXIT_FAILURE;
    std::cout << "Loading images from " << options.image_path << std::endl;
    std::cout << "Offset: " << options.offset_number << ", rename: " << options.rename << ", flatten: " << options.flatten << std::endl;

    // Browse the directory looking for files
    std::vector<std::string> files = libsee::ListFiles(options.image_path);
    std::vector<std::string> tiff_files;
    std::vector<std::string> log_files;
    std::vector<std::string> dat_files;

    for (std::string filename : files) {
        std::cout << filename << std::endl;
        if (libsee::StringContains(filename, ".tif") && libsee::StringContains(filename, "ID")  && libsee::StringContains(filename, "WS")) {
            tiff_files.push_back(filename);
        }
        else if (libsee::StringContains(filename, ".dat")) {
            dat_files.push_back(filename);
        }
        else if (libsee::StringContains(filename, ".log")) {
            log_files.push_back(filename);
        }
    }

    // Apparently we are missing one for some reason. Oh well
    // assert(tiff_files.size() == log_files.size());

    // We use a sort based on the last ID number - keeps it inline with the flatten program
    struct {
        bool operator()(std::string a, std::string b) const {
            std::vector<std::string> tokens1 = libsee::SplitStringChars(libsee::FilenameFromPath(a), "_.-");
            int idx = 0;
            for (std::string t : tokens1) {
                if (libsee::StringContains(t, "ID")){
                    break;
                }
                idx += 1;
            }
            int ida = libsee::FromString<int>(libsee::StringRemove(tokens1[idx], "ID"));
            std::vector<std::string> tokens2 = libsee::SplitStringChars(libsee::FilenameFromPath(b), "_.-");
            int idb = libsee::FromString<int>(libsee::StringRemove(tokens2[idx], "ID"));
            return ida < idb;
        }
    } SortOrder;

    std::sort(tiff_files.begin(), tiff_files.end(), SortOrder);

    // Pair up the tiffs with their log file and process them.
    for (std::string tiff : tiff_files) {
        bool paired = false;
        std::vector<std::string> tokens = libsee::SplitStringChars(libsee::FilenameFromPath(tiff), "_.-");
        std::string id = tokens[0];

        for (std::string log : log_files) {
            std::vector<std::string> tokens_log = libsee::SplitStringChars(libsee::FilenameFromPath(log), "_.-");
            if (tokens_log[0] == id) {
                std::cout << "Pairing " << tiff << " with " << log << std::endl;
                paired = true;
                ProcessTiff(options, tiff, log, image_idx);
            }
        }

        if (!paired){
            std::cout << "Failed to pair " << tiff << std::endl;
        }
        // We always move image_idx on by one. This way, we end up with non-contiguous numbers, but the 
        // numbers will match the images output by the mips program.
        image_idx += 1;
    }

    return EXIT_SUCCESS;

}