#!/bin/bash

source scripts/slurm/header.sh

if [ "$#" -ne 7 ]; then
    echo "Usage: search_mass_fft query_path mass_fft_file dataset_path k n_channels series_len fft_path"
    return 1
fi

query_path="$1"
mass_fft_file="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"
fft_path="$7"

echo "MASS with precomputed FFTs"
./mulisse search -q "${query_path}" -o "${mass_fft_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -F "${fft_path}" -c "${n_channels}" -m "${series_len}" -t scan
