#!/bin/bash

# Bundle together the files necessary for running experiments and copy them to the remote machine.
# By default the slurm script header (`scripts/slurm/header.sh`) is not included in the zip file,
# as the specific slurm settings are server dependent. If you want to include the slurm script header,
# use the `-s` flag.
#
# Usage: ./scripts/sh/scp_repo.sh [-s]

# Default value for including scripts/slurm

local_path=$(cat local_settings.json | jq '.["REPO_PATH"]' | tr -d '"')
remote_url=$(cat local_settings.json | jq '.["REMOTE_URL"]' | tr -d '"')
remote_path=$(cat local_settings.json | jq '.["REMOTE_PATH"]' | tr -d '"')

include=(lib src extern tests CMakeLists.txt scripts/py scripts/sh BREAKPOINTS SCORE_FUNC_PARAMS)
run_configs=$(ls scripts/run_configs | grep -v "local")
for file in $run_configs; do
    include+=(scripts/run_configs/$file)
done

zip -r $local_path/MULISSE.zip ${include[@]}

# Copy zip to HPC
scp $local_path/MULISSE.zip $remote_url:$remote_path

# Remove zip
rm $local_path/MULISSE.zip
