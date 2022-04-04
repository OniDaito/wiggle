/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file image.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/02/2022
 * @brief Useful image functions.
 * 
 *
 */

#include "image.hpp"

using namespace imagine;


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
bool SetNeuron(ImageU16L &image_in, ImageU8L3D &image_out, std::vector<std::vector<size_t>> &neurons, int neuron_id, bool flip_depth, int id_to_write) {
    bool neuron_set = false;

    for (uint32_t d = 0; d < image_out.depth; d++) {

        for (uint32_t y = 0; y < image_out.height; y++) {

            for (uint32_t x = 0; x < image_out.width; x++) {

                size_t channel = d * image_out.height;
                uint16_t val = image_in.data[channel + y][x];
                
                if (val != 0) {
                    std::vector<size_t>::iterator it = std::find(neurons[neuron_id].begin(),
                        neurons[neuron_id].end(), static_cast<size_t>(val));
                    if (it != neurons[neuron_id].end()) {
                        neuron_set = true;
                        uint8_t nval = id_to_write;
                        if (flip_depth) {
                            image_out.data[image_out.depth - d - 1][y][x] = nval;
                        } else {
                            image_out.data[d][y][x] = nval;
                        }
                    } 
                }
            }
        }
    }
    return neuron_set;
}

/**
 * Given a 3D image, flatten it.
 * 
 * @param mask - a 3D image
 * 
 * @return a 2D vkn image
 */
ImageU8L Flatten(ImageU8L3D &mask) {
    ImageU8L flat (mask.width, mask.height);

    for (uint32_t d = 0; d < mask.depth; d++) {
        for (uint32_t y = 0; y < flat.height; y++) {
            for (uint32_t x = 0; x < flat.width; x++) {
                uint16_t val = mask.data[d][y][x];
                uint16_t ext = flat.data[y][x];
                if (ext == 0) {
                    flat.data[y][x] = val;
                }
            }
        }
    }
    return flat;
}


/**
 * Given a 3D image, flatten it.
 * 
 * @param mask - a 3D image
 * 
 * @return a 2D vkn image
 */
ImageF32L Flatten(ImageF32L3D &image) {
    ImageF32L flat(image.width, image.height);

    for (uint32_t d = 0; d < image.depth; d++) {
        for (uint32_t y = 0; y < flat.height; y++) {
            for (uint32_t x = 0; x < flat.width; x++) {
                uint16_t val = image.data[d][y][x];
                uint16_t ext = flat.data[y][x];
                if (ext == 0) {
                    flat.data[y][x] = val;
                }
            }
        }
    }
    return flat;
}

// TODO - writeFITS should probably go into masamune eventually

void WriteFITS( std::string filename, ImageU16L3D flattened) {
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
    if (fits_write_img(fptr, TUSHORT, fpixel, nelements, &(Flatten(flattened, false)[0]), &status)) {
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


void WriteFITS( std::string filename, ImageF32L3D flattened) {
    fitsfile *fptr; 
    int status, ii, jj;
    long  fpixel, nelements, exposure;

    // initialize FITS image parameters
    int bitpix   =  FLOAT_IMG; // 16-bit unsigned short pixel values
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
    if (fits_write_img(fptr, TFLOAT, fpixel, nelements, &(Flatten(flattened, false)[0]), &status)) {
        printerror( status );
    }

    if (fits_close_file(fptr, &status)) {
        printerror( status );      
    }       
              
    return;
}


void WriteFITS( std::string filename, ImageU16L flattened) {
    fitsfile *fptr; 
    int status, ii, jj;
    long  fpixel, nelements, exposure;

    // initialize FITS image parameters
    int bitpix   =  USHORT_IMG; // 16-bit unsigned short pixel values
    long naxis    =   2;        // 2D
    long naxes[2] = { flattened.width, flattened.height }; 

    remove(filename.c_str());   // Delete old file if it already exists
    status = 0;                 // initialize status before calling fitsio routines

    if (fits_create_file(&fptr, filename.c_str(), &status)) {
         printerror( status );
    } 


    if (fits_create_img(fptr,  bitpix, naxis, naxes, &status)) {
        printerror( status ); 
    }
                  
    fpixel = 1;                                  // first pixel to write
    nelements = naxes[0] * naxes[1];  // number of pixels to write

    // write the array of unsigned integers to the FITS file
    if (fits_write_img(fptr, TUSHORT, fpixel, nelements, &(Flatten(flattened, false)[0]), &status)) {
        printerror( status );
    }

    if (fits_close_file(fptr, &status)) {
        printerror( status );      
    }       
              
    return;
}


void WriteFITS(std::string filename, ImageF32L flattened) {
    fitsfile *fptr; 
    int status, ii, jj;
    long  fpixel, nelements, exposure;

    // initialize FITS image parameters
    int bitpix   =  FLOAT_IMG; // 32 bit Float values
    long naxis    =   2;        // 2D
    long naxes[2] = { flattened.width, flattened.height }; 

    remove(filename.c_str());   // Delete old file if it already exists
    status = 0;                 // initialize status before calling fitsio routines

    if (fits_create_file(&fptr, filename.c_str(), &status)) {
         printerror( status );
    } 

    if (fits_create_img(fptr,  bitpix, naxis, naxes, &status)) {
        printerror( status ); 
    }
                  
    fpixel = 1;                                  // first pixel to write
    nelements = naxes[0] * naxes[1];  // number of pixels to write

    // write the array of unsigned integers to the FITS file
    if (fits_write_img(fptr, TFLOAT, fpixel, nelements, &(Flatten(flattened, false)[0]), &status)) {
        printerror( status );
    }

    if (fits_close_file(fptr, &status)) {
        printerror( status );      
    }       
              
    return;
}


void WriteFITS( std::string filename, ImageU8L3D flattened) {
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
    if (fits_write_img(fptr, TBYTE, fpixel, nelements, &(Flatten(flattened, false)[0]), &status)) {
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
 * @param image - ImageU8L3D
 *
 * @return bool
 */

bool non_zero(ImageU8L3D &image) {

    for (uint32_t d = 0; d < image.depth; d++) {
        for (uint32_t y = 0; y < image.height; y++) {
            for (uint32_t x = 0; x < image.width; x++) {
                uint16_t val = image.data[d][y][x];
       
                if (val != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}