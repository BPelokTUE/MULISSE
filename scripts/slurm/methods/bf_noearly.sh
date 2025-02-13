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

if [ "$#" -ne 6 ]; then
    echo "Usage: search_brute_force query_path ed_file dataset_path k n_channels series_len"
    return 1
fi

query_path="$1"
ed_file="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"

echo "BRUTE FORCE"
./mulisse search -q "${query_path}" -o "${ed_file}" -d "${dataset_path}" -T knn -k "${k}" -D ed -c "${n_channels}" -m "${series_len}" -t scan
