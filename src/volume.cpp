/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file volume.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/02/2022
 * @brief Parse tiff files, augmenting with volume rotations
 * 
 * Given a directory of worm information, create the
 * masks and anything else we need for the dataset.
 * 
 * Internally we use the following codes for the neurons
 * 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2
 * 
 * Unlike dataset, we combine all the categories into
 * one image. We Also perform rotations in 3D to get 
 * extra images for the network to work with
 * 
 *
 */

#include "volume.hpp"
#include "image.hpp"
#include "data.hpp"
#include "rots.hpp"

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string annotation_path = ".";
    std::string prefix = "";
    bool rename = false;
    bool threeclass = false;    // Forget 1 and 2 and just go with ASI, ASJ or background.
    int offset_number = 0;
    bool bottom = false;
    int channels = 2;           // 2 Channels initially in these images
    int depth = 51;             // number of z-slices - TODO - should be set automatically along with width and height
    int width = 640;            // The desired dimensions
    int height = 300;
    int stacksize = 51;         // How many stacks in our input 2D image
    size_t roi_xy = 128;           // Square across this dimension
    size_t roi_depth = 21;         // Multiplied by the depth scale later
    uint16_t cutoff = 270;
    float depth_scale = 6.2;    // Ratio of Z/Depth to XY
    int num_augs = 1;
} Options;

using namespace masamune;

// A fixed set of augmentation directions
std::vector<glm::quat> ROTS;

/**
 * Given a tiff file return the same file but with one channel and 
 * as a series of layers
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 *
 * @return bool if success or not
 */

