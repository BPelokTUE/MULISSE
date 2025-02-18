base_dir=DATA
inner_dir=mts

csv_dir=$1
csv_files=($(ls ${csv_dir}/*))
num_channels=${#csv_files[@]}

num_query_channels=$2
series_len=$3
num_queries=$4
l_min=$5
l_max=$6
num_series=${7}
k=1

dataset_name=$(basename ${csv_dir})

# 1. Create datasets and queries
mkdir -p ${base_dir}/${inner_dir}

filedir=${inner_dir}/${dataset_name}_n${num_series}_m${series_len}_c${num_channels}
dataset_path=${filedir}/${dataset_name}.bin
query_path=${filedir}/${dataset_name}_queries_cq${num_query_channels}_${l_min}_${l_max}.bin

# temporary files
ed_file=${filedir}/ed.txt
mass_file=${filedir}/mass.txt
mass_fft_file=${filedir}/mass_fft.txt
isax_ed_file=${filedir}/isax_ed.txt
isax_mass_file=${filedir}/isax_mass.txt
isax_mass_fft_file=${filedir}/isax_mass_fft.txt

if [ -f "${base_dir}/${dataset_path}" ]; then
    echo "Dataset already exists"
else
    echo "Creating dataset with ${num_series} series"
    bash scripts/slurm/methods/parse_csv.sh "${dataset_path}" "${l_min}" "${series_len}" "${num_series}" "${csv_files[@]}"
fi

echo "Creating queries with ${num_queries} queries"
bash scripts/slurm/methods/create_queries.sh "${dataset_path}" "${query_path}" "${series_len}" "${num_channels}" "${num_query_channels}" "${num_queries}" "${l_min}" "${l_max}"

# 1. Run Brute Force
sbatch scripts/slurm/methods/bf.sh "${query_path}" "${ed_file}" "${dataset_path}" "${k}" "${num_channels}" "${series_len}"

# 2. Run MASS
# sbatch scripts/slurm/methods/mass.sh "${query_path}" "${mass_file}" "${dataset_path}" "${k}" "${num_channels}" "${series_len}"

# 3. Run MULISSE
sbatch scripts/slurm/methods/mulisse.sh "${query_path}" "${isax_mass_file}" "${dataset_path}" "${k}" "${num_channels}" "${series_len}" "${l_min}" "${l_max}"

# 4. Run MULISSE ED
# sbatch scripts/slurm/methods/mulisse_ed.sh "${query_path}" "${isax_ed_file}" "${dataset_path}" "${k}" "${num_channels}" "${series_len}" "${l_min}" "${l_max}"

# 5. Run MULISSE ED no abandon
# sbatch scripts/slurm/methods/mulisse_ed_noearly.sh "${query_path}" "${isax_ed_file}" "${dataset_path}" "${k}" "${num_channels}" "${series_len}" "${l_min}" "${l_max}"


