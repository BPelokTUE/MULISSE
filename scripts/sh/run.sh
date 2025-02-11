#!/bin/bash

# source "$(dirname "$0")/../scripts/sh/methods.sh.sh"

n_series=1000
n_channels=5
series_len=512
n_queries=20
base_dir=../DATA
l_min=256
l_max=512
lengths=(512)
k=1

cd build
mkdir -p ${base_dir}

# Create dataset from CSV files
csv_dir=/home/jens/tue/data/MTS/subsequence_search/preprocessed/stocks
csv_files=($(ls ${csv_dir}/*))
n_channels=${#csv_files[@]}
filename=$(basename ${csv_dir})_n${n_series}_m${series_len}_c${n_channels}

# Get basename from csv_dir
dataset_path=${base_dir}/${filename}.bin
index_path=${base_dir}/index_${filename}_lmin${l_min}_lmax${l_max}.bin
fft_path=${base_dir}/ffts_${filename}.bin
query_path=${base_dir}/query_${filename}.txt

# Parse CSV files
../scripts/sh/methods.sh parse_csv "${dataset_path}" "${l_min}" "${series_len}" "${n_series}" "${csv_files[@]}"

# Create queries
../scripts/sh/methods.sh create_queries "${dataset_path}" "${query_path}" "${series_len}" "${n_channels}" "${n_queries}" "${lengths[@]}"

# # Create index
# ../scripts/sh/methods.sh create_index "${dataset_path}" "${index_path}" "${series_len}" "${n_channels}" "${l_min}" "${l_max}" "${fft_path}"

# ed_file=${base_dir}/ed.txt
# mass_file=${base_dir}/mass.txt
# mass_fft_file=${base_dir}/mass_fft.txt
# isax_ed_file=${base_dir}/isax_ed.txt
# isax_mass_file=${base_dir}/isax_mass.txt
# isax_mass_fft_file=${base_dir}/isax_mass_fft.txt

# # BRUTE FORCE
../scripts/sh/methods.sh search_brute_force "${query_path}" "${ed_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}"

# # ED with early abandoning
# # search_ed_early "${query_path}" "${ed_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}"

# # MASS
# # search_mass "${query_path}" "${mass_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}"

# # MASS with precomputed FFTs
# ../scripts/sh/methods.sh search_mass_fft "${query_path}" "${mass_fft_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}" "${fft_path}"

# # ISAX ED
# # search_isax_ed "${query_path}" "${isax_ed_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}" "${index_path}"

# # ISAX ED with early abandoning
# # search_isax_ed_early "${query_path}" "${isax_ed_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}" "${index_path}"

# # ISAX MASS
# # search_isax_mass "${query_path}" "${isax_mass_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}" "${index_path}"

# # ISAX MASS with precomputed FFTs
# ../scripts/sh/methods.sh search_isax_mass_fft "${query_path}" "${isax_mass_fft_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}" "${index_path}" "${fft_path}"
