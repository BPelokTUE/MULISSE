#!/bin/bash

n_series=10000
series_len=512
n_channels=4
n_queries=100
base_dir=../DATA
inner_dir=mts
l_min=128
l_max=512
lengths=(128 256 512)
k=1
query_file=${inner_dir}/test_query.txt

dataset_file=${inner_dir}/test_big.bin

cd build
mkdir -p ${base_dir}/${inner_dir}

# Create dataset
echo 'CREATING DATASET'
./mulisse create_ds -d $dataset_file -n ${n_series} -m ${series_len} -c ${n_channels} -S 8999

# # Create queries
echo 'CREATING QUERIES'
./mulisse create_qs -d $dataset_file -q ${inner_dir}/test_query.txt -m ${series_len} -c ${n_channels} --lengths ${lengths[@]} --noise 0.1 -Q ${n_queries}

# Create index for ulisse (and creates ffts)
# echo 'CREATING MULISSE INDEX'
# ./mulisse index -d $dataset_file -i ${inner_dir}/test_ind.bin -m ${series_len} -c ${n_channels} -l ${l_min} -L ${l_max} -s 32 -p 385 -C 64  -F ${inner_dir}/test_ffts.bin -S em 

ed_file=${inner_dir}/ed.txt
mass_file=${inner_dir}/mass.txt
mass_fft_file=${inner_dir}/mass_fft.txt
isax_ed_file=${inner_dir}/isax_ed.txt
isax_mass_file=${inner_dir}/isax_mass.txt
isax_mass_fft_file=${inner_dir}/isax_mass_fft.txt

# BRUTE FORCE
# echo "BRUTE FORCE"
# ./mulisse search -q ${query_file} -o ${ed_file} -d $dataset_file -T knn -k ${k} -D ed -c ${n_channels} -m ${series_len} -t scan

# # MASS with precomputed FFTs
# echo "MASS"
# ./mulisse search -q ${query_file} -o ${mass_fft_file} -d $dataset_file -T knn -k ${k} -D mass -F ${inner_dir}/test_ffts.bin -c ${n_channels} -m ${series_len} -t scan

# ISAX ED
# echo "MULISSE ED"
# ./mulisse search -q ${query_file} -o ${isax_ed_file} -d $dataset_file -T knn -k ${k} -D ed -c ${n_channels} -m ${series_len} -i ${inner_dir}/test_ind.bin

# ISAX MASS with precomputed FFTs
# echo "MULISSE"
# ./mulisse search -q ${query_file} -o ${isax_mass_fft_file} -d $dataset_file -T knn -k ${k} -D mass -c ${n_channels} -m ${series_len} -i ${inner_dir}/test_ind.bin -F ${inner_dir}/test_ffts.bin
