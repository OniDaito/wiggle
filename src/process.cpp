/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file process.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 18/10/2022
 * @brief Process an input image using the pipeline
 * 
 * Given an original tiff, create a FITS image stack
 * 
 * This program is essential for preparing the data
 * ready for HOLLy.
 *
 * ./build/process -d -u -b -j 200 -q 51 -f -i /media/proto_backup/wormz/queelim/ins-6-mCherry/20170804-QL925_SB3-d1.0/QL925_SB3-d1.0xAutoStack49.tiff -o .
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

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string prefix = "";
    std::string psf_path = "./images/PSF_born_wolf_3d.tif";
    bool rename = false;
    bool flatten = false;
    bool subpixel = true;
    bool interz = true;
    bool drop_last = false;
    bool bottom = false;
    bool autoback = false;
    bool deconv = false;        // Do we deconvolve and all that?
    bool max_intensity = false; // If flattening, use max intensity
    int channels = 2;           // 2 Channels initially in these images
    int stacksize = 51;         // How many stacks in our input 2D image
    int final_depth = 51;       // number of z-slices - TODO - should be set automatically along with width and height
    int final_width = 200;      // The input dimensions of each slice
    int final_height = 200;
    int deconv_rounds = 5;
    size_t roi_xy = 200;        // Square across this dimension
    size_t roi_depth = 21;      // Multiplied by the depth scale later
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

