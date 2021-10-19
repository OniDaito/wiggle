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

#include <getopt.h>
#include <fitsio.h>
#include <masamune/masamune_prog.h>
#include <masamune/util/string.h>
#include <masamune/util/file.h>
#include <masamune/image/tiff.h>
#include <masamune/image/basic.h>
#include <vector>

using namespace masamune;

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string annotation_path = ".";
    std::string prefix = "";
    bool flatten = false;
    bool rename = false;
    int offset_number = 0;
    bool bottom = false;
    int channels = 2; // 2 Channels initially in these images
    int depth = 51; // number of z-slices - TODO - should be set automatically along with width and height
    int width = 640; // The desired dimensions
    int height = 300;
    int stacksize = 51; // How many stacks in our input 2D image
} Options;


void printerror( int status) {
    if (status) {
       fits_report_error(stderr, status);   // print error report
       exit( status );                      // terminate the program, returning error status
    }
    return;
}

/**
 * Check the area id against the neuron list, setting it to what it claims to be.
 * Look at one channel only though, top or bottom
 */
void SetNeuron(vkn::ImageU16L &image_in, vkn::ImageU8L3D &image_out,
    std::vector<std::vector<size_t>> &neurons, int neuron_id, bool flip_depth) {

    for (uint32_t d = 0; d < image_out.depth; d++) {

        for (uint32_t y = 0; y < image_out.height; y++) {

            for (uint32_t x = 0; x < image_out.width; x++) {

                size_t channel = d * image_out.height;
                uint16_t val = image_in.image_data[channel + y][x];
                
                if (val != 0) {
                    std::vector<size_t>::iterator it = std::find(neurons[neuron_id].begin(),
                        neurons[neuron_id].end(), static_cast<size_t>(val));
                    if (it != neurons[neuron_id].end()) {
                        uint8_t nval = neuron_id;
                        if (flip_depth) {
                            image_out.image_data[image_out.depth - d - 1][y][x] = nval;
                        } else {
                            image_out.image_data[d][y][x] = nval;
                        }
                    } 
                }
            }
        }
    }
}

/**
 * Given a 3D image, flatten it.
 * 
 * @param mask - a 3D image
 * 
 * @return a 2D vkn image
 */
vkn::ImageU8L Flatten(vkn::ImageU8L3D &mask) {
    vkn::ImageU8L flat;
    flat.width = mask.width;
    flat.height = mask.height;
    vkn::Alloc(flat);

    for (uint32_t d = 0; d < mask.depth; d++) {
        for (uint32_t y = 0; y < flat.height; y++) {
            for (uint32_t x = 0; x < flat.width; x++) {
                uint16_t val = mask.image_data[d][y][x];
                uint16_t ext = flat.image_data[y][x];
                if (ext == 0) {
                    flat.image_data[y][x] = val;
                }
            }
        }
    }
    return flat;
}


void WriteFITS( std::string filename, vkn::ImageU16L3D flattened) {
    fitsfile *fptr; 
    int status, ii, jj;
    long  fpixel, nelements, exposure;

    // initialize FITS image parameters
    int bitpix   =  USHORT_IMG; // 16-bit unsigned short pixel values
    long naxis    =   3;        // 3D
    long naxes[3] = { flattened.width, flattened.height, flattened.depth }; 

    remove(filename.c_str());   // Delete old file if it already exists
    status = 0;                 // initialize status before calling fitsio routines

    if (fits_create_file(&fptr, filename.c_str(), &status)) {
         printerror( status );
    } 

    /* write the required keywords for the primary array image.     */
    /* Since bitpix = USHORT_IMG, this will cause cfitsio to create */
    /* a FITS image with BITPIX = 16 (signed short integers) with   */
    /* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
    /* FITS uses to store unsigned integers.  Note that the BSCALE  */
    /* and BZERO keywords will be automatically written by cfitsio  */
    /* in this case.                                                */

    if (fits_create_img(fptr,  bitpix, naxis, naxes, &status)) {
        printerror( status ); 
    }
                  
    fpixel = 1;                                  // first pixel to write
    nelements = naxes[0] * naxes[1] * naxes[2];  // number of pixels to write

    // write the array of unsigned integers to the FITS file
    if (fits_write_img(fptr, TUSHORT, fpixel, nelements, &(vkn::Flatten(flattened)[0]), &status)) {
        printerror( status );
    }

    /* write another optional keyword to the header */
    /* Note that the ADDRESS of the value is passed in the routine */
    /* exposure = 1500;
    if ( fits_update_key(fptr, TLONG, "EXPOSURE", &exposure,
         "Total Exposure Time", &status) )
         printerror( status );           */

    if (fits_close_file(fptr, &status)) {
        printerror( status );      
    }       
              
    return;
}


