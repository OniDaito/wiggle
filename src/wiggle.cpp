/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file wiggle.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 25/04/2022
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
#include <glm/gtc/quaternion.hpp>
#include <map>
#include "volume.hpp"
#include "image.hpp"
#include "data.hpp"
#include "rots.hpp"
#include "pipe.hpp"
#include "options.hpp"

using namespace imagine;

bool is_csv_empty(std::string path) {
    std::ifstream file(path);
    if (!file){
        return true;
    }

    return file.peek() == std::ifstream::traits_type::eof();
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

    while ((c = getopt_long(argc, (char **)argv, "i:o:a:p:rtfdmuvn:z:w:h:l:c:s:j:q:k:g:e:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                options.image_path = std::string(optarg);
                break;
            case 'a' :
                options.annotation_path = std::string(optarg);
                break;
            case 'o' :
                options.output_path = std::string(optarg);
                image_idx = GetOffetNumber(options.output_path);
                break;
            case 'l' :
                options.output_log_path = std::string(optarg);
                break;
            case 'p' :
                options.prefix = std::string(optarg);
                break;
            case 'r' :
                options.rename = true;
                break;
            case 'u' :
                options.autoback = true;
                break;
            case 'd':
                options.deconv = true;
                break;
            case 'f':
                options.flatten = true;
                break;
            case 'm':
                options.max_intensity = true;
                break;
            case 't' :
                options.threeclass = true;
                break;
            case 'v' :
                options.otsu = true;
                break;
            case 'n':
                options.offset_number = libcee::FromString<int>(optarg);
                image_idx = options.offset_number;
                break;
            case 'c':
                options.cutoff = libcee::FromString<int>(optarg);
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
            case 'j':
                options.roi_xy = libcee::FromString<int>(optarg);
                break;
            case 'q':
                options.num_augs = libcee::FromString<int>(optarg);
                break;
            case 'k' :
                options.psf_path = std::string(optarg);
                break;
            case 'g':
                RANDROT_GENERATOR.seed(libcee::FromString<int>(optarg));
                break;
            case 'e':
                options.deconv_rounds = libcee::FromString<int>(optarg);
                break;
            case 1 :
                options.interz = false;
                break;
            case 2 :
                options.subpixel = false;
                break;
        }
    }

    //return EXIT_FAILURE;
    std::cout << "Loading annotation images from " << options.annotation_path << std::endl;
    std::cout << "Offset: " << options.offset_number << ", rename: " << options.rename << std::endl;
    std::cout << "Subpixel, interplateZ: " << options.subpixel << ", " << options.interz << std::endl;

    // Rotations for the augmentation
    std::vector<glm::quat> ROTS;

    // First, find and sort the annotation files
    std::vector<std::string> tiff_anno_files = FindAnnotations(options.annotation_path);

    // Next, find the annotation dat files
    std::vector<std::string> dat_files = FindDatFiles(options.annotation_path);

    // Next, find the annotation log files
    std::vector<std::string> log_files = FindLogFiles(options.annotation_path);

    // Now find the input files
    std::cout << "Loading input images from " << options.image_path << std::endl;

    std::vector<std::string> tiff_input_files = FindInputFiles(options.image_path);

    // Pair up the tiffs with their log file and then the input and process them.
    for (std::string tiff_anno : tiff_anno_files) {
        bool paired = false;
        std::vector<std::string> tokens = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_anno), "_.-");
        std::string id = tokens[0];

        for (std::string log : log_files) {
            std::vector<std::string> tokens_log = libcee::SplitStringChars(libcee::FilenameFromPath(log), "_.-");
            if (tokens_log[0] == id) {

                for (std::string dat : dat_files) {
                    std::vector<std::string> tokens_dat = libcee::SplitStringChars(libcee::FilenameFromPath(dat), "_.-");
                    if (tokens_dat[0] == id) {
                        
                        for (std::string tiff_input : tiff_input_files) {
                            // Find the matching input stack
                            std::vector<std::string> tokens1 = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_input), "_.-");
                            int tidx = 0;

                            for (std::string t : tokens1) {
                                if (libcee::StringContains(t, "AutoStack")) {
                                    break;
                                }
                                tidx += 1;
                            }
                            
                            int ida = libcee::FromString<int>(libcee::StringRemove(tokens1[tidx], "0xAutoStack"));
                            int idb = libcee::FromString<int>(libcee::StringRemove(id, "ID"));

                            if (ida == idb) {
                                try {
                                    ROI roi;
                                    std::cout << "Masking: " << dat << std::endl;
                                    if (ProcessMask(options, tiff_anno, log, dat, ROTS, image_idx, roi)) {
                                        std::cout << "Stacking: " << tiff_input << std::endl;
                                        TiffToFits(options, ROTS, tiff_input, image_idx, roi);
                                        std::cout << "Pairing " << tiff_anno << " with " << dat << " and " << tiff_input << std::endl;
                                        paired = true;
                                        image_idx +=1;
                                        break;
                                    }
                                
                                } catch (const std::exception &e) {
                                    std::cout << "An exception occured with" << tiff_anno << " and " <<  tiff_input << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!paired){
            std::cout << "Failed to pair " << tiff_anno << std::endl;
        }
    }

    return EXIT_SUCCESS;

}
