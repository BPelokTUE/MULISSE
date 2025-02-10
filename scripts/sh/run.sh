#!/bin/bash

n_series=500
series_len=512
n_channels=5
n_queries=5
base_dir=../DATA
inner_dir=weather
l_min=128
l_max=512
lengths=(128 256 512)
k=5
query_file=${inner_dir}/test_query.txt

cd build
mkdir -p ${base_dir}/${inner_dir}

# Create random walk dataset
# ./mulisse create_ds -d ${inner_dir}/test.bin -n ${n_series} -m ${series_len} -c ${n_channels} -S 8999

# OR
# Create dataset from CSV files
csv_dir=../../DATA/mulisse_pack/weather
csv_files=(${csv_dir}/DEW ${csv_dir}/SLP ${csv_dir}/TMP ${csv_dir}/WND)
num_channels=${#csv_files[@]}
./mulisse parse_csv -d ${inner_dir}/test.bin -i ${csv_files[@]} -l ${l_min} -m ${series_len}

# Create queries
./mulisse create_qs -d ${inner_dir}/test.bin -q ${inner_dir}/test_query.txt -m ${series_len} -c ${n_channels} --lengths ${lengths[@]} --noise 0.1 -Q ${n_queries}

# Create index for ulisse (and creates ffts)
./mulisse index -d ${inner_dir}/test.bin -i ${inner_dir}/test_ind.bin -m ${series_len} -c ${n_channels} -l ${l_min} -L ${l_max} -s 32 -p 385 -C 64  -F ${inner_dir}/test_ffts.bin -S em 

ed_file=${inner_dir}/ed.txt
mass_file=${inner_dir}/mass.txt
mass_fft_file=${inner_dir}/mass_fft.txt
isax_ed_file=${inner_dir}/isax_ed.txt
isax_mass_file=${inner_dir}/isax_mass.txt
isax_mass_fft_file=${inner_dir}/isax_mass_fft.txt

# BRUTE FORCE
./mulisse search -q ${query_file} -o ${ed_file} -d ${inner_dir}/test.bin -T knn -k ${k} -D ed -c ${n_channels} -m ${series_len} -t scan

# MASS
./mulisse search -q ${query_file} -o ${mass_file} -d ${inner_dir}/test.bin -T knn -k ${k} -D mass -c ${n_channels} -m ${series_len} -t scan

# MASS with precomputed FFTs
./mulisse search -q ${query_file} -o ${mass_fft_file} -d ${inner_dir}/test.bin -T knn -k ${k} -D mass -F ${inner_dir}/test_ffts.bin -c ${n_channels} -m ${series_len} -t scan

# ISAX ED
./mulisse search -q ${query_file} -o ${isax_ed_file} -d ${inner_dir}/test.bin -T knn -k ${k} -D ed -c ${n_channels} -m ${series_len} -i ${inner_dir}/test_ind.bin

# ISAX MASS
./mulisse search -q ${query_file} -o ${isax_mass_file} -d ${inner_dir}/test.bin -T knn -k ${k} -D mass -c ${n_channels} -m ${series_len} -i ${inner_dir}/test_ind.bin

# ISAX MASS with precomputed FFTs
./mulisse search -q ${query_file} -o ${isax_mass_fft_file} -d ${inner_dir}/test.bin -T knn -k ${k} -D mass -c ${n_channels} -m ${series_len} -i ${inner_dir}/test_ind.bin -F ${inner_dir}/test_ffts.bin
