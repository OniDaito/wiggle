/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file stack.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 01/06/2021
 * @brief Given QueeLim style tiff stacks, convert to a FITS stack
 * 
 */

#include <getopt.h>
#include <fitsio.h>
#include <masamune/masamune_prog.h>
#include <masamune/util/string.h>
#include <masamune/util/file.h>
#include <masamune/image/tiff.h>
#include <masamune/image/basic.h>

#include <algorithm>

using namespace masamune;

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    size_t num_layers = 51;
    bool flatten = false;
    bool rename = false;
    bool bottom = false;
    int channels = 2; // 2 Channels initially in these images
    int offset_number = 0;
    int width = 640; // The desired dimensions
    int height = 300;
} Options;


void printerror( int status) {
    if (status) {
       fits_report_error(stderr, status);   // print error report
       exit( status );                      // terminate the program, returning error status
    }
    return;
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
    vkn::ImageU16L image;
    vkn::ImageU16L3D flattened;
    image::LoadTiff<vkn::ImageU16L>(tiff_path, image);
    flattened.width = image.width;
    flattened.depth = options.num_layers;
    flattened.height = image.height / (flattened.depth * options.channels); // Two channels
    vkn::Alloc(flattened);
    uint coff = 0;

    if (options.bottom) {
        coff = flattened.height;
    }

    for (uint32_t d = 0; d < flattened.depth; d++) {

        for (uint32_t y = 0; y < flattened.height; y++) {

            for (uint32_t x = 0; x < flattened.width; x++) {
                uint16_t val = image.image_data[(d * flattened.height * options.channels) + coff + y][x];
                flattened.image_data[d][y][x] = val;
            }
        }
    }

    std::vector<std::string> tokens_log = util::SplitStringChars(util::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = util::StringRemove(image_id, "0xAutoStack");
    std::string output_path = options.output_path + "/" + image_id + "_layered.fits";
    WriteFITS(output_path, flattened);

    return true;
}

int main (int argc, char ** argv) {
    // Parse command line options
    Options options;
    int c;
    static struct option long_options[] = {
        {"image-path", 1, 0, 0},
        {"output-path", 1, 0, 0},
        {NULL, 0, NULL, 0}
    };

    int option_index = 0;

    while ((c = getopt_long(argc, (char **)argv, "i:o:l:mrn:bw:h:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                options.image_path = std::string(optarg);
                break;
            case 'o' :
                options.output_path = std::string(optarg);
                break;
            case 'l' :
                options.num_layers = util::FromString<uint32_t>(std::string(optarg));
                break;
            case 'r' :
                options.rename = true;
                break;
            case 'n':
                options.offset_number = util::FromString<int>(optarg);
                break;
            case 'b':
                options.bottom = true;
                break;
            case 'w':
                options.width = util::FromString<int>(optarg);
                break;
            case 'h':
                options.height = util::FromString<int>(optarg);
                break;
        }
    }
   
    //return EXIT_FAILURE;
    std::cout << "Loading images from " << options.image_path << std::endl;

    std::cout << "Options: bottom: " << options.bottom << " width: " << options.width
        << " height: " << options.height << " Z layers: " << options.num_layers << std::endl;
    // Browse the directory looking for files
    std::vector<std::string> files = util::ListFiles(options.image_path);
    std::vector<std::string> tiff_files;

    for (std::string filename : files) {
        if (util::StringContains(filename, ".tif") && util::StringContains(filename, "AutoStack")) {
            tiff_files.push_back(filename);
        }
    }

      // We use a sort based on the last ID number - keeps it inline with the flatten program
    struct {
        bool operator()(std::string a, std::string b) const {
            std::vector<std::string> tokens1 = util::SplitStringChars(util::FilenameFromPath(a), "_.-");
            int ida = util::FromString<int>(util::StringRemove(tokens1[2], "0xAutoStack"));
            std::vector<std::string> tokens2 = util::SplitStringChars(util::FilenameFromPath(b), "_.-");
            int idb = util::FromString<int>(util::StringRemove(tokens2[2], "0xAutoStack"));
            return ida < idb;
        }
    } SortOrder;

    std::sort(tiff_files.begin(), tiff_files.end(), SortOrder);

    // Apparently we are missing one for some reason. Oh well
    // assert(tiff_files.size() == log_files.size());
    for (std::string tiff : tiff_files) {
        // Bottom channel
        std::cout << "Flattening: " << tiff << std::endl;
        TiffToFits(options, tiff);
    }

    return EXIT_SUCCESS;

}