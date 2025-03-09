#!/bin/bash

date_format="+%Y-%m-%d_%H:%M"

if [ "$#" -ne 1 ]; then
    echo "Usage: ./scripts/sh/archive_logs.sh <logs_parent_dir>"
    echo "  <logs_parent_dir>  The parent of the "LOGS_" directories to archive"
    exit 1
fi

logs_parent_dir=$1
if [ ! -d "$logs_parent_dir" ]; then
    echo "Directory $logs_parent_dir does not exist."
    exit 1
fi

archive_path=EXPERIMENT_LOGS/ARCHIVE/$(basename ${logs_parent_dir})_$(date $date_format).zip
zip -r $archive_path $logs_parent_dir/*
