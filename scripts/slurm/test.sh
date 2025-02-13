#!/bin/bash

n_series=1000
n_channels=5
series_len=512
n_queries=20
base_dir=DATA
inner_dir=mts
l_min=256
l_max=512
lengths=(512)
k=1

mkdir -p ${base_dir}/${inner_dir}

csv_dir=~/mts-subsequence-search/data/stocks
csv_files=($(ls ${csv_dir}/*))
n_channels=${#csv_files[@]}
filename=$(basename ${csv_dir})_n${n_series}_m${series_len}_c${n_channels}

# Get basename from csv_dir
dataset_path=${inner_dir}/${filename}.bin
index_path=${inner_dir}/index_${filename}_lmin${l_min}_lmax${l_max}.bin
fft_path=${inner_dir}/ffts_${filename}.bin
query_path=${inner_dir}/query_${filename}.txt

ed_file=${inner_dir}/ed.txt
mass_file=${inner_dir}/mass.txt
mass_fft_file=${inner_dir}/mass_fft.txt
isax_ed_file=${inner_dir}/isax_ed.txt
isax_mass_file=${inner_dir}/isax_mass.txt
isax_mass_fft_file=${inner_dir}/isax_mass_fft.txt

# Create dataset
# bash scripts/slurm/methods/parse_csv.sh "${dataset_path}" "${l_min}" "${series_len}" "${n_series}" "${csv_files[@]}"

# Create queries
# bash scripts/slurm/methods/create_queries.sh "${dataset_path}" "${query_path}" "${series_len}" "${n_channels}" "${n_queries}" "${lengths[@]}"

# Run MASS
# bash scripts/slurm/methods/mass.sh "${query_path}" "${mass_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}"

# Run MULISSE
bash scripts/slurm/methods/mulisse.sh "${query_path}" "${isax_mass_file}" "${dataset_path}" "${k}" "${n_channels}" "${series_len}" "${l_min}" "${l_max}"
