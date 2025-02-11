if [ "$#" -lt 6 ]; then
    echo "Usage: create_queries dataset_path query_path series_len n_channels n_queries lengths..."
    return 1
fi

dataset_path="$1"
query_path="$2"
series_len="$3"
n_channels="$4"
n_queries="$5"
lengths=("${@:6}")  # Get remaining arguments as lengths array

echo "CREATING QUERIES"
./mulisse create_qs -d "${dataset_path}" -q "${query_path}" -m "${series_len}" \
    -c "${n_channels}" --lengths "${lengths[@]}" --noise 0.1 -Q "${n_queries}" -u "${n_channels}"