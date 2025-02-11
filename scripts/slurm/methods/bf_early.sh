#!/bin/bash

source scripts/slurm/header.sh

if [ "$#" -ne 6 ]; then
    echo "Usage: search_ed_early query_path ed_file dataset_path k n_channels series_len"
    return 1
fi

query_path="$1"
ed_file="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"

echo "ED with early abandoning"
./mulisse search -q "${query_path}" -o "${ed_file}" -d "${dataset_path}" -T knn -k "${k}" -D ed -c "${n_channels}" -m "${series_len}" -t scan --early_abandon
