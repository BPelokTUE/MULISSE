#!/bin/bash

source scripts/slurm/header.sh

if [ "$#" -ne 7 ]; then
    echo "Usage: search_isax_ed_early query_path isax_ed_file dataset_path k n_channels series_len index_path"
    return 1
fi

query_path="$1"
isax_ed_file="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"
index_path="$7"

echo "MULISSE ED with early abandoning"
./mulisse search -q "${query_path}" -o "${isax_ed_file}" -d "${dataset_path}" -T knn -k "${k}" -D ed -c "${n_channels}" -m "${series_len}" -i "${index_path}" --early_abandon
