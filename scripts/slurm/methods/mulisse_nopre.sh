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
    echo "Usage: search_isax_mass_fft query_path isax_mass_fft_file dataset_path k n_channels series_len lmin lmax"
    return 1
fi

query_path="$1"
isax_mass_fft_file="$2"
dataset_path="$3"
k="$4"
n_channels="$5"
series_len="$6"
lmin="$7"
lmax="$8"

# Put in same directory as dataset
index_path=$(dirname "${dataset_path}")/mulisseindex_lmin${lmin}_lmax${lmax}.bin

# Create index
bash ../scripts/slurm/methods/create_index.sh "${dataset_path}" "${index_path}" "${series_len}" "${n_channels}" "${lmin}" "${lmax}" &&

echo "MULISSE MASS no precomputed FFTs"
./mulisse search -q "${query_path}" -o "${isax_mass_fft_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -c "${n_channels}" -m "${series_len}" -i "${index_path}"

# Delete index
rm -f "${index_path}"