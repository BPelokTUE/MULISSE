#!/bin/bash

parse_csv() {
    if [ "$#" -lt 5 ]; then
        echo "Usage: parse_csv dataset_path l_min series_len n_series csv_files..."
        return 1
    fi

    local dataset_path="$1"
    local l_min="$2"
    local series_len="$3"
    local n_series="$4"
    local csv_files=("${@:5}")  # Get remaining arguments as csv_files array

    echo "PARSING CSV FILES"
    # echo "  Dataset path: ${dataset_path}"
    # echo "  Series length: ${series_len}"
    # echo "  Number of series: ${n_series}"
    # echo "  CSV files: ${csv_files[@]}"
    ./mulisse parse_csv -d "${dataset_path}" -i "${csv_files[@]}" -l "${l_min}" -m "${series_len}" -n "${n_series}"
}

search_brute_force() {
    if [ "$#" -ne 6 ]; then
        echo "Usage: search_brute_force query_path ed_file dataset_path k n_channels series_len"
        return 1
    fi

    local query_path="$1"
    local ed_file="$2"
    local dataset_path="$3"
    local k="$4"
    local n_channels="$5"
    local series_len="$6"

    echo "BRUTE FORCE"
    ./mulisse search -q "${query_path}" -o "${ed_file}" -d "${dataset_path}" -T knn -k "${k}" -D ed -c "${n_channels}" -m "${series_len}" -t scan
}

search_ed_early() {
    if [ "$#" -ne 6 ]; then
        echo "Usage: search_ed_early query_path ed_file dataset_path k n_channels series_len"
        return 1
    fi

    local query_path="$1"
    local ed_file="$2"
    local dataset_path="$3"
    local k="$4"
    local n_channels="$5"
    local series_len="$6"

    echo "ED with early abandoning"
    ./mulisse search -q "${query_path}" -o "${ed_file}" -d "${dataset_path}" -T knn -k "${k}" -D ed -c "${n_channels}" -m "${series_len}" -t scan --early_abandon
}

search_mass() {
    if [ "$#" -ne 6 ]; then
        echo "Usage: search_mass query_path mass_file dataset_path k n_channels series_len"
        return 1
    fi

    local query_path="$1"
    local mass_file="$2"
    local dataset_path="$3"
    local k="$4"
    local n_channels="$5"
    local series_len="$6"

    echo "MASS"
    ./mulisse search -q "${query_path}" -o "${mass_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -c "${n_channels}" -m "${series_len}" -t scan
}

search_mass_fft() {
    if [ "$#" -ne 7 ]; then
        echo "Usage: search_mass_fft query_path mass_fft_file dataset_path k n_channels series_len fft_path"
        return 1
    fi

    local query_path="$1"
    local mass_fft_file="$2"
    local dataset_path="$3"
    local k="$4"
    local n_channels="$5"
    local series_len="$6"
    local fft_path="$7"

    echo "MASS with precomputed FFTs"
    ./mulisse search -q "${query_path}" -o "${mass_fft_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -F "${fft_path}" -c "${n_channels}" -m "${series_len}" -t scan
}

search_isax_ed() {
    if [ "$#" -ne 7 ]; then
        echo "Usage: search_isax_ed query_path isax_ed_file dataset_path k n_channels series_len index_path"
        return 1
    fi

    local query_path="$1"
    local isax_ed_file="$2"
    local dataset_path="$3"
    local k="$4"
    local n_channels="$5"
    local series_len="$6"
    local index_path="$7"

    echo "MULISSE ED"
    ./mulisse search -q "${query_path}" -o "${isax_ed_file}" -d "${dataset_path}" -T knn -k "${k}" -D ed -c "${n_channels}" -m "${series_len}" -i "${index_path}"
}

search_isax_ed_early() {
    if [ "$#" -ne 7 ]; then
        echo "Usage: search_isax_ed_early query_path isax_ed_file dataset_path k n_channels series_len index_path"
        return 1
    fi

    local query_path="$1"
    local isax_ed_file="$2"
    local dataset_path="$3"
    local k="$4"
    local n_channels="$5"
    local series_len="$6"
    local index_path="$7"

    echo "MULISSE ED with early abandoning"
    ./mulisse search -q "${query_path}" -o "${isax_ed_file}" -d "${dataset_path}" -T knn -k "${k}" -D ed -c "${n_channels}" -m "${series_len}" -i "${index_path}" --early_abandon
}

search_isax_mass() {
    if [ "$#" -ne 7 ]; then
        echo "Usage: search_isax_mass query_path isax_mass_file dataset_path k n_channels series_len index_path"
        return 1
    fi

    local query_path="$1"
    local isax_mass_file="$2"
    local dataset_path="$3"
    local k="$4"
    local n_channels="$5"
    local series_len="$6"
    local index_path="$7"

    echo "MULISSE MASS"
    ./mulisse search -q "${query_path}" -o "${isax_mass_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -c "${n_channels}" -m "${series_len}" -i "${index_path}"
}

search_isax_mass_fft() {
    if [ "$#" -ne 8 ]; then
        echo "Usage: search_isax_mass_fft query_path isax_mass_fft_file dataset_path k n_channels series_len index_path fft_path"
        return 1
    fi

    local query_path="$1"
    local isax_mass_fft_file="$2"
    local dataset_path="$3"
    local k="$4"
    local n_channels="$5"
    local series_len="$6"
    local index_path="$7"
    local fft_path="$8"

    echo "MULISSE MASS with precomputed FFTs"
    ./mulisse search -q "${query_path}" -o "${isax_mass_fft_file}" -d "${dataset_path}" -T knn -k "${k}" -D mass -c "${n_channels}" -m "${series_len}" -i "${index_path}" -F "${fft_path}"
}

create_queries() {
    if [ "$#" -lt 6 ]; then
        echo "Usage: create_queries dataset_path query_path series_len n_channels n_queries lengths..."
        return 1
    fi

    local dataset_path="$1"
    local query_path="$2"
    local series_len="$3"
    local n_channels="$4"
    local n_queries="$5"
    local lengths=("${@:6}")  # Get remaining arguments as lengths array

    echo "CREATING QUERIES"
    ./mulisse create_qs -d "${dataset_path}" -q "${query_path}" -m "${series_len}" \
        -c "${n_channels}" --lengths "${lengths[@]}" --noise 0.1 -Q "${n_queries}" -u "${n_channels}"
}

create_index() {
    if [ "$#" -ne 7 ]; then
        echo "Usage: create_index dataset_path index_path series_len n_channels base_cardinality word_length"
        return 1
    fi

    local dataset_path="$1"
    local index_path="$2"
    local series_len="$3"
    local n_channels="$4"
    local l_min="$5"
    local l_max="$6"
    local fft_path="$7"

    echo "CREATING INDEX"
    ./mulisse create_index -d "${dataset_path}" -i "${index_path}" -m "${series_len}" \
        -c "${n_channels}" -l "${l_min}" -L "${l_max}" -s 32 -p "$((series_len - l_min + 1))" -C 64 -F "${fft_path}" -S em
}

# Print usage if no arguments provided
if [ "$#" -eq 0 ]; then
    echo "Available functions:"
    echo "  parse_csv"
    echo "  search_brute_force"
    echo "  search_ed_early"
    echo "  search_mass"
    echo "  search_mass_fft"
    echo "  search_isax_ed"
    echo "  search_isax_ed_early"
    echo "  search_isax_mass"
    echo "  search_isax_mass_fft"
    echo "  create_queries"
    echo "  create_index"
    echo "Run any function with no arguments to see its usage."
    exit 1
fi

# Execute the function if script is run directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    "$@"
fi