void WriteFITS( std::string filename, vkn::ImageU8L3D flattened) {
    fitsfile *fptr; 
    int status, ii, jj;
    long  fpixel, nelements, exposure;

    // initialize FITS image parameters
    int bitpix   =  BYTE_IMG; // 8-bit unsigned short pixel values
    long naxis    =   3;        // 3D
    long naxes[3] = { flattened.width, flattened.height, flattened.depth }; 

    remove(filename.c_str());   // Delete old file if it already exists
    status = 0;                 // initialize status before calling fitsio routines

    if (fits_create_file(&fptr, filename.c_str(), &status)) {
         printerror( status );
    } 

    if (fits_create_img(fptr,  bitpix, naxis, naxes, &status)) {
        printerror( status ); 
    }
                  
    fpixel = 1;                                  // first pixel to write
    nelements = naxes[0] * naxes[1] * naxes[2];  // number of pixels to write

    // write the array of unsigned integers to the FITS file
    if (fits_write_img(fptr, TBYTE, fpixel, nelements, &(vkn::Flatten(flattened)[0]), &status)) {
        printerror( status );
    }

    if (fits_close_file(fptr, &status)) {
        printerror( status );      
    }       
              
    return;
}

/**
 * Return false if all elements are zero
 *
 * @param image - vkn::ImageU8L3D
 *
 * @return bool
 */

