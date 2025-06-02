#!/bin/bash

#SBATCH --time=1-00:00:00
#SBATCH -p mcs.default.q
#SBATCH -N 1
#SBATCH --ntasks 1
#SBATCH --cpus-per-task=16
#SBATCH --mem=128G
#SBATCH --job-name=run_mulisse
#SBATCH --output=logs/%j.out

module load GCCcore/13.3.0 &&
module load CMake/3.29.3-GCCcore-13.3.0 &&
module load Ninja/1.12.1-GCCcore-13.3.0 &&
module load Boost/1.85.0-GCC-13.3.0 &&
module load FFTW/3.3.10-GCC-13.3.0 &&
module load tbb/2021.13.0-GCCcore-13.3.0 &&
module load Python/3.12.3-GCCcore-13.3.0

experiment_name=$1
shift

timeout_str=""
dirty_str=""
config_files=()
while [[ "$#" -gt 0 ]]; do
    case $1 in
    -t | --timeout)
	timout_str="-t $2"
	shift
	;;
    -d | --dirty)
	dirty_str="-d"
	;;
    *)
	config_files+=("$1")
	;;
    esac

    shift
done

## Work on network director:w
# mkdir $TMPDIR/MULISSE
# cp -r ./scripts ./build ./BREAKPOINTS ./local_settings.json $TMPDIR/MULISSE/
# cd $TMPDIR/MULISSE

logs_dirs=()
for config_file in $config_files; do
    suffix="${config_file%.*}"
    logs_dir="LOGS_$(basename ${suffix})"
    data_dir="DATA_$(basename ${suffix})"
    ./scripts/py/run_mulisse.py -l $logs_dir -D $data_dir -i $config_file $timeout_str $dirty_str
    logs_dirs+=("$logs_dir")
    if [[ -z "$dirty_str" ]]; then
        rm -rf $data_dir
    fi
done

zip_name="${experiment_name}_$(date +%Y-%m-%d_%H:%M).zip"
zip -r $zip_name ${logs_dirs[@]}
mv $zip_name $HOME/MULISSE/EXPERIMENT_LOGS/

