#!/bin/bash

module load 2023 &&
module load GCCcore/12.3.0 &&
module load CMake/3.26.3-GCCcore-12.3.0 &&
module load Ninja/1.11.1-GCCcore-12.3.0 &&
module load Boost/1.82.0-GCC-12.3.0 &&
module load FFTW/3.3.10-GCC-12.3.0 &&

# module load eb/4.9.4 &&
# eblocalinstall FFTW-3.3.10-GCC-12.3.0.eb &&

./scripts/sh/build.sh