bool non_zero(vkn::ImageU8L3D &image) {

    for (uint32_t d = 0; d < image.depth; d++) {
        for (uint32_t y = 0; y < image.height; y++) {
            for (uint32_t x = 0; x < image.width; x++) {
                uint16_t val = image.image_data[d][y][x];
       
                if (val != 0) {
                    return true;
                }
            }
        }
    }
    return false;
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

bool TiffToFits(Options &options, std::string &tiff_path, int image_idx) {
    vkn::ImageU16L image;
    vkn::ImageU16L3D stacked;
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

    // std::string output_path = options.output_path + "/" + image_id + "_mip.tiff";
    // image::SaveTiff(output_path, flattened);
    /*if (flattened.width != options.width || flattened.height != options.height) {
        std::cout << "Resizing " << output_path << std::endl;
        image::Resize(flattened, options.width, options.height);
    }*/

    // Perform a resize with nearest neighbour sampling if we have different sizes.
    if (options.width != stacked.width || options.height != stacked.height || options.depth != stacked.depth) {
        vkn::ImageU16L3D resized = image::Resize(stacked, options.width, options.height, options.depth);
        WriteFITS(output_path, resized);
    } else {
        WriteFITS(output_path, stacked);
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

    SetNeuron(image_in, neuron_mask, neurons, 1, !options.flatten);
    SetNeuron(image_in, neuron_mask, neurons, 2, !options.flatten);
    SetNeuron(image_in, neuron_mask, neurons, 3, !options.flatten);
    SetNeuron(image_in, neuron_mask, neurons, 4, !options.flatten);

    std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(log_path), "_.");
    std::string image_id = util::StringRemove(tokens_log[0], "ID");

    if (options.rename == true) {
        image_id = util::IntToStringLeadingZeroes(image_idx, 5);
        std::cout << "Renaming " <<  tiff_path << " to " << image_id << std::endl;
    }

    std::string output_path = options.output_path + "/" + options.prefix + image_id + "_mask.fits";
    std::string output_path_png = options.output_path + "/" + options.prefix + image_id + "_mask.png";

    if (non_zero(neuron_mask)) {

        //image::SaveTiff(output_path, asi);
        if (options.flatten){
            vkn::ImageU8L neuron_mask_flat = Flatten(neuron_mask);
            if (neuron_mask_flat.width != options.width || neuron_mask_flat.height != options.height) {
                image::Resize(neuron_mask_flat, options.width, options.height);
            }
            //image::SaveTiff(output_path, asi_flat);
            vkn::ImageU8L neuron_mask_flip = image::MirrorVertical(neuron_mask_flat);
            image::Save(output_path_png, neuron_mask_flip);

        } else {
            // image::SaveTiff(output_path, asi);
            if (neuron_mask.width != options.width || neuron_mask.height != options.height || neuron_mask.depth != options.depth) {
                 vkn::ImageU8L3D resized = image::Resize(neuron_mask, options.width, options.height, options.depth);
                 WriteFITS(output_path, resized);
            } else {
                WriteFITS(output_path, neuron_mask);
            }
        } 
    }

    image_idx +=1;
    return true;
}


/**
 * Given a path to the output, return the index for the next
 * run.
 *
 * @param image_path - the output path
 *
 * @return std::vector<std::string> a list of log files
 */

int GetOffetNumber(std::string output_path) {
    std::vector<std::string> files = util::ListFiles(output_path);
    int count  = 0;

    for (std::string filename : files) {
        if (util::StringContains(filename, "layered")) {
            count += 1;
        }
    }

    return count;
}


/**
 * Given a path to the annotations directory find the log files
 * that hold the data we need.
 *
 * @param annotation_path - the file path to the annotations
 *
 * @return std::vector<std::string> a list of log files
 */

std::vector<std::string> FindLogFiles(std::string annotation_path) {
    std::vector<std::string> log_files;
    std::vector<std::string> files_anno = util::ListFiles(annotation_path);

    for (std::string filename : files_anno) {
        if (util::StringContains(filename, ".log")) {
            log_files.push_back(filename);
        }
    }

    return log_files;
}


/**
 * Given a path to the annotations directory find the tiff 
 * images with the watershedded categories.
 *
 * @param annotation_path - the file path to the annotations
 *
 * @return std::vector<std::string> a list of tiff files
 */

std::vector<std::string> FindAnnotations(std::string annotation_path) {
    std::vector<std::string> files_anno = util::ListFiles(annotation_path);
    std::vector<std::string> tiff_anno_files;

    for (std::string filename : files_anno) {
        if (util::StringContains(filename, ".tif") && util::StringContains(filename, "ID")  && util::StringContains(filename, "WS")) {
            tiff_anno_files.push_back(filename);
        }
    }

    // Apparently we are missing one for some reason. Oh well
    // assert(tiff_files.size() == log_files.size());

    // We use a sort based on the last ID number - keeps it inline with the flatten program
    struct {
        bool operator()(std::string a, std::string b) const {
            std::vector<std::string> tokens1 = util::SplitStringChars(util::FilenameFromPath(a), "_.-");
            int idx = 0;
            for (std::string t : tokens1) {
                if (util::StringContains(t, "ID")){
                    break;
                }
                idx += 1;
            }
            int ida = util::FromString<int>(util::StringRemove(tokens1[idx], "ID"));
            std::vector<std::string> tokens2 = util::SplitStringChars(util::FilenameFromPath(b), "_.-");
            int idb = util::FromString<int>(util::StringRemove(tokens2[idx], "ID"));
            return ida < idb;
        }
    } SortOrderAnno;

    std::sort(tiff_anno_files.begin(), tiff_anno_files.end(), SortOrderAnno);
    return tiff_anno_files;
}

/**
 * Given a path to the raw input directory find the tiff images
 * that hold the data we need.
 *
 * @param image - the file path to the raw input data
 *
 * @return std::vector<std::string> a list of log files
 */

std::vector<std::string> FindInputFiles(std::string image_path) {
    // Browse the directory looking for files
    std::vector<std::string> files_input = util::ListFiles(image_path);
    std::vector<std::string> tiff_input_files;

    for (std::string filename : files_input) {
        if (util::StringContains(filename, ".tif") && util::StringContains(filename, "AutoStack")) {
            tiff_input_files.push_back(filename);
        }
    }

    // We use a sort based on the last ID number - keeps it inline with the sort above
    struct {
        bool operator()(std::string a, std::string b) const {
            std::vector<std::string> tokens1 = util::SplitStringChars(util::FilenameFromPath(a), "_.-");
            int idx = 0;
            for (std::string t : tokens1) {
                if (util::StringContains(t, "AutoStack")) {
                    break;
                }
                idx += 1;
            }

            int ida = util::FromString<int>(util::StringRemove(tokens1[idx], "0xAutoStack"));
            std::vector<std::string> tokens2 = util::SplitStringChars(util::FilenameFromPath(b), "_.-");
            int idb = util::FromString<int>(util::StringRemove(tokens2[idx], "0xAutoStack"));
            return ida < idb;
        }
    } SortOrderInput;

    std::sort(tiff_input_files.begin(), tiff_input_files.end(), SortOrderInput);
    return tiff_input_files;
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

    while ((c = getopt_long(argc, (char **)argv, "i:o:a:p:frbn:z:w:h:s:?", long_options, &option_index)) != -1) {
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
                            std::cout << "Pairing " << tiff_anno << " with " << log << " and " << tiff_input << std::endl;
                            paired = true;
                            ProcessTiff(options, tiff_anno, log, image_idx);
                            std::cout << "Stacking: " << tiff_input << std::endl;
                            TiffToFits(options, tiff_input, image_idx);
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