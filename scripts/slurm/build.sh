#!/bin/bash

#SBATCH --account=tesr90714
#SBATCH --time=1:00:00
#SBATCH -p cbuild
#SBATCH -N 1
#SBATCH --ntasks 1
#SBATCH --cpus-per-task=16
#SBATCH --job-name=mulisse
#SBATCH --output=log/build.log

module load 2023 &&
module load GCCcore/12.3.0 &&
module load CMake/3.26.3-GCCcore-12.3.0 &&
module load Ninja/1.11.1-GCCcore-12.3.0 &&
module load Boost/1.82.0-GCC-12.3.0 &&
module load FFTW/3.3.10-GCC-12.3.0 &&

./scripts/sh/build.sh