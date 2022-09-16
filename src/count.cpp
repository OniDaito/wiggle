/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file count.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/02/2022
 * @brief Parse tiff files, count the flourescence
 * 
 * Given a directory of worm information, create the masks
 * and do a count of the flourescence.
 * 
 * Internally we use the following codes for the neurons
 * 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2
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

typedef struct {
    int64_t asi1;
    int64_t asi2;
    int64_t asj1;
    int64_t asj2;
} Counts;

typedef struct {
    int64_t asi1;
    int64_t asi2;
    int64_t asj1;
    int64_t asj2;
    int64_t asi1_mode;
    int64_t asi2_mode;
    int64_t asj1_mode;
    int64_t asj2_mode;

} BaseCounts;


using namespace imagine;

/**
 * Given a tiff file return the same file but with one channel and 
 * as a series of layers
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 *
 * @return ImageU16L3D
 */

ImageU16L3D TiffToStack(Options &options, std::string &tiff_path) {
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
            for (uint32_t x = 0; x < stacked.width; x++) {
                uint16_t val = image.data[(d * stacked.height * options.channels) + coff + y][x];
                stacked.data[d][y][x] = val;
            }
        }
    }
         
    return stacked;
}


bool is_csv_empty(std::string path) {
    std::ifstream file(path);
    if (!file){
        return true;
    }

    return file.peek() == std::ifstream::traits_type::eof();
}


BaseCounts GetCSVCounts(std::string &coord_path) {
    BaseCounts counts = {0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<std::string> lines = libcee::ReadFileLines(coord_path);
    if(lines.size() != 4) {  return counts; }

    for (std::string line : lines) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        std::string n = libcee::RemoveChar(libcee::RemoveChar(tokens[0], ','), ' ');
        uint64_t c = libcee::FromString<int64_t>(tokens[1]);
        uint64_t m = libcee::FromString<int64_t>(tokens[6]);
        if (n == "ASI-1") { counts.asi1 = c; counts.asi1_mode = m; }
        if (n == "ASI-2") { counts.asi2 = c; counts.asi2_mode = m; }
        if (n == "ASJ-1") { counts.asj1 = c; counts.asj1_mode = m; }
        if (n == "ASJ-2") { counts.asj2 = c; counts.asj2_mode = m; }
    }
    return counts;
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

ImageU8L3D ProcessMask(Options &options, std::string &tiff_path, std::string &log_path) {
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

    n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, true, 1);
    n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, true, 2);
    n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, true, 3);
    n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, true, 4);

    return neuron_mask;
}

Counts GetCount(const ImageU16L3D &raw, const ImageU8L3D &mask,  const BaseCounts &base){
    Counts counts = {0, 0, 0, 0};

    // We can computed in the same way that neuroshed does but for now, I'm using all the mask
    std::vector<uint16_t> asi1;
    std::vector<uint16_t> asi2;
    std::vector<uint16_t> asj1;
    std::vector<uint16_t> asj2;

    for (size_t z = 0; z < raw.depth; z++) {
        for (size_t y = 0; y < raw.height; y++) {
            for (size_t x = 0; x < raw.width; x++) {
                uint8_t m = mask.data[z][y][x];

                if (m == 1) { asi1.push_back(static_cast<uint16_t>(raw.data[z][y][x])); }
                else if (m == 2) { asi2.push_back(static_cast<uint16_t>(raw.data[z][y][x])); }
                else if (m == 3) { asj1.push_back(static_cast<uint16_t>(raw.data[z][y][x])); }
                else if (m == 4) { asj2.push_back(static_cast<uint16_t>(raw.data[z][y][x])); }
            }
        }
    }

    // Uncomment the below to do the neuroshed count
    /*
    std::sort(asi1.begin(), asi1.end());
    std::sort(asi2.begin(), asi2.end());
    std::sort(asj1.begin(), asj1.end());
    std::sort(asj2.begin(), asj2.end());

    std::reverse(asi1.begin(), asi1.end());
    std::reverse(asi2.begin(), asi2.end());
    std::reverse(asj1.begin(), asj1.end());
    std::reverse(asj2.begin(), asj2.end());

    asi1.resize(4000, 0);
    asi2.resize(4000, 0);
    asj1.resize(4000, 0);
    asj2.resize(4000, 0);

    for (size_t i = 0; i < 4000; i++) {
        counts.asi1 += static_cast<int64_t>(asi1[i]) - base.asi1_mode;
        counts.asi2 += static_cast<int64_t>(asi2[i]) - base.asi2_mode;
        counts.asj1 += static_cast<int64_t>(asj1[i]) - base.asj1_mode;
        counts.asj2 += static_cast<int64_t>(asj2[i]) - base.asj2_mode;
    }*/

    // And comment out the 6 lines below 
  
    for (size_t i = 0; i < asi1.size(); i++) { counts.asi1 += static_cast<int64_t>(asi1[i]); }
    for (size_t i = 0; i < asi2.size(); i++) { counts.asi2 += static_cast<int64_t>(asi2[i]); }
    for (size_t i = 0; i < asj1.size(); i++) { counts.asj1 += static_cast<int64_t>(asj1[i]); }
    for (size_t i = 0; i < asj2.size(); i++) { counts.asj2 += static_cast<int64_t>(asj2[i]); }

    // std::cout << "Hishest Scores " <<  asi1[0] << " " <<  asi2[0] << " " << asj1[0] << " " <<  asj2[0] << std::endl;
    std::cout << "Counts " << counts.asi1 << " " << counts.asi2 << " "  << counts.asj1 << " "  << counts.asj2 << std::endl;
    return counts;
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

    while ((c = getopt_long(argc, (char **)argv, "i:o:a:l:p:bt?", long_options, &option_index)) != -1) {
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
            case 'b':
                options.bottom = true;
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

    bool write_csv_header = is_csv_empty(options.output_log_path);

    std::ofstream out_stream; //ofstream is the class for fstream package
    out_stream.open(options.output_log_path, std::ios::app);

    if (write_csv_header) {
        out_stream << "fileraw,filemask,asi1,asi2,asj1,asj2,basi1,basi2,basj1,basj2" << std::endl;
    }

    // Pair up the tiffs with their log file and then the input and process them.
    for (std::string tiff_anno : tiff_anno_files) {
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
                                    std::cout << "Pairing " << tiff_anno << " with " << dat << " and " << tiff_input << std::endl;
                                    ImageU8L3D mask = ProcessMask(options, tiff_anno, log);
                                    BaseCounts base_count = GetCSVCounts(dat);
                                    ImageU16L3D raw_data = TiffToStack(options, tiff_input);
                                    Counts count = GetCount(raw_data, mask, base_count);
                                    out_stream << tiff_input << "," << tiff_anno << "," << count.asi1 << "," << count.asi2 << "," << count.asj1 << "," << count.asj2 << ","
                                        << base_count.asi1 << "," << base_count.asi2 << "," << base_count.asj1 << "," << base_count.asj2 << std::endl;
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
    }

    return EXIT_SUCCESS;

}