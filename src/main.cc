/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file main.cc
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

using namespace masamune;

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
} Options;


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
    vkn::ImageU16L image;

    std::vector<std::string> lines = util::ReadFileLines(log_path);
    image::LoadTiff<vkn::ImageU16L>(tiff_path, image);

    size_t idx = 0;
    std::vector<std::vector<size_t>> neurons; // 0: ASI-1, 1: ASI-2, 2: ASJ-1, 3: ASJ-2

    // Read the log file and extract the     
    for (int i = 0; i < 4; i++) {
        neurons.push_back(std::vector<size_t>());
    }

    for (std::string line : lines) {
        std::vector<std::string> tokens = util::SplitStringWhitespace(line);
        if (util::ToLower(tokens[0]) == "associate") {
            idx += 1;
        } else {
            neurons[idx-1].push_back(util::FromString<size_t>(tokens[0]));
        }
    }

    // Export ASI-1 first. We split the tiffs so they have layers
    vkn::ImageU8L3D asi1;
    int neuron_id = 0;
    asi1.width = image.width;
    asi1.depth = 51;
    asi1.height = image.height / asi1.depth ;

    for (uint32_t d = 0; d < asi1.depth; d++) {
        asi1.image_data.push_back(std::vector<std::vector<uint8_t>>());

        for (uint32_t y = 0; y < asi1.height; y++) {
            asi1.image_data[d].push_back(std::vector<uint8_t>());

            for (uint32_t x = 0; x < asi1.width; x++) {
                uint16_t val = image.image_data[d * asi1.height + y][x];
                if (val == 0) {
                    asi1.image_data[d][y].push_back(0);
                } else {
                    std::vector<size_t>::iterator it = std::find(neurons[neuron_id].begin(),
                        neurons[neuron_id].end(), static_cast<size_t>(val));
                    if (it != neurons[neuron_id].end()) {
                        asi1.image_data[d][y].push_back(1);
                    } else {
                        asi1.image_data[d][y].push_back(0);
                    }
                }
            }
        }
    }

    std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(log_path), "_.");
    std::string image_id = tokens_log[0];
    std::string output_path = options.output_path + "/" + image_id + "_asi1.tiff";
    image::SaveTiff(output_path, asi1);

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

    while ((c = getopt_long(argc, (char **)argv, "i:o:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                options.image_path = std::string(optarg);
                break;
            case 'o' :
                options.output_path = std::string(optarg);
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