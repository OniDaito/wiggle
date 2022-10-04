/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file graph.cpp
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
    std::string output_log_path = "";
    std::string psf_path = "./images/PSF_born_wolf_3d.tif";
    bool rename = false;
    bool flatten = false;
    bool threeclass = false;    // Forget 1 and 2 and just go with ASI, ASJ or background.
    bool drop_last = false;
    int offset_number = 0;
    bool bottom = false;
    bool deconv = false;         // Do we deconvolve and all that?
    bool max_intensity = false;    // If flattening, use max intensity
    int channels = 2;           // 2 Channels initially in these images
    int stacksize = 51;         // How many stacks in our input 2D image
    int final_depth = 51;             // number of z-slices - TODO - should be set automatically along with width and height
    int final_width = 200;            // The input dimensions of each slice
    int final_height = 200;
    size_t roi_xy = 200;           // Square across this dimension
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

ImageF32L3D ProcessPipe(ImageU16L3D const &image_in,  ROI &roi, float noise, bool deconv, std::string &psf_path) {
    ImageU16L3D prefinal = Crop(image_in, roi.x, roi.y, roi.z, roi.xy_dim, roi.xy_dim, roi.depth);

    // Convert to float as we need to do some operations
    ImageF32L3D converted = Convert<ImageF32L3D>(prefinal);
    // Rather than a straight sub, do an if < noise, set to 0 instead.
    //std::function<float(float)> noise_func = [noise](float x) { if (x < noise) {return 0.0f;} return x; };
    //converted = ApplyFunc<ImageF32L3D, float>(converted, noise_func);
    converted = Sub(converted, noise, true);

    // Perform a subtraction on the images, removing background
    if (deconv){ 
        // Contrast
        //std::function<float(float)> log_func = [](float x) { return std::max(10.0f * log2(x), 0.0f); };
        //converted = ApplyFunc<ImageF32L3D, float>(converted, log_func);

        // Deconvolve with a known PSF
        std::string path_kernel(psf_path);
        ImageF32L3D kernel = LoadTiff<ImageF32L3D>(path_kernel);
        ImageF32L3D deconved = DeconvolveFFT(converted, kernel, 5);
        
        return deconved;
    }
    return converted;
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
    ImageU16L3D stacked(image.width, (image.height / (options.stacksize * options.channels)), options.stacksize);
    uint coff = 0;

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

    if (options.drop_last) {
        stacked.data.pop_back();
        stacked.depth -= 1;
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

    ImageF32L3D processed = ProcessPipe(stacked, roi, options.cutoff, options.deconv, options.psf_path);
  
    // Rotate, normalise then sum projection
    output_path = options.output_path + "/" + image_id + "_"  + "_layered.fits";
    if (options.flatten) {
        auto ptype = ProjectionType::SUM;
        
        if (options.max_intensity) {
            ptype = ProjectionType::MAX_INTENSITY;
        }

        ImageF32L summed = Project(processed, ptype);
        FlipVerticalI(summed);

        if (options.final_width != summed.width || options.final_height != summed.height) {
            summed = Resize(summed, options.final_width, options.final_height);
        }

        SaveFITS(output_path, summed);

        // Write a JPG just in case
        ImageU8L jpeged = Convert<ImageU8L>(Convert<ImageF32L>(summed));
        std::string output_path_jpg = options.output_path + "/" +  image_id + "_" + "_raw.jpg";
        SaveJPG(output_path_jpg, jpeged);
    } else {
        // ImageF32L3D normalised = Normalise(rotated);
        //FlipVerticalI(normalised);
        //ImageF32L3D resized = Resize(normalised, options.final_width, options.final_height, options.final_depth);
        FlipVerticalI(processed);
        if (options.final_width != processed.width || options.final_height != processed.height || options.final_depth != processed.depth) {
            if (processed.depth % 2 == 1) {
                processed.data.pop_back();
                processed.depth -= 1;
            }
            ImageF32L3D resized = Resize(processed, options.final_width, options.final_height, options.final_depth);
            SaveFITS(output_path, resized);
        } else {
             SaveFITS(output_path, processed);   
        }
    }
            
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
 * @return
 */

bool ProcessMask(Options &options, std::string &tiff_path, std::string &log_path, std::string &coord_path, int image_idx, ROI &roi) {
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

    if (options.drop_last) {
        neuron_mask.data.pop_back();
        neuron_mask.depth -= 1;
    } 

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

    bool write_csv_header = is_csv_empty(options.output_log_path);

    std::ofstream out_stream; //ofstream is the class for fstream package
    out_stream.open(options.output_log_path, std::ios::app); 

    if (write_csv_header) {
        out_stream << "id,p0x,p0y,p0z,fluor0,back0,mode0,minfluor0,size0,p1x,p1y,p1z,fluor1,back1,mode1,minfluor1,size1,p2x,p2y,p2z,fluor2,back2,mode2,minfluor2,size2,p3x,p3y,p3z,fluor3,back3,mode3,minfluor3,size3,orig" << std::endl;
    }


    // Find the ROI using the mask - Do this on a smaller version of the image for speed.

    // Perform some augmentation by moving the ROI around a bit. Save these augs for the masking
    // that comes later as the mask must match
    // ROI is larger here than final as we need 'rotation' space
    int half_roi_xy = static_cast<int>(ceil(static_cast<float>(options.roi_xy) / 2.0));
    int half_roi_depth = static_cast<int>(ceil(static_cast<float>(options.roi_depth) / 2.0));

    // Because we are going to AUG, we make the ROI a bit bigger so we can rotate around
    ImageU8L3D smaller = Resize(neuron_mask, static_cast<int>(ceil(static_cast<float>(neuron_mask.width) / 2.0)), 
                                    static_cast<int>(ceil(static_cast<float>(neuron_mask.height) / 2.0)), 
                                    static_cast<int>(ceil(static_cast<float>(neuron_mask.depth) / 2.0)));


    
    ROI roi_found = FindROI(smaller, half_roi_xy, half_roi_depth);
    roi.x = roi_found.x * 2;
    roi.y = roi_found.y * 2;
    roi.z = roi_found.z * 2;
    roi.xy_dim = options.roi_xy;
    roi.depth = options.roi_depth;

    // Check on ROI-Z as it's likely to not shift
    if (options.roi_depth == neuron_mask.depth) {
        roi.z = 0;
    }
 
    std::cout << tiff_path << ",ROI," << libcee::ToString(roi.x) << "," << libcee::ToString(roi.y) << "," << libcee::ToString(roi.z) << "," << roi.xy_dim << "," << roi.depth << std::endl;
    ImageU8L3D cropped = Crop(neuron_mask, roi.x, roi.y, roi.z, roi.xy_dim, roi.xy_dim, roi.depth);
    FlipVerticalI(cropped);

    // Read the dat file and write out the coordinates in order as an entry in a CSV file
    std::vector<std::string> lines = libcee::ReadFileLines(coord_path);
    if(lines.size() != 4) {  return false; }

    // Should be ASI-1, ASI-2, ASJ-1, ASJ-2
    std::vector<glm::vec4> graph;
    std::vector<std::string> extra_data;

    // Get the required data from the .dat files in the annotation
    for (std::string line : lines) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        std::string x = libcee::RemoveChar(libcee::RemoveChar(tokens[4], ','), ' ');
        std::string y = libcee::RemoveChar(libcee::RemoveChar(libcee::RemoveChar(tokens[3], ','), ' '), '[');
        std::string z = libcee::RemoveChar(libcee::RemoveChar(libcee::RemoveChar(tokens[5], ','), ' '), ']');
        glm::vec4 p = glm::vec4(libcee::FromString<float>(x), libcee::FromString<float>(y), libcee::FromString<float>(z), 1.0f);
    
        std::string fl = libcee::RemoveChar(libcee::RemoveChar(tokens[1], ','), ' ');
        std::string mode_fl = libcee::RemoveChar(libcee::RemoveChar(tokens[6], ','), ' ');
        std::string min_fl = libcee::RemoveChar(libcee::RemoveChar(tokens[7], ','), ' ');
        std::string bg = libcee::RemoveChar(libcee::RemoveChar(tokens[2], ','), ' ');
        std::string rsize = libcee::RemoveChar(libcee::RemoveChar(tokens[8], ','), ' ');

        std::string extra = fl + "," + bg + "," + mode_fl + "," + min_fl + "," + rsize;
        extra_data.push_back(extra);

        // Must have all 4 neurons for now
        if (p.x == 0 && p.y == 0 && p.z == 0) {
            return false;
        }

        p.x = (p.x - roi.x);
        p.y = (p.y - roi.y);
        p.z = (p.z - roi.z);
        graph.push_back(p);
    }

    float rw = static_cast<float>(options.final_width) / static_cast<float>(options.roi_xy);
    float rh = static_cast<float>(options.final_height) / static_cast<float>(options.roi_xy);
    float rd = static_cast<float>(options.final_depth) / static_cast<float>(options.roi_depth);

    // Save the masks
    std::string csv_line =  libcee::IntToStringLeadingZeroes(image_idx, 5) + ", ";
    std::string output_path = options.output_path + "/" +  libcee::IntToStringLeadingZeroes(image_idx, 5) + "_mask.fits";
    ImageU8L mipped = Project(cropped, ProjectionType::MAX_INTENSITY);

    // Not sure why the inverse. GLM versus our sampling I suppose
    ImageU8L resized = Resize(mipped, options.final_width, options.final_height);

    if (options.flatten){
        SaveFITS(output_path, resized);
    } else {
        if (options.final_width != cropped.width || options.final_depth != cropped.depth || options.final_height != cropped.height) {
            if (cropped.depth % 2 == 1) {
                cropped.data.pop_back();
            }

            ImageU8L3D resized3d = Resize(cropped, options.final_width, options.final_height, options.final_depth);
            SaveFITS(output_path, resized3d);
        } else {
            SaveFITS(output_path, cropped);
        }
    }

    // Write a JPG just in case
    ImageU8L jpeged = Convert<ImageU8L>(Convert<ImageF32L>(resized));
    std::string output_path_jpg = options.output_path + "/" +  libcee::IntToStringLeadingZeroes(image_idx, 5) + "_mask.jpg";
    SaveJPG(output_path_jpg, jpeged);

    // Write out to the CSV file
    for (int j = 0; j < 4; j++){
        glm::vec4 av = graph[j];
        std::string x = libcee::ToString(av.x * rw);
        std::string y = libcee::ToString((static_cast<float>(options.roi_xy) - av.y) * rh);
        std::string z = libcee::ToString(av.z * rd);
        csv_line += x + "," + y + "," + z + "," + extra_data[j] + ",";
    }

    csv_line += tiff_path;
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

    while ((c = getopt_long(argc, (char **)argv, "i:o:a:p:rtfdbmvn:z:w:h:l:c:s:j:q:k:?", long_options, &option_index)) != -1) {
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
            case 'd':
                options.deconv = true;
                break;
            case 'f':
                options.flatten = true;
                break;
            case 'm':
                options.max_intensity = true;
                break;
            case 'v':
                options.drop_last = true;
                break;
            case 't' :
                options.threeclass = true;
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
                options.roi_depth = libcee::FromString<int>(optarg);
                break;
            case 'k' :
                options.psf_path = std::string(optarg);
                break;
        }
    }

    //return EXIT_FAILURE;
    std::cout << "Loading annotation images from " << options.annotation_path << std::endl;
    std::cout << "Offset: " << options.offset_number << ", rename: " << options.rename << std::endl;

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
                                    if (ProcessMask(options, tiff_anno, log, dat, image_idx, roi)) {
                                        std::cout << "Stacking: " << tiff_input << std::endl;
                                        TiffToFits(options, tiff_input, image_idx, roi);
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
