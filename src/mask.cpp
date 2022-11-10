/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file mask.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 10/11/2022
 * @brief Create a graphed dataset for our worm data
 * 
 * Given a directory of worm information, create nice
 * source images and a series of graph coordinates
 * 
 * Internally we use the following codes for the neurons
 * 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2
 * 
 *
 */

#include <libcee/string.hpp>
#include <libcee/file.hpp>
#include <libcee/threadpool.hpp>
#include <imagine/imagine.hpp>
#include <sys/stat.h>
#include <map>
#include "volume.hpp"
#include "image.hpp"
#include "data.hpp"
#include "rots.hpp"
#include "pipe.hpp"
#include "options.hpp"

using namespace imagine;

bool folder_exists(std::string foldername)  {
    struct stat st;
    stat(foldername.c_str(), &st);
    return st.st_mode & S_IFDIR;
}

bool StackMask(Options &options, std::string &tiff_path, std::string &log_path, std::string &coord_path) {
    ImageU16L image_in = LoadTiff<ImageU16L>(tiff_path);
    std::vector<std::vector<size_t>> neurons; // 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2
    size_t idx = 0;

    // Read the log file and extract the neuron numbers
    // Making the asssumption that all log files have the neurons in the same order
    for (int i = 0; i < 5; i++) {
        neurons.push_back(std::vector<size_t>());
    }

    std::vector<std::string> lines_log = libcee::ReadFileLines(log_path);

    for (std::string line : lines_log) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        if (libcee::ToLower(tokens[0]) == "associate") {
            idx += 1;
        } else {
            neurons[idx].push_back(libcee::FromString<size_t>(tokens[0]));
        }
    }

    // Join all our neurons
    ImageU8L3D neuron_mask(image_in.width, image_in.height / options.stacksize, options.stacksize);

    bool n1 = false, n2 = false, n3 = false, n4 = false;

    if(options.threeclass){
      n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, true, 1);
      n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, true, 1);
      n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, true, 2);
      n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, true, 2);
    } else {
      n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, true, 1);
      n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, true, 2);
      n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, true, 3);
      n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, true, 4);
    }
    
    if (!n1 || !n2 || !n3 || !n4) {
        return false;
    }

    std::vector<std::string> end = libcee::SplitStringString(tiff_path, options.output_path);
    std::string new_path =   tiff_path.replace(0, options.base_path.length(), options.output_path);
    std::string output_path = new_path.replace(new_path.length() - 5, 5, ".fits");
    std::string new_dir = libcee::PathFromPath(output_path);

    if (!folder_exists(new_dir)) {
        std::cout << "Making new directory " << new_dir << std::endl;
        mkdir(new_dir.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
   
    FlipVerticalI(neuron_mask); 

    std::cout << "Saving new mask to " << output_path << std::endl;
    
    if (options.final_width != neuron_mask.width || options.final_height != neuron_mask.height || options.final_depth != neuron_mask.depth) {
        if (neuron_mask.depth % 2 == 1) {
            neuron_mask.data.pop_back();
        }
        ImageU8L3D resized = Resize(neuron_mask, options.final_width, options.final_height, options.final_depth);
        try {
            SaveFITS(output_path, resized);
        } catch (std::exception& exc) {
            std::cout << "Failed to save " << output_path << std::endl;
        }

    } else {
        try {
            SaveFITS(output_path, neuron_mask);   
        } catch (std::exception& exc) {
            std::cout << "Failed to save " << output_path << std::endl;
        }
    }
  
    return true;
}


int main (int argc, char ** argv) {
    // Parse command line options
    Options options;
    int c;
    static struct option long_options[] = {
        {"no-interz", no_argument, NULL, 1},
        {"no-subpixel", no_argument, NULL, 2},
        {NULL, 0, NULL, 0}
    };

    int option_index = 0;
    int image_idx = 0;

    while ((c = getopt_long(argc, (char **)argv, "o:a:ts:l:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'a' :
                options.annotation_path = std::string(optarg);
                break;
            case 'o' :
                options.output_path = std::string(optarg);
                break;
            case 't' :
                options.threeclass = true;
                break;
            case 's':
                options.stacksize = libcee::FromString<int>(optarg);
                break;
            case 'l' :
                options.base_path = std::string(optarg);
                break;
        }
    }

    options.stacksize = 51;
    options.final_depth = 51;
    options.final_width = 640;
    options.final_height = 300;

    std::cout << "Loading annotation images from " << options.annotation_path << std::endl;
 
    // First, find and sort the annotation files
    std::vector<std::string> tiff_anno_files = FindAnnotations(options.annotation_path);

    // Next, find the annotation dat files
    std::vector<std::string> dat_files = FindDatFiles(options.annotation_path);

    // Next, find the annotation log files
    std::vector<std::string> log_files = FindLogFiles(options.annotation_path);

 

    // Pair up the tiffs with their log file and then the input and process them.
    for (std::string tiff_anno : tiff_anno_files) {
        std::vector<std::string> tokens = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_anno), "_.-");
        std::string id = tokens[0];

        for (std::string log : log_files) {
            std::vector<std::string> tokens_log = libcee::SplitStringChars(libcee::FilenameFromPath(log), "_.-");
            if (tokens_log[0] == id) {

                for (std::string dat : dat_files) {
                    std::vector<std::string> tokens_dat = libcee::SplitStringChars(libcee::FilenameFromPath(dat), "_.-");
                    
                    if (tokens_dat[0] == id) {
                        
                        try {
                            std::cout << "Masking: " << dat << std::endl;
                            StackMask(options, tiff_anno, log, dat);
                        
                        } catch (const std::exception &e) {
                            std::cout << "An exception occured with" << tiff_anno << std::endl;
                        }
                    }
                    
                }
            }
        }
    }

    return EXIT_SUCCESS;

}
