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
#include "options.hpp"
#include "pipe.hpp"

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

    std::string watershed_path, annotation_path, image_path;

    while ((c = getopt_long(argc, (char **)argv, "i:o:a:p:rtfdbmuvn:z:w:h:l:c:s:j:q:k:g:e:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                image_path = std::string(optarg);
                break;
            case 'a' :
                annotation_path = std::string(optarg);
                break;
            case 'o' :
                options.output_path = std::string(optarg);
                break;
            case 'l' :
                options.output_log_path = std::string(optarg);
                break;
            case 'p' :
                options.prefix = std::string(optarg);
                break;
            case 'r' :
                watershed_path = true;
                break;
            case 'u' :
                options.autoback = true;
                break;
            case 'b':
                options.bottom = true;
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

    ROI roi;
    std::vector<glm::quat> ROTS;
    std::string coord_path = "";
    
    if (options.num_augs > 1) {
        ProcessMask(options, watershed_path, annotation_path, coord_path, ROTS, 0, roi);
        TiffToFits(options, ROTS, image_path, 0, roi);
    } else {
        ProcessMaskNoAug(options, watershed_path, annotation_path, coord_path, 0, roi);
        TiffToFits(options, ROTS, image_path, 0, roi);
    }
 
    return EXIT_SUCCESS;

}