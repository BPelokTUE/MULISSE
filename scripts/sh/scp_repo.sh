#!/bin/bash

# Bundle together the files necessary for running experiments and copy them to the remote machine.
# By default the slurm script header (`scripts/slurm/header.sh`) is not included in the zip file,
# as the specific slurm settings are server dependent. If you want to include the slurm script header,
# use the `-s` flag.
#
# Usage: ./scripts/sh/scp_repo.sh [-s]

# Default value for including scripts/slurm
INCLUDE_SLURM_HEADER=false

while [[ "$#" -gt 0 ]]; do
    case $1 in
    -s | --slurm_header)
        INCLUDE_SLURM_HEADER=true
        ;;
    *)
        echo "Usage: $0 [-s]"
        echo "  -s  Copy the slurm header to the remote"
        exit 1
        ;;
    esac
    shift
done

local_path=$(cat scripts/local_settings.json | jq '.["REPO_PATH"]' | tr -d '"')
remote_url=$(cat scripts/local_settings.json | jq '.["REMOTE_URL"]' | tr -d '"')
remote_path=$(cat scripts/local_settings.json | jq '.["REMOTE_PATH"]' | tr -d '"')

include=(lib src extern tests CMakeLists.txt scripts/py scripts/sh)
run_configs=$(ls run_configs | grep -v "local")
for file in $run_configs; do
    include+=(scripts/run_configs/$file)
done

if [ "$INCLUDE_SLURM_HEADER" = true ]; then
    include+=(scripts/slurm)
else
    slurm_files=$(ls scripts/slurm | grep -v header.sh)
    for file in $slurm_files; do
        include+=(scripts/slurm/$file)
    done
fi
echo ${include[@]}

zip -r $local_path/MULISSE.zip ${include[@]}

# Copy zip to HPC
scp $local_path/MULISSE.zip $remote_url:$remote_path

# Remove zip
rm $local_path/MULISSE.zip