bool TiffToFits(Options &options, std::string &tiff_path, int image_idx, ROI &roi) {
    vkn::ImageU16L image;
    vkn::ImageU16L3D stacked;
    vkn::ImageU16L3D prefinal;
    image::LoadTiff<vkn::ImageU16L>(tiff_path, image);
    stacked.width = image.width;
    stacked.depth = options.stacksize;
    stacked.height = image.height / (stacked.depth * options.channels); // Two channels
    vkn::Alloc(stacked);
    uint coff = 0;

    if (options.bottom) {
        coff = stacked.height;
    }

    for (uint32_t d = 0; d < stacked.depth; d++) {
        for (uint32_t y = 0; y < stacked.height; y++) {
            // To get the FITS to match, we have to flip/mirror in the Y axis, unlike for PNG flatten.
            for (uint32_t x = 0; x < stacked.width; x++) {
                uint16_t val = image.image_data[(d * stacked.height * options.channels) + coff + y][x];
                // val = std::max(val - options.cutoff, 0);
                stacked.image_data[d][stacked.height - y - 1][x] = val;
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

    // Find ROI on a half-sized image as it's much faster.
    prefinal = stacked;
    prefinal = image::Resize(stacked, stacked.width / 2, stacked.height/ 2, stacked.depth/ 2);

    // Perform some augmentation by moving the ROI around a bit. Save these augs for the masking
    // that comes later as the mask must match
    float half_roi = static_cast<float>(options.roi_xy ) / 2.0;
    int d = static_cast<int>(ceil(sqrt(2.0f * half_roi * half_roi))) * 2;
    int depth = static_cast<int>(ceil(static_cast<float>(d) / options.depth_scale));

    // Because we are going to AUG, we make the ROI a bit bigger so we can rotate around
    ROI roi_found = FindROICentred(prefinal, d / 2, depth / 2);
    roi.x = roi_found.x * 2;
    roi.y = roi_found.y * 2;
    roi.z = roi_found.z * 2; 
    roi.xy_dim = roi_found.xy_dim * 2;
    roi.depth = roi_found.depth * 2;
    prefinal = image::Crop(stacked, roi.x, roi.y, roi.z, roi.xy_dim, roi.xy_dim, roi.depth);

    // Do some contrast work
    vkn::ImageF32L3D converted;
    image::Convert(prefinal, converted);

    std::function<float(float)> contrast = [](float x) { return x * x; };
    vkn::ImageF32L3D contrasted = vkn::ApplyFunc(converted, contrast);
    
    // Now perform some rotations and save the resulting 2D fits images
    ROTS.clear();
    glm::quat q(1.0,0,0,0);
    ROTS.push_back(q);

    for (int i = 0; i < options.num_augs; i++){
        // Rotate, normalise then sum projection
        std::string aug_id  = util::IntToStringLeadingZeroes(i, 2);
        output_path = options.output_path + "/" + image_id + "_" + aug_id + "_layered.fits";
        vkn::ImageF32L3D rotated = Augment(contrasted, q, options.roi_xy, options.depth_scale);
        vkn::ImageF32L3D normalised = image::Normalise(rotated);
        vkn::ImageF32L summed = vkn::Project(normalised, vkn::ProjectionType::SUM);
        summed = vkn::ApplyFunc(summed, contrast);

        // TODO - there is a bug in float resize. Not sure why!
        // Perform a resize with nearest neighbour sampling if we have different sizes.
        //if (options.width != summed.width || options.height != summed.height) {
        //    summed = image::Resize(summed, options.width, options.height);
        //} 

        //WriteFITS(output_path, flattened);
        WriteFITS(output_path, summed);
        q = RandRot();
        ROTS.push_back(q);
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

bool ProcessTiff(Options &options, std::string &tiff_path, std::string &log_path, int image_idx, ROI &roi) {
    vkn::ImageU16L image_in;
    vkn::ImageU8L3D prefinal;

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
        SetNeuron(image_in, neuron_mask, neurons, 1, false, 1);
        SetNeuron(image_in, neuron_mask, neurons, 2, false, 1);
        SetNeuron(image_in, neuron_mask, neurons, 3, false, 2);
        SetNeuron(image_in, neuron_mask, neurons, 4, false, 2);
    } else {
        SetNeuron(image_in, neuron_mask, neurons, 1, false, 1);
        SetNeuron(image_in, neuron_mask, neurons, 2, false, 2);
        SetNeuron(image_in, neuron_mask, neurons, 3, false, 3);
        SetNeuron(image_in, neuron_mask, neurons, 4, false, 4);
    }

    std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(log_path), "_.");
    std::string image_id = util::StringRemove(tokens_log[0], "ID");

    if (options.rename == true) {
        image_id = util::IntToStringLeadingZeroes(image_idx, 5);
        std::cout << "Renaming " <<  tiff_path << " to " << image_id << std::endl;
    }

    std::string output_path = options.output_path + "/" + options.prefix + image_id + "_mask.fits";
    std::string output_path_png = options.output_path + "/" + options.prefix + image_id + "_mask.png";
    prefinal = neuron_mask;
    prefinal = image::Crop(prefinal, roi.x, roi.y, roi.z, options.roi_xy, options.roi_xy, options.roi_depth);
        

    // Now generate the masks
    for (int i = 0; i < options.num_augs; i++){
        glm::quat q = ROTS[i];
        std::string aug_id  = util::IntToStringLeadingZeroes(i, 2);
        output_path = options.output_path + "/" + image_id + "_" + aug_id + "_mask.fits";
        prefinal = Augment(prefinal, q, options.roi_xy, options.depth_scale);

        // Perform a resize with nearest neighbour sampling if we have different sizes.
        if (options.width != prefinal.width || options.height != prefinal.height || options.depth != prefinal.depth) {
            prefinal = image::Resize(prefinal, options.width, options.height, options.depth);
        } 
        WriteFITS(output_path, prefinal);
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
    int image_idx = 0;

    while ((c = getopt_long(argc, (char **)argv, "i:o:a:p:rtbn:z:w:h:s:j:q:?", long_options, &option_index)) != -1) {
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
            case 'r' :
                options.rename = true;
                break;
            case 'b':
                options.bottom = true;
                break;
            case 't' :
                options.threeclass = true;
                break;
            case 'n':
                options.offset_number = util::FromString<int>(optarg);
                image_idx = options.offset_number;
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
            case 'q':
                options.num_augs = util::FromString<int>(optarg);
                break;
        }
    }

    //return EXIT_FAILURE;
    std::cout << "Loading annotation images from " << options.annotation_path << std::endl;
    std::cout << "Offset: " << options.offset_number << ", rename: " << options.rename << std::endl;
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
                            ROI roi;
                            std::cout << "Stacking: " << tiff_input << std::endl;
                            TiffToFits(options, tiff_input, image_idx, roi);
                            std::cout << "Pairing " << tiff_anno << " with " << log << " and " << tiff_input << std::endl;
                            paired = true;
                            // ProcessTiff(options, tiff_anno, log, image_idx, roi);
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