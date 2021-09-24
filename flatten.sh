#!/bin/sh
# Usage ./flatten.sh <source> <dest>

# Flatten to get our mips
touch flatten.log
touch count.log
COUNT=`tail -n 1 count.log`

if [ -n "$COUNT" ]; then
  echo $COUNT
else
  COUNT=0
fi

echo $COUNT

for d in `ls $1` ; do
    echo $1/$d
    ./build/flatten -m -r -b -n $COUNT -i $1/$d -o $2 >> flatten.log
    LAST=`ls $2/*mip* | tail -n 1`
    LAST=$(basename $LAST)
    COUNT=`echo $LAST | sed 's/\(.*\)_\(.*\)/\1/g'`
    COUNT=`echo $COUNT | sed 's/^0*//'`
    ((COUNT=COUNT+1))
    echo $COUNT
    echo $COUNT >> count.log
done
