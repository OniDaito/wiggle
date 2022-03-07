/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file multiset.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/09/2021
 * @brief Parse our tiff files and create our masks
 * 
 * Given a directory of worm information, create the
 * masks and anything else we need for the dataset.
 * 
 * Internally we use the following codes for the neurons
 * 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2
 * 
 * Unlike dataset, we combine all the categories into
 * one image. 
 * 
 * TODO for now a voxel can only be one category. This
 * might need changing in the future.
 *
 */

#include "multiset.hpp"
#include "image.hpp"
#include "data.hpp"
#include "threadpool.hpp"

using namespace masamune;

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string annotation_path = ".";
    std::string prefix = "";
    bool flatten = false;
    bool rename = false;
    bool crop = false;          // Crop the image to an ROI.
    bool threeclass = false;    // Forget 1 and 2 and just go with ASI, ASJ or background.
    int offset_number = 0;
    bool bottom = false;
    int channels = 2;           // 2 Channels initially in these images
    int depth = 51;             // number of z-slices - TODO - should be set automatically along with width and height
    int width = 640;            // The desired dimensions
    int height = 300;
    int stacksize = 51;         // How many stacks in our input 2D image
    int roi_xy = 128;
    int roi_depth = 25;
    int num_rois = 10;
    uint16_t cutoff = 0;
} Options;


// A fixed set of augmentation directions
std::vector<ROI> AUGS;


/**
 * Given a tiff file return the same file but with one channel and 
 * as a series of layers
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 *
 * @return bool if success or not
 */