ImageF32L3D ProcessPipe(ImageU16L3D const &image_in,  ROI &roi, bool autoback, float noise, bool deconv, const std::string &psf_path) {

    ImageU16L3D prefinal = Crop(image_in, roi.x, roi.y, roi.z, roi.xy_dim, roi.xy_dim, roi.depth);

    // Convert to float as we need to do some operations
    ImageF32L3D converted = Convert<ImageF32L3D>(prefinal);

    if (autoback){
        // Perform an automatic background subtraction based on local means
        size_t kernel_dia = 3; // A 3 x 3 x 3 kernel to compute the average
        size_t kernel_rad = 1;
        std::vector<int> modes;

        for (uint32_t z = kernel_rad; z < converted.depth - kernel_rad; z++) {
            for (uint32_t y = kernel_rad; y < converted.height - kernel_rad; y++) {
                for (uint32_t x = kernel_rad; x < converted.width - kernel_rad; x++) {
                    // Add up all the pixels - 27 of them
                    float avg = converted.data[z][y][x];
                    // First 9
                    avg += converted.data[z-kernel_rad][y-kernel_rad][x-kernel_rad];
                    avg += converted.data[z-kernel_rad][y-kernel_rad][x];
                    avg += converted.data[z-kernel_rad][y-kernel_rad][x+kernel_rad];
                    avg += converted.data[z-kernel_rad][y][x-kernel_rad];
                    avg += converted.data[z-kernel_rad][y][x];
                    avg += converted.data[z-kernel_rad][y][x+kernel_rad];
                    avg += converted.data[z-kernel_rad][y+kernel_rad][x-kernel_rad];
                    avg += converted.data[z-kernel_rad][y+kernel_rad][x];
                    avg += converted.data[z-kernel_rad][y+kernel_rad][x+kernel_rad];

                    // Mid 8
                    avg += converted.data[z][y-kernel_rad][x-kernel_rad];
                    avg += converted.data[z][y-kernel_rad][x];
                    avg += converted.data[z][y-kernel_rad][x+kernel_rad];
                    avg += converted.data[z][y][x-kernel_rad];
                    avg += converted.data[z][y][x+kernel_rad];
                    avg += converted.data[z][y+kernel_rad][x-kernel_rad];
                    avg += converted.data[z][y+kernel_rad][x];
                    avg += converted.data[z][y+kernel_rad][x+kernel_rad];

                    // Last 9
                    avg += converted.data[z+kernel_rad][y-kernel_rad][x-kernel_rad];
                    avg += converted.data[z+kernel_rad][y-kernel_rad][x];
                    avg += converted.data[z+kernel_rad][y-kernel_rad][x+kernel_rad];
                    avg += converted.data[z+kernel_rad][y][x-kernel_rad];
                    avg += converted.data[z+kernel_rad][y][x];
                    avg += converted.data[z+kernel_rad][y][x+kernel_rad];
                    avg += converted.data[z+kernel_rad][y+kernel_rad][x-kernel_rad];
                    avg += converted.data[z+kernel_rad][y+kernel_rad][x];
                    avg += converted.data[z+kernel_rad][y+kernel_rad][x+kernel_rad];

                    avg /= 27.0;
                    int mode = static_cast<int>(round(avg));
                    modes.push_back(mode);
                }
            }
        }

        typedef std::pair<int, int> mode_pair;

        struct mode_predicate {
            bool operator() (mode_pair const& lhs, mode_pair const& rhs) {
                return lhs.second < rhs.second;
            }
        };

        // Now find the mode of all the values we have
        std::map<int, int> mode_map;

        for (int n = 0; n < modes.size(); n++) {
            mode_map[modes[n]]++;
        }

        mode_predicate mp;
        int final_mode = std::max_element(mode_map.begin(), mode_map.end(), mp)->first;
        std::cout << "Background Value:" << final_mode << std::endl; 
        converted = Sub(converted, static_cast<float>(final_mode), true);

    } else {
        converted = Sub(converted, noise, true);
    }

    // Perform a subtraction on the images, removing background
    if (deconv){ 

        std::vector<std::vector<float>> last (converted.data[converted.depth-1]);
        // Depth typically isnt even so ignore the last when performing deconvolution
        if (converted.depth % 2 != 0) {
            converted.data.pop_back();
            converted.depth -= 1;
        } 

        std::string path_kernel(psf_path);
        ImageF32L3D kernel = LoadTiff<ImageF32L3D>(path_kernel);
        ImageF32L3D deconved = DeconvolveFFT(converted, kernel, 5);

        // Depth typically isnt even so ignore the last when per
        if (converted.depth % 2 != 0) {
            deconved.data.push_back(last);
            deconved.depth += 1;
        } 
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

bool TiffToFits(Options &options, std::string &tiff_path) {
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

   

    std::vector<std::string> tokens_log = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = libcee::StringRemove(image_id, "0xAutoStack");
    std::string output_path = options.output_path + "/" + image_id + "_layered.fits";

    // Because we are going to AUG, we make the ROI a bit bigger so we can rotate around
    ImageU16L3D smaller = Resize(stacked, stacked.width / 2, stacked.height / 2, stacked.depth / 2);
    ROI roi = FindROI(smaller, options.roi_xy / 2, options.roi_depth / 2);
    roi.x = roi.x * 2;
    roi.y = roi.y * 2;
    roi.z = roi.z * 2; 
    roi.xy_dim = options.roi_xy;
    roi.depth = options.roi_depth;

    std::cout << tiff_path << ",ROI," << libcee::ToString(roi.x) << "," << libcee::ToString(roi.y) << "," << libcee::ToString(roi.z) << "," << roi.xy_dim << "," << roi.depth << std::endl;
    ImageF32L3D processed = ProcessPipe(stacked, roi, options.autoback, options.cutoff, options.deconv, options.psf_path);
  
    // Rotate, normalise then sum projection
    output_path = options.output_path + "/" + image_id + "_"  + "_layered.fits";

    if (options.flatten) {
        auto ptype = ProjectionType::SUM;
        
        if (options.max_intensity) {
            ptype = ProjectionType::MAX_INTENSITY;
        }

        ImageF32L summed = Project(processed, ptype);

        if (options.final_width != summed.width || options.final_height != summed.height) {
            summed = Resize(summed, options.final_width, options.final_height);
        }

        // Write a JPG just in case
        ImageU8L jpeged = Convert<ImageU8L>(Convert<ImageF32L>(summed));
        std::string output_path_jpg = options.output_path + "/" +  image_id + "_" + "_raw.jpg";
        SaveJPG(output_path_jpg, jpeged);

        FlipVerticalI(summed);
        SaveFITS(output_path, summed);

  
    } else {
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

    while ((c = getopt_long(argc, (char **)argv, "i:o:fdbmuvz:w:h:c:s:j:q:k:e:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                options.image_path = std::string(optarg);
                break;
            case 'o' :
                options.output_path = std::string(optarg);
                image_idx = GetOffetNumber(options.output_path);
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
            case 'u' :
                options.autoback = true;
                break;
            case 'v':
                options.drop_last = true;
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
            case 'e':
                options.deconv_rounds = libcee::FromString<int>(optarg);
                break;
            case 'g':
                RANDROT_GENERATOR.seed(libcee::FromString<int>(optarg));
                break;
            case 1 :
                options.interz = false;
                break;
            case 2 :
                options.subpixel = false;
                break;
        }
    }

    // Now find the input files
    std::cout << "Loading input images: " << options.image_path << std::endl;

    try {
        TiffToFits(options, options.image_path);
    } catch (const std::exception &e) {
        std::cout << "An exception occured with" << options.image_path << std::endl;
    }
    
    return EXIT_SUCCESS;

}
