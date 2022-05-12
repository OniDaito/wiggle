# Wiggle

A set of utilities for working with the Worm Data. 

Typical use would be to point flatten at a directory of images, flattening them into a set of 16bit PNG files with a maximum intensity projection. Then, pointing dataset at the corresponding directory containing the analysis files. The resulting dataset will be ready for use with the U-Net or other dataset.

## flatten

Flatten does several things. It cuts the image into the correct slices, creating either a stacked tiff file, or a 2D projection tiff of somekind, typically a maximum intensity projection.

## dataset

This program works on the analysis files, creating a set of different png images for our neural network. Typically this will be ASI or ASJ masks in 2D, or a set of 3D masks with both neurons combined.

## Building

This project is built with [meson]() and either [gcc]() or [clang](). It requires the following libraries:

* [libcee]()
* [imagine]()
* [libtiff]()
* [libpng]()
* [libglfw3]()

Under debian, you can install most of these with the following command

    apt-get install libglfw3-dev libpng-dev libcfitsio-dev libjpeg-dev fftw3-dev libtiff-dev

The build commands are as follows:

    meson setup build
    ninja -C build

For a faster release build:

    meson setup --buildtype=release release
    ninja -C release

## Running

Graph runs as follows:

    ./release/graph -t -r -q 10 -j 200 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180302-QL285-d0.0 -l /phd/wormz/queelim/dataset_aug6/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL285-d0.0 -o /phd/wormz/queelim/dataset_aug6 >> dataset.log

Single runs as follows:

    ./release/single -i /media/proto_backup/wormz/queelim/ins-6-mCherry/20170804-QL923_SB3-d1.0/QL923_SB3-d1.0xAutoStack54.tiff -w /media/proto_backup/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL923_SB3-d1.0/ID54_WS2.tiff -a /media/proto_backup/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL923_SB3-d1.0/ID54_2.log -o /media/proto_backup/wormz/queelim/temp


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
