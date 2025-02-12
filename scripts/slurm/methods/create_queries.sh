#!/bin/bash

#SBATCH --account=tesr90714
#SBATCH --time=1:00:00
#SBATCH -p rome
#SBATCH -N 1
#SBATCH --ntasks 1
#SBATCH --cpus-per-task=16
#SBATCH --mem=16G
#SBATCH --job-name=mulisse
#SBATCH --output=logs/%j.out

# Load required modules
module load 2023 &&
module load GCCcore/12.3.0 &&
module load CMake/3.26.3-GCCcore-12.3.0 &&
module load Ninja/1.11.1-GCCcore-12.3.0 &&
module load Boost/1.82.0-GCC-12.3.0 &&
module load FFTW/3.3.10-GCC-12.3.0 &&

cd build

if [ "$#" -lt 7 ]; then
    echo "Usage: create_queries dataset_path query_path series_len num_channels num_query_channels n_queries lengths..."
    return 1
fi

dataset_path="$1"
query_path="$2"
series_len="$3"
num_channels="$4"
num_query_channels="$5"
n_queries="$6"
lengths=("${@:7}")  # Get remaining arguments as lengths array

echo "CREATING QUERIES"
./mulisse create_qs -d "${dataset_path}" -q "${query_path}" -m "${series_len}" \
    -c "${num_channels}" --lengths "${lengths[@]}" --noise 0.1 -Q "${n_queries}" -u "${num_query_channels}"