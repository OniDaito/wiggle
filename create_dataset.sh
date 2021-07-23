#!/bin/sh
# Usage ./create_dataset.sh <source> <dest>
touch flatten.log
COUNT=0
echo $COUNT >> count.log

for d in `ls $1` ; do
    echo $1/$d
    ./build/flatten -m -r -n $COUNT -i $1/$d -o $2 >> flatten.log
    COUNT=`ls $2 | wc -l`
    ((COUNT=COUNT+1))
    echo $COUNT
    echo $COUNT >> count.log
done
