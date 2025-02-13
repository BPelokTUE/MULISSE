#!/bin/sh

# Remove the suffix _cq5 from all directories in DATA/mts
for dir in $(ls -d DATA/mts/*_cq5); do
    newdir=$(echo $dir | sed 's/_cq5//')
    mv $dir $newdir
done