#!/bin/bash
LAYERED_IN=$1
NEURONS_IN=$1
DATASET_OUT=$1

for entry in "$LAYERED_IN"/*_layered*
do
    filename="${entry##*/}"
    name=$(basename -- "$entry")
    IFS='_' read -ra tokens <<< "$name"
    asi="${tokens[0]}_asi.fits"
    asj="${tokens[0]}_asj.fits"

    echo $NEURONS_IN/$asi
    if test -f "$NEURONS_IN/$asi"; then
        
        if test -f "$NEURONS_IN/$asj"; then
            #cp $NEURONS_IN/$asi $DATASET_OUT/.
            #cp $NEURONS_IN/$asj $DATASET_OUT/.
            #cp $entry $DATASET_OUT/.
            echo "$filename, $asi, $asj" >> $DATASET_OUT/dataset.csv
        fi
        
    fi

done
