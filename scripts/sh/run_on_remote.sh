#!/bin/bash

# This script bundles together other scripts to run experiments on a remote machine.
# The remote is configured in `scripts/local_settings.json`
#
# The script involves the following:
# 1. Copy the necessary files onto the remote with `scp`
# 2. Build the project on the remote
# 3. Clean up artifacts from previous run on the remote
# 4. Run the experiments defined in the configuration files passed as an argument or the default one

NO_SCP_REPO=false
INCLUDE_SLURM_FILES=false
IGNORE_TESTS=true
CLEAN_BUILD=false
reading_config_files=false
experiment_name=""
config_files=()

while [[ "$#" -gt 0 ]]; do
    if [[ $1 == -* ]]; then
        echo $1
        reading_config_files=false
    fi

    case $1 in
    -i | --input_configs)
        reading_config_files=true
        ;;
    -n | --name)
        experiment_name=$2
        shift
        ;;
    --no_scp_repo)
        NO_SCP_REPO=true
        ;;
    -t | --tests)
        IGNORE_TESTS=false
        ;;
    --clean_build)
        CLEAN_BUILD=true
        ;;
    *)
        if $reading_config_files; then
            config_files+=("$1")
        else
            echo "Usage: $0 -i config1.json [config2.json ...] -n name [--no_scp_repo] [-t | --tests] [--clean_build]"
            echo "  -i, --input_configs  Paths to the configuration files"
            echo "  -n, --name           Name of the experiment"
            echo "  --no_scp_repo        Skip copying the repository to the remote"
            echo "  -t, --tests          Build tests as well"
            echo "  --clean_build        Perform a clean build on the remote"
            exit 1
        fi
        ;;
    esac
    shift
done

if [[ -z "$experiment_name" ]]; then
    echo "Experiment name is required (-n|--name name)"
    exit 1
fi
if [[ ${#config_files[@]} -eq 0 ]]; then
    echo "At least one configuration file is required (-i|--input_configs file1 file2 ...)"
    exit 1
fi

build_flags=$([[ "$CLEAN_BUILD" == true ]] && echo "-c" || echo "")
build_flags+=$([[ "$IGNORE_TESTS" == true ]] && echo "" || echo " -t")

# Step 1
if ! $NO_SCP_REPO; then
    ./scripts/sh/scp_repo.sh
fi

# Step 2
remote_url=$(cat scripts/local_settings.json | jq '.["REMOTE_URL"]' | tr -d '"')
remote_path=$(cat scripts/local_settings.json | jq '.["REMOTE_PATH"]' | tr -d '"')

remote_cmd="cd '${remote_path}' && \
    unzip -o MULISSE.zip && \
    chmod u+x ./scripts/slurm/build.sh && \
    ./scripts/slurm/build.sh ${build_flags}"
ssh "$remote_url" "$remote_cmd"

# Step 3
remote_cmd="cd '${remote_path}' && \
    rm -rf DATA && \
    rm -rf LOGS"
ssh "$remote_url" "$remote_cmd"

# Step 4
remote_cmd="cd '${remote_path}' && \
    chmod u+x ./scripts/slurm/run_mulisse.sh && \
    sbatch ./scripts/slurm/run_mulisse.sh $experiment_name ${config_files[@]}"
ssh "$remote_url" "$remote_cmd"
