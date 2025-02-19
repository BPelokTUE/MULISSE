#!/bin/bash

#SBATCH --time=1:00:00
#SBATCH -p cbuild
#SBATCH -N 1
#SBATCH --ntasks 1
#SBATCH --cpus-per-task=16
#SBATCH --job-name=mulisse
#SBATCH --output=log/build.log

source scripts/slurm/header.sh

./scripts/sh/build.sh $@
