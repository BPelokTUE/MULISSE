#!/bin/bash

source scripts/slurm/header.sh

if [ "$#" -ne 6 ]; then
    echo "Usage: search_mass query_path mass_file dataset_path k n_channels series_len"
    return 1
fi

query_path="$1"
mass_file="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"

echo "MASS"
./mulisse search -q "${query_path}" -o "${mass_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -c "${n_channels}" -m "${series_len}" -t scan
