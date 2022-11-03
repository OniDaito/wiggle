# Wiggle

A set of utilities for working with the Worm Data from QueeLim Ch'ng's lab. The data consists of a number of tiffs with a corresponding set of annotations, held in directories with a similar name to the source directories.

Typical use would be to point flatten at a directory of images, flattening them into a set of 16bit PNG files with a maximum intensity projection. Then, pointing dataset at the corresponding directory containing the analysis files. The resulting dataset will be ready for use with the U-Net or other dataset.

## Programs built

### Wiggle

### Single


## Building

This project is built with [meson]() and [gcc](). It requires the following libraries:

* [libcee]() - automatically pulled in via meson.
* [imagine]() - build and install this separately - do this first!
* [libtiff]()
* [libpng]()
* [libglfw3]()

Under debian, you can install most of these with the following command

    apt-get install libglfw3-dev libpng-dev libcfitsio-dev libjpeg-dev fftw3-dev libtiff-dev

The following commands will build the release version:

    meson setup --buildtype=release release
    ninja -C release

## Running

Wiggle runs as follows:

    ./release/graph -t -r -q 10 -j 200 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180302-QL285-d0.0 -l /phd/wormz/queelim/dataset_aug6/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL285-d0.0 -o /phd/wormz/queelim/dataset_aug6 >> dataset.log

Single runs as follows:

    ./release/single -i /media/proto_backup/wormz/queelim/ins-6-mCherry/20170804-QL923_SB3-d1.0/QL923_SB3-d1.0xAutoStack54.tiff -w /media/proto_backup/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL923_SB3-d1.0/ID54_WS2.tiff -a /media/proto_backup/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL923_SB3-d1.0/ID54_2.log -o /media/proto_backup/wormz/queelim/temp


### Command line parameters

