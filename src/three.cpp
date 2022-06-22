/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file three.cpp
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
 * We don't augment but we do create a 3D FITS file
 */

#include <libcee/string.hpp>
#include <libcee/file.hpp>
#include <libcee/threadpool.hpp>
#include <imagine/imagine.hpp>
#include "volume.hpp"
#include "image.hpp"
#include "data.hpp"
#include "rots.hpp"

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string annotation_path = ".";
    std::string output_log_path = ".";
    std::string prefix = "";
    bool rename = false;
    bool threeclass = false;    // Forget 1 and 2 and just go with ASI, ASJ or background.
    int offset_number = 0;
    bool bottom = false;
    int channels = 2;           // 2 Channels initially in these images
    int depth = 51;             // number of z-slices - TODO - should be set automatically along with width and height
    int width = 640;            // The input dimensions of each slice
    int height = 300;
    int final_depth = 16;             // number of z-slices - TODO - should be set automatically along with width and height
    int final_width = 128;            // The input dimensions of each slice
    int final_height = 128;
    int stacksize = 51;         // How many stacks in our input 2D image
    size_t roi_xy = 128;           // Square across this dimension
    size_t roi_depth = 21;         // Multiplied by the depth scale later
    uint16_t cutoff = 270;
    float depth_scale = 6.2;    // Ratio of Z/Depth to XY
    int num_augs = 1;
} Options;

using namespace imagine;

// A fixed set of augmentation directions
std::vector<glm::quat> ROTS;

/**
 * @brief The Image processing pipeline for source images.
 * Perform a Crop, noise subtraction and deconvolution
 *
 * 
 * @param image_in 
 * @param roi 
 * @return ImageF32L3D 
 */

