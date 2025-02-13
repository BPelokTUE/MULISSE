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

if [ "$#" -lt 5 ]; then
    echo "Usage: parse_csv dataset_path l_min series_len n_series csv_files..."
    exit 1
fi

dataset_path="$1"
l_min="$2"
series_len="$3"
n_series="$4"
csv_files=("${@:5}")

echo "PARSING CSV FILES"
./mulisse parse_csv -d "${dataset_path}" -i "${csv_files[@]}" -l "${l_min}" -m "${series_len}" -n "${n_series}"
