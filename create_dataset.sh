#!/bin/sh
# Usage ./create_dataset.sh <source> <dest>
touch flatten.log

for d in `ls $1` ; do
    COUNT=`ls $d | wc -l`
    echo $COUNT
    echo $COUNT >> count.log
    ./build/flatten -m -r -n $COUNT -i $d -o $2 >> flatten.log
done
