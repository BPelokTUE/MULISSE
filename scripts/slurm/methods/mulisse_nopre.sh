#!/bin/bash

source scripts/slurm/header.sh

if [ "$#" -ne 6 ]; then
    echo "Usage: search_isax_mass_fft query_path isax_mass_fft_file dataset_path k n_channels series_len lmin lmax"
    return 1
fi

query_path="$1"
isax_mass_fft_file="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"
lmin="$7"
lmax="$8"

index_path="${dataset_path}_lmin${lmin}_lmax${lmax}.mulisseindex"

# Create index
bash ../scripts/slurm/methods/create_index.sh "${dataset_path}" "${index_path}" "${series_len}" "${n_channels}" "${lmin}" "${lmax}" &&

echo "MULISSE MASS no precomputed FFTs"
./mulisse search -q "${query_path}" -o "${isax_mass_fft_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -c "${n_channels}" -m "${series_len}" -i "${index_path}"