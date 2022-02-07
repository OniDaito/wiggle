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

For a faster release build:

    meson setup --buildtype release release
    ninja -C release

## Running

multiset

    ./build/multiset -b -r -i /media/stuff/Projects/wormz/queelim/Solange_analysisImages/20170724-QL922_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL285_S1-d1.0 -o /media/stuff/Projects/wormz/queelim/dataset

If we want to get the ROI of these images

    ./build/multiset -b -r -i /media/stuff/Projects/wormz/queelim/Solange_analysisImages/20170724-QL922_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL285_S1-d1.0 -o /media/stuff/Projects/wormz/queelim/dataset -z 25 -w 320 -h 150 -c -j 128 -k 128 -l 25

Counting the flourescence from a saved log file from multiset, repl.acing the /media/proto_backup/wormz/queelim with /phd/wormz/queelim so we get the paths to the dat files right (optional)

    ./fluro -i /media/proto_backup/wormz/queelim/dataset_21_10_2021/dataset.log -p /media/proto_backup/wormz/queelim -m /phd/wormz/queelim > //media/proto_backup/wormz/queelim/dataset_21_10_2021/fluoro.csv

### Command line parameters

For multiset

    -i The input image path
    -o The output image path
    -a The path to the annotation images
    -l The number of layers in Z
    -r Rename the images to a number with leading zeroes
    -n offset for the numbering of images
    -b Use the bottom channel, default is top channel
    -w New width to resize
    -h New height to resize
    -z New depth to resize
    -b use the bottom instead of the top channel
