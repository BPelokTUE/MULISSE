#!/bin/bash

#SBATCH --job-name=run_mulisse

source scripts/slurm/header.sh

experiment_name=$1
shift

log_dirs=()
for config_file in "$@"; do
    log_dir="LOGS_$(basename ${config_file%.*})"
    ./scripts/py/run_mulisse.py -i $config_file
    mv LOGS $log_dir
    log_dirs+=("$log_dir")
    rm -rf DATA
done

zip_name="${experiment_name}_$(date +%Y-%m-%d_%H:%M).zip"
zip -r $zip_name ${log_dirs[@]}
mv $zip_name EXPERIMENT_LOGS/
