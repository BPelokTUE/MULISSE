#!/bin/bash

# Load required modules
module load GCCcore/13.3.0 &&
module load CMake/3.29.3-GCCcore-13.3.0 &&
module load Ninja/1.12.1-GCCcore-13.3.0 &&
module load Boost/1.85.0-GCC-13.3.0 &&
module load FFTW/3.3.10-GCC-13.3.0 &&
module load tbb/2021.13.0-GCCcore-13.3.0

./scripts/sh/build.sh $@

module load Python/3.12.3-GCCcore-13.3.0

pip install -r scripts/py/requirements.txt

