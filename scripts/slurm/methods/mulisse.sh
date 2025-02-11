#!/bin/bash

source scripts/slurm/header.sh

if [ "$#" -ne 8 ]; then
    echo "Usage: search_isax_mass_fft query_path isax_mass_fft_file dataset_path k n_channels series_len index_path fft_path"
    return 1
fi

query_path="$1"
isax_mass_fft_file="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"
index_path="$7"
fft_path="$8"

echo "MULISSE MASS with precomputed FFTs"
./mulisse search -q "${query_path}" -o "${isax_mass_fft_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -c "${n_channels}" -m "${series_len}" -i "${index_path}" -F "${fft_path}"