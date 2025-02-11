#!/bin/bash

#SBATCH --account=tesr90714
#SBATCH --time=1:00:00
#SBATCH -p rome
#SBATCH -N 1
#SBATCH --ntasks 1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --job-name=mulisse
#SBATCH --output=log/parse_csv.log

module load 2023 &&
module load GCCcore/12.3.0 &&
module load CMake/3.26.3-GCCcore-12.3.0 &&
module load Ninja/1.11.1-GCCcore-12.3.0 &&
module load Boost/1.82.0-GCC-12.3.0 &&
module load FFTW/3.3.10-GCC-12.3.0

if [ "$#" -ne 7 ]; then
    echo "Usage: create_index dataset_path index_path series_len n_channels base_cardinality word_length"
    return 1
fi

dataset_path="$1"
index_path="$2"
series_len="$3"
n_channels="$4"
l_min="$5"
l_max="$6"
fft_path="$7"

echo "CREATING INDEX"
./mulisse create_index -d "${dataset_path}" -i "${index_path}" -m "${series_len}" \
    -c "${n_channels}" -l "${l_min}" -L "${l_max}" -s 32 -p "$((series_len - l_min + 1))" -C 64 -F "${fft_path}" -S em