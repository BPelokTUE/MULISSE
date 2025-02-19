#!/bin/bash

#SBATCH --time=24:00:00
#SBATCH -p cbuild
#SBATCH -N 1
#SBATCH --ntasks 1
#SBATCH --cpus-per-task=16
#SBATCH --job-name=mulisse
#SBATCH --output=log/run_mulisse.log

source scripts/slurm/header.sh

if [[ " $@ " == *" -b "* ]]; then
    pip install tqdm
fi

./scripts/py/run_mulisse.py $@
