#!/bin/bash

n_series=1000
n_channels=5
series_len=10000
n_queries=20
base_dir=../DATA
inner_dir=mts
l_min=730
l_max=730
lengths=(730)
k=1

cd build
mkdir -p ${base_dir}/${inner_dir}


# Create random walk dataset
# echo "CREATING DATASET"
# filename=rwalk_n${n_series}_m${series_len}_c${n_channels}
# dataset_path=${inner_dir}/${filename}.bin
# ./mulisse create_ds -d ${dataset_path} -n ${n_series} -m ${series_len} -c ${n_channels} -S 8999

# OR
# Create dataset from CSV files
csv_dir=/home/jens/tue/data/MTS/subsequence_search/preprocessed/stocks
csv_files=($(ls ${csv_dir}/*))
n_channels=${#csv_files[@]}
filename=$(basename ${csv_dir})_n${n_series}_m${series_len}_c${n_channels}

# Get basename from csv_dir
dataset_path=${inner_dir}/${filename}.bin
index_path=${inner_dir}/index_${filename}_lmin${l_min}_lmax${l_max}.bin
fft_path=${inner_dir}/ffts_${filename}.bin
query_path=${inner_dir}/query_${filename}.txt

# Parse CSV files
echo "PARSING CSV FILES"
./mulisse parse_csv -d ${dataset_path} -i ${csv_files[@]} -l ${l_min} -m ${series_len} -n ${n_series}

# Create queries
echo "CREATING QUERIES"
./mulisse create_qs -d ${dataset_path} -q ${query_path} -m ${series_len} -c ${n_channels} --lengths ${lengths[@]} --noise 0.1 -Q ${n_queries} -u ${n_channels}

# Create index for ulisse (and creates ffts)
echo "CREATING INDEX"
./mulisse index -d ${dataset_path} -i ${index_path} -m ${series_len} -c ${n_channels} -l ${l_min} -L ${l_max} -s 32 -p $((l_max - l_min + 10)) -C 64  -F ${fft_path} -S em 

ed_file=${inner_dir}/ed.txt
mass_file=${inner_dir}/mass.txt
mass_fft_file=${inner_dir}/mass_fft.txt
isax_ed_file=${inner_dir}/isax_ed.txt
isax_mass_file=${inner_dir}/isax_mass.txt
isax_mass_fft_file=${inner_dir}/isax_mass_fft.txt

# BRUTE FORCE
# echo "BRUTE FORCE"
# ./mulisse search -q ${query_path} -o ${ed_file} -d ${dataset_path} -T knn -k ${k} -D ed -c ${n_channels} -m ${series_len} -t scan

# ED with early abandoning
# echo "ED with early abandoning"
# ./mulisse search -q ${query_path} -o ${ed_file} -d ${dataset_path} -T knn -k ${k} -D ed -c ${n_channels} -m ${series_len} -t scan --early_abandon

# MASS
# echo "MASS"
# ./mulisse search -q ${query_path} -o ${mass_file} -d ${dataset_path} -T knn -k ${k} -D mass -c ${n_channels} -m ${series_len} -t scan

# MASS with precomputed FFTs
# echo "MASS with precomputed FFTs"
# ./mulisse search -q ${query_path} -o ${mass_fft_file} -d ${dataset_path} -T knn -k ${k} -D mass -F ${fft_path} -c ${n_channels} -m ${series_len} -t scan

# ISAX ED
# echo "MULISSE ED"
# ./mulisse search -q ${query_path} -o ${isax_ed_file} -d ${dataset_path} -T knn -k ${k} -D ed -c ${n_channels} -m ${series_len} -i ${index_path}

# ISAX ED with early abandoning
# echo "MULISSE ED with early abandoning"
# ./mulisse search -q ${query_path} -o ${isax_ed_file} -d ${dataset_path} -T knn -k ${k} -D ed -c ${n_channels} -m ${series_len} -i ${index_path} --early_abandon

# ISAX MASS
# echo "MULISSE MASS"
# ./mulisse search -q ${query_path} -o ${isax_mass_file} -d ${dataset_path} -T knn -k ${k} -D mass -c ${n_channels} -m ${series_len} -i ${index_path}

# ISAX MASS with precomputed FFTs
echo "MULISSE MASS with precomputed FFTs"
./mulisse search -q ${query_path} -o ${isax_mass_fft_file} -d ${dataset_path} -T knn -k ${k} -D mass -c ${n_channels} -m ${series_len} -i ${index_path} -F ${fft_path}
