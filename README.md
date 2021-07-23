# Wiggle

A set of utilities for working with the Worm Data. 

Typical use would be to point flatten at a directory of images, flattening them into a set of 16bit PNG files with a maximum intensity projection. Then, pointing dataset at the corresponding directory containing the analysis files. The resulting dataset will be ready for use with the U-Net or other dataset.

## flatten

Flatten does several things. It cuts the image into the correct slices, creating either a stacked tiff file, or a 2D projection tiff of somekind, typically a maximum intensity projection.

## dataset

This program works on the analysis files, creating a set of different png images for our neural network. Typically this will be ASI or ASJ masks in 2D, or a set of 3D masks with both neurons combined.

## Building

This project is built with [meson]() and either [gcc]() or [clang](). It requires the following libraries:

* [masamune]()
* [libtiff]()
* [libpng]()

The build commands are as follows:

    meson setup build
    ninja -C build

## Running

There are several options one can pass to both programs. An example run of dataset might be...

    ./build/dataset -f -r -i /media/stuff/Projects/wormz/queelim/Solange_analysis/20170724-QL922_S1-d1.0 -o /media/stuff/Projects/wormz/queelim/dataset

and for flatten...

    ./build/flatten -m -i /media/stuffProjects/wormz/queelim/Solange_analysisImages/20170724-QL922_S1-d1.0 -o /media/stuff/Projects/wormz/queelim/dataset

### Command line parameters

#### flatten

    -i The input image path
    -o The output image path
    -l The number of layers in Z
    -m Perform a maximum intensity projection reduction
    -r Rename the images to a number with leading zeroes
    -n offset for the numbering of images
    -b Use the bottom channel, default is top channel
    -w New width to resize
    -h New height to resize 

#### Dataset

    -f flatten the resulting images rather than having 3D tiffs
    -i path to the input files directory
    -o path to the output files directory
    -n offset for numbering the output files
    -r rename files with numbers as oppose to keeping the original name
    -b use the bottom instead of the top channel