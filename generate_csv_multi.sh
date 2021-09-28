#!/bin/bash
LAYERED_IN=$1
NEURONS_IN=$1
DATASET_OUT=$1

for entry in "$LAYERED_IN"/*_layered*
do
    filename="${entry##*/}"
    name=$(basename -- "$entry")
    IFS='_' read -ra tokens <<< "$name"
    mask="${tokens[0]}_mask.fits"

    echo $NEURONS_IN/$mask
    if test -f "$NEURONS_IN/$mask"; then
        echo "$filename, $mask" >> $DATASET_OUT/dataset.csv
    fi

done
