#!/bin/bash

# Adjust slurm settings for the specific server
#SBATCH --time=7-00:00:00
#SBATCH -p <YOUR_PARTITION>
#SBATCH -N 1
#SBATCH --ntasks 1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --job-name=run_mulisse
#SBATCH --output=logs/%j.out

source scripts/sh/load_modules.sh

experiment_name=$1
shift

# Handle optional timeout argument
timeout_str=""
if [[ $1 == -t ]]; then
    timeout_str="-t $2"
    shift 2
fi

# Run the experiments
log_dirs=()
for config_file in "$@"; do
    log_dir="LOGS_$(basename ${config_file%.*})"
    ./scripts/py/run_mulisse.py -i $config_file $timeout_str
    mv LOGS $log_dir
    log_dirs+=("$log_dir")
    rm -rf DATA
done

# Zip the logs
zip_name="${experiment_name}_$(date +%Y-%m-%d_%H:%M).zip"
zip -r $zip_name ${log_dirs[@]}
mv $zip_name EXPERIMENT_LOGS/