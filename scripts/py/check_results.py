import argparse
import pandas

# Default logs dir
LOGS_DIR = "LOGS"

# Separators
COL_SEP = ","
ITEM_SEP = ";"

# Columns
ID_COL = "id"
QUERY_ID_COL = "query_id"
SETTINGS_ID_COL = "settings_id"
RESULTS_SET_TS_INDICES_COL = "result_set_ts_indices"
RESULTS_SET_TS_POSITIONS_COL = "result_set_ts_positions"
SEARCH_METHOD_COL = "search_method"
DISTANCE_MEASURE_COL = "distance_measure"
FFTS_FILE_COL = "ffts_file"


def get_method_name(settings_df: pandas.DataFrame, settings_id: int) -> str:
    setting = settings_df[settings_df[ID_COL] == settings_id].iloc[0]
    parts = [setting[SEARCH_METHOD_COL], setting[DISTANCE_MEASURE_COL]]
    if pandas.notna(setting[FFTS_FILE_COL]) and setting[FFTS_FILE_COL] != "":
        parts.append("ffts")
    return "-".join(parts)


if __name__ == "__main__":
    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-l",
        "--logs_dir",
        default=LOGS_DIR,
        help="Directory containing the logs",
    )
    args = parser.parse_args()

    logs_dir = args.logs_dir
    DATASET_SETTINGS_CSV = f"{logs_dir}/dataset_settings.csv"
    INDEX_SETTINGS_CSV = f"{logs_dir}/index_settings.csv"
    SEARCH_SETTING_CSV = f"{logs_dir}/search_settings.csv"
    RUNS_CSV = f"{logs_dir}/runs.csv"

    # Check results
    search_settings_df = pandas.read_csv(SEARCH_SETTING_CSV)
    runs_df = pandas.read_csv(RUNS_CSV)

    differences_found = False
    for query_id in runs_df[QUERY_ID_COL].unique():
        results_by_method = {}
        for _, row in runs_df[runs_df[QUERY_ID_COL] == query_id].iterrows():
            ts_indices_str = row[RESULTS_SET_TS_INDICES_COL]
            ts_positions_str = row[RESULTS_SET_TS_POSITIONS_COL]
            method_name = get_method_name(search_settings_df, row[SETTINGS_ID_COL])

            results_by_method[method_name] = {
                "ts_indices": ts_indices_str.split(ITEM_SEP),
                "ts_positions": ts_positions_str.split(ITEM_SEP),
            }

        # Find discrepancies
        differences = []
        keys = list(results_by_method.keys())
        ref_key = keys[0]
        ref_ts_indices = results_by_method[ref_key]["ts_indices"]
        ref_ts_positions = results_by_method[ref_key]["ts_positions"]

        for key in keys[1:]:
            ts_indices = results_by_method[key]["ts_indices"]
            ts_positions = results_by_method[key]["ts_positions"]

            if ts_indices != ref_ts_indices or ts_positions != ref_ts_positions:
                differences.append(key)
                differences_found = True

        if len(differences) > 0:
            print(f"Query {query_id} has different results:")
            print(f"\tReference:")
            print(f"\t\t{results_by_method[ref_key]}")
            print("\tDifferences:")
            for key in differences:
                print(f"\t\t{key}: {results_by_method[key]}")
            print()

    if not differences_found:
        print("No differences found.")
