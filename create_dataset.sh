#!/bin/sh
# Usage ./create_dataset.sh <source> <dest> <annotations>

# Flatten to get our mips
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

# Now lets have a look at the dataset
mapfile -t counts < count.log
touch dataset.log
IDX=0

for d in in `ls $3` ; do
    COUNT=${arr[$IDX]}
    echo $COUNT $1/$d
    ./build/dataset -f -r -n $COUNT -i $1/$d -o $2 >> dataset.log
    ((IDX=IDX+1))
done