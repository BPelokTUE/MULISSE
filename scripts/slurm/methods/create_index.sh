#!/bin/bash

source scripts/slurm/header.sh

if [ "$#" -ne 7 ]; then
    echo "Usage: create_index dataset_path index_path series_len n_channels base_cardinality word_length"
    return 1
fi

dataset_path="$1"
index_path="$2"
series_len="$3"
n_channels="$4"
l_min="$5"
l_max="$6"
fft_path="$7"

echo "CREATING INDEX"
./mulisse create_index -d "${dataset_path}" -i "${index_path}" -m "${series_len}" \
    -c "${n_channels}" -l "${l_min}" -L "${l_max}" -s 32 -p "$((series_len - l_min + 1))" -C 64 -F "${fft_path}" -S em