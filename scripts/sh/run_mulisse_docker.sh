#!/bin/bash

CONFIGS_DIR="scripts/run_configs"

configs=()
for arg in "$@"; do
    if ! [ -f "$CONFIGS_DIR/$arg" ]; then
        if [[ $arg == $CONFIGS_DIR/* ]]; then
            new_arg=${arg#$CONFIGS_DIR/}
            if [ -f "$CONFIGS_DIR/$new_arg" ]; then
                configs+=("$new_arg")
                continue
            fi
        fi
        echo "Config file $arg not found. Config files should be given relative to $CONFIGS_DIR"
        exit 1
    fi
    configs+=("$arg")
done

docker build -t mulisse .
for config in "${configs[@]}"; do
    logs_dir="EXPERIMENT_LOGS/$(dirname $config)/LOGS_$(basename $config .json)"
    docker run --rm -v $(pwd)/${logs_dir}:/mulisse/LOGS -v $(pwd)/mulisse_pack:/mulisse/mulisse_pack mulisse -i /mulisse/scripts/run_configs/$config
done
