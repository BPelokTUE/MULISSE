#!/usr/bin/env python3

import argparse

import pandas as pd

# Default logs dir
LOGS_DIR = "LOGS"

# Separators
COL_SEP = ","
ITEM_SEP = ";"

# Columns
ID_COL = "id"
QUERY_ID_COL = "query_id"
SETTINGS_ID_COL = "settings_id"
RESULT_SET_TS_INDICES_COL = "result_set_ts_indices"
RESULT_SET_TS_POSITIONS_COL = "result_set_ts_positions"
RESULT_SET_DISTANCES = "result_set_distances"
SEARCH_METHOD_COL = "search_method"
DISTANCE_MEASURE_COL = "distance_measure"
INDEX_FILE_COL = "index_file"
FFTS_FILE_COL = "ffts_file"
DATASET_FILE_COL = "dataset_file"
EARLY_ABANDONING_COL = "early_abandoning"


def get_items(s: str) -> list:
    if ITEM_SEP in str(s):
        return s.split(ITEM_SEP)
    return [s]


def get_method_name(settings_df: pd.DataFrame, settings_id: int) -> str:
    setting = settings_df[settings_df[ID_COL] == settings_id].iloc[0]
    parts = [setting[SEARCH_METHOD_COL], setting[DISTANCE_MEASURE_COL]]
    if pd.notna(setting[FFTS_FILE_COL]) and setting[FFTS_FILE_COL] != "":
        parts.append("ffts")
    if pd.notna(setting[EARLY_ABANDONING_COL]) and setting[EARLY_ABANDONING_COL] != "":
        uses_early_abandon = bool(setting[EARLY_ABANDONING_COL])
        if uses_early_abandon:
            parts.append("early")
    if pd.notna(setting[INDEX_FILE_COL]) and setting[INDEX_FILE_COL] != "":
        index_name = setting[INDEX_FILE_COL].split("/")[-1].split(".")[0]
        parts.append(index_name)
    return "-".join(parts)


def results_equal(results_1: dict[str, list], results_2: dict[str, list], max_diff_ratio: float = 0.01) -> bool:
    if len(results_1) != len(results_2):
        return False

    results_size = len(results_1["ts_indices"])
    if results_size != len(results_2["ts_indices"]):
        return False

    for i in range(results_size):
        if (
            results_1["ts_indices"][i] != results_2["ts_indices"][i]
            or results_1["ts_positions"][i] != results_2["ts_positions"][i]
        ):
            max_diff = results_1["distances"][i] * max_diff_ratio
            if abs(results_1["distances"][i] - results_2["distances"][i]) > max_diff:
                return False

    return True


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
    search_settings_df = pd.read_csv(SEARCH_SETTING_CSV)
    runs_df = pd.read_csv(RUNS_CSV)

    search_settings_by_dataset: dict[str, list] = {}
    for _, row in search_settings_df.iterrows():
        dataset = row[DATASET_FILE_COL]
        if dataset not in search_settings_by_dataset:
            search_settings_by_dataset[dataset] = []
        search_settings_by_dataset[dataset].append(row[ID_COL])

    for dataset_file, search_settings_ids in search_settings_by_dataset.items():
        print(f"Checking dataset {dataset_file}")
        dataset_df = runs_df[runs_df[SETTINGS_ID_COL].isin(search_settings_ids)]
        for query_id in dataset_df[QUERY_ID_COL].unique():
            results_by_method = {}
            for _, row in dataset_df[dataset_df[QUERY_ID_COL] == query_id].iterrows():
                method_name = get_method_name(search_settings_df, row[SETTINGS_ID_COL])
                results_by_method[method_name] = {
                    "ts_indices": get_items(row[RESULT_SET_TS_INDICES_COL]),
                    "ts_positions": get_items(row[RESULT_SET_TS_POSITIONS_COL]),
                    "distances": get_items(row[RESULT_SET_DISTANCES]),
                }

            # Find discrepancies/se
            differences = []
            keys = list(results_by_method.keys())
            ref_key = keys[0]

            for key in keys[1:]:
                if not results_equal(results_by_method[ref_key], results_by_method[key]):
                    differences.append(key)

            if len(differences) > 0:
                print(f"Query {query_id} has different results:")
                print(f"\tReference ({ref_key}):")
                print(f"\t\t{results_by_method[ref_key]}")
                print("\tDifferences:")
                for key in differences:
                    print(f"\t\t{key}: {results_by_method[key]}")
                print()
