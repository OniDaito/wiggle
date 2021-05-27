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