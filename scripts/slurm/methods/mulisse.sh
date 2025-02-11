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

if [ "$#" -ne 8 ]; then
    echo "Usage: search_isax_mass_fft query_path isax_mass_fft_file dataset_path k n_channels series_len index_path fft_path"
    return 1
fi

query_path="$1"
isax_mass_fft_file="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"
index_path="$7"
fft_path="$8"

echo "MULISSE MASS with precomputed FFTs"
./mulisse search -q "${query_path}" -o "${isax_mass_fft_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -c "${n_channels}" -m "${series_len}" -i "${index_path}" -F "${fft_path}"