#!/bin/bash
MIPS_IN=$1
NEURONS_IN=$2
DATASET_OUT=$3

for entry in "$MIPS_IN"/*
do
    filename="${entry##*/}"
    name=$(basename -- "$entry")
    IFS='_' read -ra tokens <<< "$name"
    asi="${tokens[0]}_asi.png"
    asj="${tokens[0]}_asj.png"

    echo $NEURONS_IN/$asi
    if test -f "$NEURONS_IN/$asi"; then
        
        if test -f "$NEURONS_IN/$asj"; then
            cp $NEURONS_IN/$asi $DATASET_OUT/.
            cp $NEURONS_IN/$asj $DATASET_OUT/.
            cp $entry $DATASET_OUT/.
            echo "$filename, $asi, $asj" >> $DATASET_OUT/dataset.csv
        fi
        
    fi

done