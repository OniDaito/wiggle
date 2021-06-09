/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file solange.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 19/05/2021
 * @brief Parse our tiff files and create our masks
 * 
 * Given a directory of worm information, create the masks
 * we need.
 *
 */

#include <getopt.h>
#include <masamune/masamune_prog.h>
#include <masamune/util/string.h>
#include <masamune/util/file.h>
#include <masamune/image/tiff.h>
#include <masamune/image/basic.h>
#include <vector>

using namespace masamune;

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string prefix = "";
    bool flatten = false;
} Options;

void SetNeuron(vkn::ImageU16L &image_in, vkn::ImageU8L3D &image_out,
std::vector<std::vector<size_t>> &neurons, int neuron_id) {

    for (uint32_t d = 0; d < image_out.depth; d++) {

        for (uint32_t y = 0; y < image_out.height; y++) {

            for (uint32_t x = 0; x < image_out.width; x++) {
                uint16_t val = image_in.image_data[d * image_out.height + y][x];
                if (val != 0) {
                    std::vector<size_t>::iterator it = std::find(neurons[neuron_id].begin(),
                        neurons[neuron_id].end(), static_cast<size_t>(val));
                    if (it != neurons[neuron_id].end()) {
                        uint8_t nval = neuron_id;
                        image_out.image_data[d][y][x] = nval;
                    } 
                }
            }
        }
    }
}

/**
 * Given a 3D image, flatten it.
 * 
 * @param mask - a 3D image

 * @return a 2D vkn image
 */
vkn::ImageU8L Flatten(vkn::ImageU8L3D &mask) {
    vkn::ImageU8L flat;
    flat.width = mask.width;
    flat.height = mask.height;
    vkn::Alloc(flat);
    
    for (uint32_t d = 0; d < mask.depth; d++) {

        for (uint32_t y = 0; y < flat.height; y++) {
            for (uint32_t x = 0; x < flat.width; x++) {
                uint16_t val = mask.image_data[d][y][x];
                uint16_t ext = flat.image_data[y][x];
                if (ext == 0) {
                    flat.image_data[y][x] = val;
                }
            }
        }
    }
    return flat;
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

bool ProcessTiff(Options &options, std::string &tiff_path, std::string &log_path) {
    vkn::ImageU16L image_in;
    std::vector<std::string> lines = util::ReadFileLines(log_path);
    image::LoadTiff<vkn::ImageU16L>(tiff_path, image_in);
    size_t idx = 0;
    std::vector<std::vector<size_t>> neurons; // 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2

    // Read the log file and extract the     
    for (int i = 0; i < 5; i++) {
        neurons.push_back(std::vector<size_t>());
    }

    for (std::string line : lines) {
        std::vector<std::string> tokens = util::SplitStringWhitespace(line);
        if (util::ToLower(tokens[0]) == "associate") {
            idx += 1;
        } else {
            neurons[idx].push_back(util::FromString<size_t>(tokens[0]));
        }
    }

    // Export ASI first. We split the tiffs so they have layers
    vkn::ImageU8L3D asi;
    asi.width = image_in.width;
    asi.depth = 51;
    asi.height = image_in.height / asi.depth;
    vkn::Alloc(asi);

    SetNeuron(image_in, asi, neurons, 0);
    SetNeuron(image_in, asi, neurons, 1);

    std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(log_path), "_.");
    std::string image_id = util::StringRemove(tokens_log[0], "ID");
    std::string output_path = options.output_path + "/" + options.prefix + image_id + "_asi.tiff";
    std::string output_path_png = options.output_path + "/" + options.prefix + image_id + "_asi.png";

    image::SaveTiff(output_path, asi);
    if (options.flatten){
        vkn::ImageU8L asi_flat = Flatten(asi);
        image::SaveTiff(output_path, asi_flat);
        image::Save(output_path_png, asi_flat);

    } else {
        image::SaveTiff(output_path, asi);
    } 

    // Now look at ASJ
    vkn::ImageU8L3D asj;
    asj.width = image_in.width;
    asj.depth = 51;
    asj.height = image_in.height / asj.depth;
    vkn::Alloc(asj);

    SetNeuron(image_in, asj, neurons, 2);
    SetNeuron(image_in, asj, neurons, 3);

    output_path = options.output_path + "/" + options.prefix + image_id + "_asj.tiff";
    output_path_png = options.output_path + "/" + options.prefix + image_id + "_asj.png";

    if (options.flatten){
        vkn::ImageU8L asj_flat = Flatten(asj);
        image::SaveTiff(output_path, asj_flat);
        image::Save(output_path_png, asj_flat);
    } else {
        image::SaveTiff(output_path, asj);
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

    while ((c = getopt_long(argc, (char **)argv, "i:o:p:f?", long_options, &option_index)) != -1) {
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
        }
    }
   
    //return EXIT_FAILURE;
    std::cout<< "Loading images from " << options.image_path << std::endl;

    // Browse the directory looking for files
    std::vector<std::string> files = util::ListFiles(options.image_path);
    std::vector<std::string> tiff_files;
    std::vector<std::string> log_files;
    std::vector<std::string> dat_files;

    for (std::string filename : files) {
        std::cout << filename << std::endl;
        if (util::StringContains(filename, ".tif")) {
            tiff_files.push_back(filename);
        }
        else if (util::StringContains(filename, ".dat")) {
            dat_files.push_back(filename);
        }
        else if (util::StringContains(filename, ".log")) {
            log_files.push_back(filename);
        }
    }

    // Apparently we are missing one for some reason. Oh well
    // assert(tiff_files.size() == log_files.size());

    // Pair up the tiffs with their log file and process them.
    for (std::string tiff : tiff_files) {
        std::vector<std::string> tokens = util::SplitStringChars(util::FilenameFromPath(tiff), "_.");
        std::string id = tokens[0];
        for (std::string log : log_files) {
            std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(log), "_.");
            if (tokens_log[0] == id){
                std::cout << "Pairing " << tiff << " with " << log << std::endl;
                ProcessTiff(options, tiff, log);
            }
        }
    }


    return EXIT_SUCCESS;

}