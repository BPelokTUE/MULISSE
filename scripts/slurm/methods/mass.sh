#!/bin/bash

source scripts/slurm/header.sh

if [ "$#" -ne 7 ]; then
    echo "Usage: search_mass_fft query_path mass_fft_file dataset_path k n_channels series_len"
    return 1
fi

query_path="$1"
out_path="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"

index_path="${dataset_path}.massindex"
fft_path="${dataset_path}.ffts"

# Create index first
bash ../scripts/slurm/methods/create_index.sh "${dataset_path}" "${index_path}" "${series_len}" "${n_channels}" "${series_len}" "${series_len}" "${fft_path}" &&

# Remove the dummy index
rm -f "${index_path}" &&

echo "MASS with precomputed FFTs"
./mulisse search -q "${query_path}" -o "${out_path}" -d "${dataset_path}" -T knn -k "${k}" -D mass -F "${fft_path}" -c "${n_channels}" -m "${series_len}" -t scan