ImageF32L3D ProcessPipe(ImageU16L3D const &image_in,  ROI &roi) {
    // Find ROI on a half-sized image as it's much faster.
    ImageU16L3D prefinal = Clone(image_in);
    prefinal = Crop(prefinal, roi.x, roi.y, roi.z, roi.xy_dim, roi.xy_dim, roi.depth);

    // Convert to float as we need to do some operations
    ImageF32L3D converted = Convert<ImageF32L3D>(prefinal);

    // Perform a subtraction on the images, removing background
    converted = Sub(converted, 280.0f, true);

    //std::function<float(float)> log_func = [](float x) { return std::max(10.0f * log2(x), 0.0f); };
    //converted = ApplyFunc<ImageF32L3D, float>(converted, log_func);

    // Deconvolve with a known PSF
    std::string path_kernel("./images/PSF3.tif");
    ImageF32L3D kernel = LoadTiff<ImageF32L3D>(path_kernel);
    ImageF32L3D deconved = DeconvolveFFT(converted, kernel, 10);
    
    return deconved;
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

bool TiffToFits(Options &options, std::string &tiff_path, int image_idx, ROI &roi) {
    ImageU16L image = LoadTiff<ImageU16L>(tiff_path);
    ImageU16L3D stacked(image.width, image.height / (options.stacksize * options.channels), options.stacksize);
    uint coff = 0;
    stacked.depth -= 1;
    stacked.data.pop_back();

    if (options.bottom) {
        coff = stacked.height;
    }

    for (uint32_t d = 0; d < stacked.depth; d++) {
        for (uint32_t y = 0; y < stacked.height; y++) {
            // To get the FITS to match, we have to flip/mirror in the Y axis, unlike for PNG flatten.
            for (uint32_t x = 0; x < stacked.width; x++) {
                uint16_t val = image.data[(d * stacked.height * options.channels) + coff + y][x];
                // val = std::max(val - options.cutoff, 0);
                stacked.data[d][stacked.height - y - 1][x] = val;
                //stacked.data[d][y][x] = val;
            }
        }
    }

    std::vector<std::string> tokens_log = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = libcee::StringRemove(image_id, "0xAutoStack");
    std::string output_path = options.output_path + "/" + image_id + "_layered.fits";

    if (options.rename == true) {
        image_id  = libcee::IntToStringLeadingZeroes(image_idx, 5);
        output_path = options.output_path + "/" + image_id + "_layered.fits";
        std::cout << "Renaming " << tiff_path << " to " << output_path << std::endl;
    }

    ImageF32L3D processed = ProcessPipe(stacked, roi);
    ImageF32L3D normalised = Normalise(processed);
    
    // Flip vertically again for some reason
    FlipVerticalI(normalised);
    ImageF32L3D resized = Resize(normalised, options.final_width, options.final_height, options.final_depth);
    SaveFITS(output_path, resized);
         
    return true;
}


bool is_csv_empty(std::string path) {
    std::ifstream file(path);
    if (!file){
        return true;
    }

    return file.peek() == std::ifstream::traits_type::eof();
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

bool ProcessMask(Options &options, std::string &tiff_path, std::string &log_path, std::string &coord_path, int image_idx, ROI &roi) {
    std::vector<std::string> lines = libcee::ReadFileLines(log_path);
    ImageU16L image_in = LoadTiff<ImageU16L>(tiff_path);
    size_t idx = 0;
    std::vector<std::vector<size_t>> neurons; // 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2

    // Read the log file and extract the neuron numbers
    // Making the asssumption that all log files have the neurons in the same order
    for (int i = 0; i < 5; i++) {
        neurons.push_back(std::vector<size_t>());
    }

    for (std::string line : lines) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        if (libcee::ToLower(tokens[0]) == "associate") {
            idx += 1;
        } else {
            neurons[idx].push_back(libcee::FromString<size_t>(tokens[0]));
        }
    }

    // Join all our neurons
    ImageU8L3D neuron_mask(image_in.width, image_in.height / options.depth, options.stacksize);
    bool n1 = false, n2 = false, n3 = false, n4 = false;

    if (options.threeclass) {
        n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, 1);
        n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, 1);
        n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, 2);
        n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, 2);
    } else {
        n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, 1);
        n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, 2);
        n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, 3);
        n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, 4);
    }

    if (!n1 || !n2 || !n3 || !n4) {
        return false;
    }
   

    bool write_csv_header = is_csv_empty(options.output_log_path);

    std::ofstream out_stream; //ofstream is the class for fstream package
    out_stream.open(options.output_log_path, std::ios::app);

    if (write_csv_header) {
        out_stream << "id,p0x,p0y,p0z,p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z" << std::endl;
    }

    // Find the ROI using the mask - Do this on a smaller version of the image for speed.
    ImageU8L3D smaller = Clone(neuron_mask);
    smaller = Resize(neuron_mask, neuron_mask.width / 2, neuron_mask.height/ 2, neuron_mask.depth/ 2);

    // Perform some augmentation by moving the ROI around a bit. Save these augs for the masking
    // that comes later as the mask must match
    float half_roi = static_cast<float>(options.roi_xy ) / 2.0;
    int d = static_cast<int>(ceil(sqrt(2.0f * half_roi * half_roi))) * 2;
    int depth = static_cast<int>(ceil(static_cast<float>(d) / options.depth_scale));

    // Because we are going to AUG, we make the ROI a bit bigger so we can rotate around
    ROI roi_found = FindROI(smaller, d / 2, depth / 2);
    roi.x = roi_found.x * 2;
    roi.y = roi_found.y * 2;
    roi.z = roi_found.z * 2; 
    roi.xy_dim = roi_found.xy_dim * 2;
    roi.depth = roi_found.depth * 2;
    ImageU8L3D cropped = Crop(neuron_mask, roi.x, roi.y, roi.z, roi.xy_dim, roi.xy_dim, roi.depth);
    
    std::vector<std::string> tokens_log = libcee::SplitStringChars(libcee::FilenameFromPath(log_path), "_.");
    std::string image_id = libcee::StringRemove(tokens_log[0], "ID");

    if (options.rename == true) {
        image_id = libcee::IntToStringLeadingZeroes(image_idx, 5);
        std::cout << "Renaming " <<  tiff_path << " to " << image_id << std::endl;
    }

    std::string output_path = options.output_path + "/" + options.prefix + image_id + "_mask.fits";
    std::string output_path_png = options.output_path + "/" + options.prefix + image_id + "_mask.png";
    ImageU8L3D resized = Resize(cropped, options.final_width, options.final_height, options.final_depth);

    SaveFITS(output_path, resized);
     
    // Now write out the graph co-ords
     // Read the dat file and write out the coordinates in order as an entry in a CSV file
    lines = libcee::ReadFileLines(coord_path);
    if(lines.size() != 4) {  return false; }

    // Should be ASI-1, ASI-2, ASJ-1, ASJ-2
    std::vector<glm::vec4> graph;
    std::string csv_line =  libcee::IntToStringLeadingZeroes(image_idx, 5) + ", ";


    for (std::string line : lines) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        std::string x = libcee::RemoveChar(libcee::RemoveChar(tokens[4], ','), ' ');
        std::string y = libcee::RemoveChar(libcee::RemoveChar(libcee::RemoveChar(tokens[3], ','), ' '), '[');
        std::string z = libcee::RemoveChar(libcee::RemoveChar(libcee::RemoveChar(tokens[5], ','), ' '), ']');
        glm::vec4 p = glm::vec4(libcee::FromString<float>(x), libcee::FromString<float>(y), libcee::FromString<float>(z), 1.0f);
        
        // Must have all 4 neurons for now
        if (p.x == 0 && p.y == 0 && p.z == 0) {
            return false;
        }

        float rw = static_cast<float>(options.final_width) / static_cast<float>(cropped.width);
        float rh = static_cast<float>(options.final_height) / static_cast<float>(cropped.height);
        float rd = static_cast<float>(options.final_depth) / static_cast<float>(cropped.depth);

        p.x = (p.x - roi.x) * rw;
        p.y = (p.y - roi.y) * rh;
        p.z = (p.z - roi.z) * rd;

        graph.push_back(p);
    }

    for (int j = 0; j < 4; j++){
        glm::vec4 av = graph[j];
        std::string x = libcee::ToString(av.x);
        std::string y = libcee::ToString(av.y);
        std::string z = libcee::ToString(av.z);
        csv_line += x + ", " + y + ", " + z + ", "; 
    }

    csv_line = libcee::rtrim(csv_line);
    csv_line = libcee::rtrim(csv_line);
    out_stream << csv_line << std::endl;

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

    while ((c = getopt_long(argc, (char **)argv, "i:o:a:l:p:rtbn:z:w:h:s:j:q:?", long_options, &option_index)) != -1) {
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
            case 'b':
                options.bottom = true;
                break;
            case 't' :
                options.threeclass = true;
                break;
            case 'n':
                options.offset_number = libcee::FromString<int>(optarg);
                image_idx = options.offset_number;
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
        }
    }

    //return EXIT_FAILURE;
    std::cout << "Loading annotation images from " << options.annotation_path << std::endl;
    std::cout << "Offset: " << options.offset_number << ", rename: " << options.rename << std::endl;
    std::cout << "Output Size Width: " << options.final_width << ", Height: " << options.final_height << ", Depth: " << options.final_depth << std::endl;

    // First, find and sort the annotation files
    std::vector<std::string> tiff_anno_files = FindAnnotations(options.annotation_path);

    // Next, find the annotation log files
    std::vector<std::string> log_files = FindLogFiles(options.annotation_path);

    // Next, find the annotation dat files
    std::vector<std::string> dat_files = FindDatFiles(options.annotation_path);

    // Now find the input files
    std::cout << "Loading input images from " << options.image_path << std::endl;
    std::cout << "Options: bottom: " << options.bottom << ", Stacksize:" << options.stacksize << std::endl;

    std::vector<std::string> tiff_input_files = FindInputFiles(options.image_path);

    // Pair up the tiffs with their log file and then the input and process them.
    for (std::string tiff_anno : tiff_anno_files) {
        bool paired = false;
        std::vector<std::string> tokens = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_anno), "_.-");
        std::string id = tokens[0];
        std::string mask_path = options.output_path + "/" + options.prefix + libcee::IntToStringLeadingZeroes(image_idx, 5) + "_mask.fits";


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
                                    std::cout << "Stacking: " << tiff_input << std::endl;
                                    if (ProcessMask(options, tiff_anno, log, dat, image_idx, roi)) {
                                        if (TiffToFits(options, tiff_input, image_idx, roi)) {
                                            std::cout << "Pairing " << tiff_anno << " with " << dat << " and " << tiff_input << std::endl;
                                            paired = true;
                                        }
                                    }
                                    image_idx += 1;
                                    break;
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
            remove(mask_path.c_str());
        }
    }

    return EXIT_SUCCESS;

}