#!/bin/bash

n_series=5000
series_len=2048
n_channels=4
n_queries=100
base_dir=../DATA
inner_dir=mts
l_min=1796
l_max=2048
lengths=(1796 1900 2048)
k=1

filename=rwalk_N${n_series}_C${n_channels}_M${series_len}
dataset_file=${inner_dir}/$filename.bin
query_file=${inner_dir}/query_$filename.txt
index_file=${inner_dir}/index_$filename.bin
ffts_file=${inner_dir}/ffts_$filename.bin

cd build
mkdir -p ${base_dir}/${inner_dir}

# Create dataset
echo 'CREATING DATASET'
./mulisse create_ds -d $dataset_file -n ${n_series} -m ${series_len} -c ${n_channels} -S 8999

# # # Create queries
echo 'CREATING QUERIES'
./mulisse create_qs -d $dataset_file -q $query_file -m ${series_len} -c ${n_channels} --lengths ${lengths[@]} --noise 0.1 -Q ${n_queries}

# # Create index for ulisse (and creates ffts)
echo 'CREATING MULISSE INDEX'
./mulisse index -d $dataset_file -i $index_file -m ${series_len} -c ${n_channels} -l ${l_min} -L ${l_max} -s 32 -p $(( l_max - l_min + 1)) -C 64  -F $ffts_file -S em 

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
echo "MASS"
./mulisse search -q ${query_file} -o ${mass_fft_file} -d $dataset_file -T knn -k ${k} -D mass -F $ffts_file -c ${n_channels} -m ${series_len} -t scan

# ISAX ED
echo "MULISSE ED"
./mulisse search -q ${query_file} -o ${isax_ed_file} -d $dataset_file -T knn -k ${k} -D ed -c ${n_channels} -m ${series_len} -i $index_file

# ISAX MASS with precomputed FFTs
echo "MULISSE"
./mulisse search -q ${query_file} -o ${isax_mass_fft_file} -d $dataset_file -T knn -k ${k} -D mass -c ${n_channels} -m ${series_len} -i $index_file -F $ffts_file
