#!/bin/bash

# This script bundles together other scripts to run experiments on a remote machine.
# The remote is configured in `scripts/local_settings.json`
#
# The script involves the following:
# 1. Copy the necessary files onto the remote with `scp`
# 2. SSH onto the remote
# 3. Build the project on the remote
# 4. Clean up artifacts from previous run on the remote, unless specified otherwise
# 5. Run the experiments defined in the configuration files passed as an argument or the default one
# 6. Copy the results back to the local machine
# 7. Run analysis of the results on the local machine
#
# Usage:
# scripts/sh/run_on_remote.sh [config_file:path_to_config_file] [cleanup:0/1]

NO_SCP_REPO=false
INCLUDE_SLURM_HEADER=false
IGNORE_TESTS=true
CLEAN_BUILD=false
CLEAN_PREV_RUN=true
CLEANUP=true
reading_config_files=false
config_files=()

while [[ "$#" -gt 0 ]]; do
    if [[ $1 == -* ]]; then
        echo $1
        reading_config_files=false
    fi

    case $1 in
    -i|--input_configs)
        reading_config_files=true
        ;;
    --no_scp_repo)
        NO_SCP_REPO=true
        ;;
    -s | --slurm)
        INCLUDE_SLURM_HEADER=true
        ;;
    -t | --tests)
        IGNORE_TESTS=false
        ;;
    -d | --dirty | --no_cleanup)
        CLEANUP=false
        ;;
    --clean_build)
        CLEAN_BUILD=true
        ;;
    --no_clean_prev_run)
        CLEAN_PREV_RUN=false
        ;;
    *)
        if $reading_config_files; then
            config_files+=("$1")
        else
            echo "Usage: $0 [--no_scp_repo] [-s | --slurm] [-t | --tests] [--clean_build] [--no_clean_prev_run]"
            echo "  -i, --input_configs      Paths to the configuration files"
            echo "  --no_scp_repo        Skip copying the repository to the remote"
            echo "  -s, --slurm          Include SLURM header in the scripts"
            echo "  -t, --tests          Build tests as well"
            echo "  -d, --dirty, --no_cleanup"
            echo "                       Do not clean data from current run"
            echo "  --clean_build        Perform a clean build on the remote"
            echo "  --no_clean_prev_run  Do not clean up artifacts from the previous run"
            exit 1
        fi
        ;;
    esac
    shift
done

scp_flags=$([[ "$INCLUDE_SLURM_HEADER" == true ]] && echo "-s" || echo "")
build_flags=$([[ "$CLEAN_BUILD" == true ]] && echo "-c" || echo "")
build_flags+=$([[ "$IGNORE_TESTS" == true ]] && echo "" || echo " -t")
if [[ ${#config_files[@]} -eq 0 ]]; then
    config_files=("")
fi

# Step 1
if ! $NO_SCP_REPO; then
    ./scripts/sh/scp_repo.sh $scp_flags
fi

# Step 2
remote_url=$(cat scripts/local_settings.json | jq '.["REMOTE_URL"]' | tr -d '"')
remote_path=$(cat scripts/local_settings.json | jq '.["REMOTE_PATH"]' | tr -d '"')

# Step 3
remote_cmd="cd '${remote_path}' && \
    unzip -o MULISSE.zip && \
    chmod u+x ./scripts/slurm/build.sh && \
    ./scripts/slurm/build.sh ${build_flags}"
ssh "$remote_url" "$remote_cmd"

config_id=0
logs_dirs=()
for config_file in "${config_files[@]}"; do
    # Step 4
    if $CLEAN_PREV_RUN; then
        remote_cmd="cd '${remote_path}' && \
            rm -rf DATA && \
            rm -rf LOGS"
        echo "Running: ${remote_cmd}"
        ssh "$remote_url" "$remote_cmd"
    fi

    # Step 5
    config_flag=$([[ "$config_file" == "" ]] && echo "" || echo "-i $config_file")
    dirty_flag=$([[ "$CLEANUP" == true ]] && echo "" || echo "-d")
    remote_cmd="cd '${remote_path}' && \
        chmod u+x ./scripts/slurm/run_mulisse.sh && \
        ./scripts/slurm/run_mulisse.sh -b $config_flag $dirty_flag"
    ssh "$remote_url" "$remote_cmd" 

    # Step 6
    config_name=$([[ "$config_file" == "" ]] && echo "default" || echo "$(basename $config_file)")
    logs_dir="LOGS_${config_id}_${config_name}"
    scp -r "$remote_url:$remote_path/LOGS" "$logs_dir"
    config_id=$((config_id + 1))
    logs_dirs+=("$logs_dir")
done

chmod u+x ./scripts/py/check_results.py
for logs_dir in "${logs_dirs[@]}"; do
    # Step 7
    echo "Checking results in ${logs_dir}:"
    ./scripts/py/check_results.py -l $logs_dir
    echo ""
done
