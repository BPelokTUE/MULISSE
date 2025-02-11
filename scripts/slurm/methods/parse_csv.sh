#!/bin/bash

source scripts/slurm/header.sh

if [ "$#" -lt 5 ]; then
    echo "Usage: parse_csv dataset_path l_min series_len n_series csv_files..."
    exit 1
fi

dataset_path="$1"
l_min="$2"
series_len="$3"
n_series="$4"
csv_files=("${@:5}")

echo "DATASET PATH: ${dataset_path}"

echo "PARSING CSV FILES"
./mulisse parse_csv -d "${dataset_path}" -i "${csv_files[@]}" -l "${l_min}" -m "${series_len}" -n "${n_series}"
