#!/usr/bin/env bash

DELETE_BUILD_DIR=false
IGNORE_TESTS="ON"
WARNINGS="ON"
USE_DOUBLE="OFF"
USE_DOUBLE_FFT_PRE="OFF"
USE_SINGLE_MASS="OFF"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--clean)
            DELETE_BUILD_DIR=true
            echo "Deleting build directory..."
            ;;
        -t|--tests)
            IGNORE_TESTS="OFF"
            echo "Including tests in the build..."
            ;;
        -n|--no-warn)
            WARNINGS="OFF"
            echo "Disabling warnings..."
            ;;
        --use_double)
            USE_DOUBLE="ON"
            echo "Using double precision in the build..."
            ;;
        --use_double_fft_pre)
            USE_DOUBLE_FFT_PRE="ON"
            echo "Using double precision precomputed FFTs in the build..."
            ;;
        --use_single_mass)
            USE_SINGLE_MASS="ON"
            echo "Using single precision MASS in the build..."
            ;;
        *)
            echo "Usage: $0 [-c|--clean] [-t|--tests] [-n|--no-warn] [--use_double|--use_double_fft_pre|--use_single_mass]"
            echo "  -c, --clean             Delete the build directory before building"
            echo "  -t, --tests             Include tests in the build"
            echo "  -n, --no-warn           Disable warnings"
            echo "  --use_double            Use double precision in the build"
            echo "  --use_double_fft_pre    Use double precision precomputed FFTs in the build"
            echo "  --use_single_mass       Use single precision MASS in the build"
            exit 1
            ;;
    esac
    shift
done

if $DELETE_BUILD_DIR; then
    rm -rf build
fi

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DIGNORE_TESTS=${IGNORE_TESTS} -DWARN=${WARNINGS} -DUSE_DOUBLE=${USE_DOUBLE} \
      -DUSE_SINGLE_MASS=${USE_SINGLE_MASS} -DUSE_DOUBLE_FFT_PRE=${USE_DOUBLE_FFT_PRE} -G Ninja ..
cmake --build .