bool TiffToFits(Options &options, std::string &tiff_path, int image_idx, bool flipy) {
    vkn::ImageU16L image;
    vkn::ImageU16L3D stacked;
    image::LoadTiff<vkn::ImageU16L>(tiff_path, image);
    stacked.width = image.width;
    stacked.depth = options.stacksize;
    stacked.height = image.height / (stacked.depth * options.channels); // Two channels
    vkn::Alloc(stacked);
    uint coff = 0;
    ROI roi;
    ThreadPool pool{ static_cast<size_t>(options.num_rois) }; // 1 thread per ROI
    std::vector<std::future<int>> futures;

    if (options.bottom) {
        coff = stacked.height;
    }

    for (uint32_t d = 0; d < stacked.depth; d++) {
        for (uint32_t y = 0; y < stacked.height; y++) {
            // To get the FITS to match, we have to flip/mirror in the Y axis, unlike for PNG flatten.
            for (uint32_t x = 0; x < stacked.width; x++) {
                uint16_t val = image.image_data[(d * stacked.height * options.channels) + coff + y][x];

                if (flipy) {
                    stacked.image_data[d][stacked.height - y - 1][x] = std::max(val - options.cutoff, 0);
                } else {
                    stacked.image_data[d][y][x] = std::max(val - options.cutoff, 0);
                }
            }
        }
    }

    std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = util::StringRemove(image_id, "0xAutoStack");
    std::string output_path = options.output_path + "/" + image_id + "_layered.fits";

    if (options.rename == true) {
        image_id  = util::IntToStringLeadingZeroes(image_idx, 5);
        output_path = options.output_path + "/" + image_id + "_layered.fits";
        std::cout << "Renaming " << tiff_path << " to " << output_path << std::endl;
    }

    vkn::ImageU16L3D final_image = stacked;

    // Perform a resize with nearest neighbour sampling if we have different sizes.
    if (options.width != stacked.width || options.height != stacked.height || options.depth != stacked.depth) {
        final_image = image::Resize(stacked, options.width, options.height, options.depth);
    }

    if (options.crop) {
        AUGS.clear();
        ROI roi_found = FindROI(final_image,options.roi_xy, options.roi_depth);
        ROI roi;
        roi.x = roi_found.x;
        roi.y = roi_found.y;
        roi.z = roi_found.z;

        AUGS.push_back(roi);

        for (int i = 1; i < options.num_rois; i++){
            int minx = -static_cast<int>(roi.x) / 10;
            int maxx = (final_image.width - roi.x - options.roi_xy) / 10;
            int miny = -static_cast<int>(roi.y) / 10;
            int maxy = (final_image.height - roi.y - options.roi_xy) / 10;

            roi.x += minx + (rand() % (maxx - minx));
            roi.y += miny + (rand() % (maxy - miny));

            AUGS.push_back(roi);
            roi.x = roi_found.x;
            roi.y = roi_found.y;
        }

        // Perform some augmentation by moving the ROI around a bit. Save these augs for the masking
        // that comes later as the mask must match

        for (int i = 0; i < options.num_rois; i++){
            futures.push_back(pool.execute( [i, options, image_id, final_image] {
                std::string augnum = util::IntToStringLeadingZeroes(i, 2);
                std::string output_path = options.output_path + "/" + image_id + "_" + augnum + "_layered.fits";
                ROI roi = AUGS[i];
                vkn::ImageU16L3D cropped_image = image::Crop(final_image, roi.x, roi.y, roi.z, options.roi_xy, options.roi_xy, options.roi_depth);
                
                if (options.flatten){
                    vkn::ImageU16L flattened = vkn::Project(cropped_image, vkn::ProjectionType::SUM);
                    vkn::ImageF32L converted;
                    vkn::Convert(flattened, converted);

                    // Increase the Contrast
                    for (uint32_t h = 0; h < converted.height; h++) {
                        for (uint32_t w = 0; w < converted.width; w++) {
                            float val = converted.image_data[h][w];
                            converted.image_data[h][w] = val * 2.0;
                        }
                    }

                    WriteFITS(output_path, converted);
                } else {
                    WriteFITS(output_path, final_image);
                }
                return i;
            }));
        }

        for (auto &fut : futures) { fut.get(); }

    } else { 

      if (options.flatten){
          vkn::ImageU16L flattened = vkn::Project(final_image, vkn::ProjectionType::SUM);
          WriteFITS(output_path, flattened);
      } else {
          WriteFITS(output_path, final_image);
      }
    }
    return true;
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
    vkn::ImageU16L image_in;
    std::vector<std::string> lines = util::ReadFileLines(log_path);
    image::LoadTiff<vkn::ImageU16L>(tiff_path, image_in);
    size_t idx = 0;
    std::vector<std::vector<size_t>> neurons; // 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2

    // Read the log file and extract the neuron numbers
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

    // Join all our neurons
    vkn::ImageU8L3D neuron_mask;
    neuron_mask.width = image_in.width;
    neuron_mask.depth = options.stacksize;
    neuron_mask.height = image_in.height / neuron_mask.depth;
    vkn::Alloc(neuron_mask);

    if (options.threeclass) {
        SetNeuron(image_in, neuron_mask, neurons, 1, !options.flatten, 1);
        SetNeuron(image_in, neuron_mask, neurons, 2, !options.flatten, 1);
        SetNeuron(image_in, neuron_mask, neurons, 3, !options.flatten, 2);
        SetNeuron(image_in, neuron_mask, neurons, 4, !options.flatten, 2);
    } else {
        SetNeuron(image_in, neuron_mask, neurons, 1, !options.flatten, 1);
        SetNeuron(image_in, neuron_mask, neurons, 2, !options.flatten, 2);
        SetNeuron(image_in, neuron_mask, neurons, 3, !options.flatten, 3);
        SetNeuron(image_in, neuron_mask, neurons, 4, !options.flatten, 4);
    }

    std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(log_path), "_.");
    std::string image_id = util::StringRemove(tokens_log[0], "ID");

    if (options.rename == true) {
        image_id = util::IntToStringLeadingZeroes(image_idx, 5);
        std::cout << "Renaming " <<  tiff_path << " to " << image_id << std::endl;
    }

    std::string output_path = options.output_path + "/" + options.prefix + image_id + "_mask.fits";
    std::string output_path_png = options.output_path + "/" + options.prefix + image_id + "_mask.png";
    vkn::ImageU8L3D final_image = neuron_mask; 

    if (non_zero(neuron_mask)) {

        if (neuron_mask.width != options.width || neuron_mask.height != options.height || neuron_mask.depth != options.depth) {
            final_image = image::Resize(neuron_mask, options.width, options.height, options.depth);

            if (options.crop) {
                for (int i = 0; i < options.num_rois; i++) {
                    std::string aug = util::IntToStringLeadingZeroes(i, 2);
                    output_path = options.output_path + "/" + image_id + "_" + aug + "_mask.fits";
                    final_image = image::Crop(final_image, AUGS[i].x, AUGS[i].y, AUGS[i].z, options.roi_xy, options.roi_xy, options.roi_depth);        
                    WriteFITS(output_path, final_image);
                }
            }
        }
    }

    if (!options.crop) {
      WriteFITS(output_path, final_image);
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
    int image_idx = 0;

    while ((c = getopt_long(argc, (char **)argv, "i:o:a:p:frtbcn:z:w:h:s:j:l:q:d:?", long_options, &option_index)) != -1) {
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
            case 'p' :
                options.prefix = std::string(optarg);
                break;
            case 'f' :
                options.flatten = true;
                break;
            case 'r' :
                options.rename = true;
                break;
            case 'b':
                options.bottom = true;
                break;
            case 't' :
                options.threeclass = true;
                break;
            case 'c' :
                options.crop = true;
                break;
            case 'n':
                options.offset_number = util::FromString<int>(optarg);
                image_idx = options.offset_number;
                break;
            case 'd':
                options.cutoff = util::FromString<uint16_t>(optarg);
                break;
            case 'z':
                options.depth = util::FromString<int>(optarg);
                break;
            case 'w':
                options.width = util::FromString<int>(optarg);
                break;
            case 'h':
                options.height = util::FromString<int>(optarg);
                break;
            case 's':
                options.stacksize = util::FromString<int>(optarg);
                break;
            case 'j':
                options.roi_xy = util::FromString<int>(optarg);
                break;
            case 'l':
                options.roi_depth = util::FromString<int>(optarg);
                break;
            case 'q':
                options.num_rois = util::FromString<int>(optarg);
                break;
        }
    }

    //return EXIT_FAILURE;
    std::cout << "Loading annotation images from " << options.annotation_path << std::endl;
    std::cout << "Offset: " << options.offset_number << ", rename: " << options.rename << ", flatten: " << options.flatten << std::endl;
    std::cout << "Output Size Width: " << options.width << ", Height: " << options.height << ", Depth: " << options.depth << std::endl;

    // First, find and sort the annotation files
    std::vector<std::string> tiff_anno_files = FindAnnotations(options.annotation_path);

    // Next, find the annotation log files
    std::vector<std::string> log_files = FindLogFiles(options.annotation_path);

    // Now find the input files
    std::cout << "Loading input images from " << options.image_path << std::endl;
    std::cout << "Options: bottom: " << options.bottom << ", Stacksize:" << options.stacksize << std::endl;

    std::vector<std::string> tiff_input_files = FindInputFiles(options.image_path);

    // Pair up the tiffs with their log file and then the input and process them.
    for (std::string tiff_anno : tiff_anno_files) {
        bool paired = false;
        std::vector<std::string> tokens = util::SplitStringChars(util::FilenameFromPath(tiff_anno), "_.-");
        std::string id = tokens[0];

        for (std::string log : log_files) {
            std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(log), "_.-");
            if (tokens_log[0] == id) {
                
                for (std::string tiff_input : tiff_input_files) {
                    // Find the matching input stack
                    std::vector<std::string> tokens1 = util::SplitStringChars(util::FilenameFromPath(tiff_input), "_.-");
                    int tidx = 0;

                    for (std::string t : tokens1) {
                        if (util::StringContains(t, "AutoStack")) {
                            break;
                        }
                        tidx += 1;
                    }
                    
                    int ida = util::FromString<int>(util::StringRemove(tokens1[tidx], "0xAutoStack"));
                    int idb = util::FromString<int>(util::StringRemove(id, "ID"));

                    if (ida == idb) {
                        try {
                            std::cout << "Stacking: " << tiff_input << std::endl;
                            TiffToFits(options, tiff_input, image_idx, options.flatten);
                            std::cout << "Pairing " << tiff_anno << " with " << log << " and " << tiff_input << std::endl;
                            paired = true;
                            // TODO - currently having issues with the ROI so I've left it out
                            // ProcessTiff(options, tiff_anno, log, image_idx);
                            image_idx += 1;
                            break;
                        } catch (const std::exception &e) {
                             std::cout << "An exception occured with" << tiff_anno << " and " <<  tiff_input << std::endl;
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
