#!/bin/bash

# Adjust the loaded modules for the specific server
module load GCCcore/13.3.0 &&
module load CMake/3.29.3-GCCcore-13.3.0 &&
module load Ninja/1.12.1-GCCcore-13.3.0 &&
module load Boost/1.85.0-GCC-13.3.0 &&
module load FFTW/3.3.10-GCC-13.3.0 &&
module load Python/3.12.3-GCCcore-13.3.0
