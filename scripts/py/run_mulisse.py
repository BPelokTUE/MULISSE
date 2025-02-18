#!/usr/bin/env python3

import itertools
import os
import subprocess
from typing import Any, Iterator

from tqdm import tqdm

from local_settings import CSV_PATH, REPO_PATH

# -------------------#
# PARAMETERS         #
# -------------------#

run_synthetic = True
csv_data_dirs = ["stocks", "weather"]
dataset_sizes = [1000]
series_lengths = [512]
syn_num_channels = [10]
query_set_sizes = [100]
# Ratios of l_min and l_max compared to series_length
l_range_ratios = [(0.2, 1.0)]
# 0.0 => completely random selection of channels; 1.0 => all channels
used_channel_ratios = [0.0]
index_types = ["isax"]
isax_split_strategies = ["em"]
isax_breakpoint_strategies = ["equiprobable"]
isax_leaf_capacities = [64]
isax_start_bit_numbers = [4]
# Ratios of segment length compared to series_length
num_segments = [16]
# Ratios of envelope size compared to series_length - l_min + 1
envelope_size_ratios = [1.0]
scan_methods = ["scan"]
distance_measures = ["ed", "mass"]
early_abandon = [False, True]
# The FFT file path is determined automatically
precalculate_ffts = [False, True]
search_types = ["knn"]
search_ks = [1]
search_rs = [1.0]
search_approx = [False]
search_raw = [False]

# --------------------#
# LENGTH SETTINGS     #
# --------------------#

length_settings = [{"series_len": series_lengths, "l_range": l_range_ratios}]

# --------------------#
# DATASET SETTINGS    #
# --------------------#

dataset_settings = []
if run_synthetic:
    dataset_settings.append(
        {
            "command": "create_ds",
            "location": "synthetic",
            "size": dataset_sizes,
            "num_channels": syn_num_channels,
        }
    )
csv_data_paths = [os.path.join(CSV_PATH, data_dir) for data_dir in csv_data_dirs]
for path in csv_data_paths:
    dataset_settings.append(
        {
            "command": "parse_csv",
            "location": os.path.basename(path),
            "size": dataset_sizes,
            "num_channels": [len(os.listdir(path))],
        }
    )

# --------------------#
# QUERY SETTINGS      #
# --------------------#

query_settings = [{"size": query_set_sizes, "used_channel_ratio": used_channel_ratios}]

# --------------------#
# INDEX SETTINGS      #
# --------------------#

index_settings = []
isax_index_types = ["isax", "isax_envelope"]
if any(t in index_types for t in isax_index_types):
    index_settings.append(
        {
            "index_type": "isax",
            "split_strategy": isax_split_strategies,
            "breakpoint_strategy": isax_breakpoint_strategies,
            "leaf_capacity": isax_leaf_capacities,
            "first_layer_bits": isax_start_bit_numbers,
            "num_segments": num_segments,
            "pos_per_env": envelope_size_ratios,
        }
    )
    index_types = [t for t in index_types if t not in isax_index_types]

if len(index_types) > 0:
    index_settings.append(
        {
            "index_type": index_types,
            "num_segments": num_segments,
            "pos_per_env": envelope_size_ratios,
        }
    )

# -------------------#
# SEARCH SETTINGS    #
# -------------------#

method_settings_base = []
if "knn" in search_types:
    method_settings_base.append({"search_type": "knn", "k": search_ks})
if "r_range" in search_types:
    method_settings_base.append({"search_type": "r_range", "r": search_rs})

for i, settings in enumerate(method_settings_base):
    method_settings_base[i] = dict(settings, **{"approx": search_approx, "raw": search_raw})


def combine_settings(settings1, settings2):
    return [dict(**d1, **d2) for d1 in settings1 for d2 in settings2]


index_method_settings_base = [{"method_type": index_types}]
index_method_settings_base = combine_settings(index_method_settings_base, method_settings_base)
scan_method_settings_base = [{"method_type": scan_methods}]
scan_method_settings_base = combine_settings(scan_method_settings_base, method_settings_base)

index_method_settings = []
scan_method_settings = []

if any(d in distance_measures for d in ["ed", "euclidean"]):
    ed_settings = {"distance": "ed", "early_abandon": early_abandon}
    for base_setting in index_method_settings_base:
        index_method_settings.append(dict(base_setting, **ed_settings))
    for base_setting in scan_method_settings_base:
        scan_method_settings.append(dict(base_setting, **ed_settings))

if any(d in distance_measures for d in ["mass"]):
    mass_settings = {"distance": "mass", "precalculate_ffts": precalculate_ffts}
    for base_setting in index_method_settings_base:
        index_method_settings.append(dict(base_setting, **mass_settings))
    for base_setting in scan_method_settings_base:
        scan_method_settings.append(dict(base_setting, **mass_settings))

# Print settings
# print(json.dumps(length_settings))
# print(json.dumps(dataset_settings))
# print(json.dumps(query_settings))
# print(json.dumps(index_settings))
# print(json.dumps(index_method_settings))
# print(json.dumps(scan_method_settings))

# --------------------#
# ITERATOR CLASS      #
# --------------------#


class SettingIterator:
    """
    Iterator class for all setting combinations. Setting combination are calculated as the union of the Descartes
    products of the values of setting dictionaries. For example, given the settings:
    ```
    [
        {"a": [1, 2], "b": [3, 4]},
        {"a": 1, "b": 3, "c": [5, 6]}
    ]
    ```
    The following setting combinations are produced:
    ```
    {"a": 1, "b": 3}
    {"a": 1, "b": 4},
    {"a": 2, "b": 3},
    {"a": 2, "b": 4},
    {"a": 1, "b": 3, "c": 5},
    {"a": 1, "b": 3, "c": 6}
    ```
    """

    def __init__(self, settings: list[dict[str, Any]]):
        self.settings = settings
        for setting in self.settings:
            for key, value in setting.items():
                if not isinstance(value, list):
                    setting[key] = [value]

        self.all_combinations = []
        for setting in self.settings:
            keys, values = zip(*setting.items())
            for combination in itertools.product(*values):
                self.all_combinations.append(dict(zip(keys, combination)))

    def __len__(self) -> int:
        return len(self.all_combinations)

    def __iter__(self) -> Iterator[dict[str, Any]]:
        return iter(self.all_combinations)


# --------------------#
# RUN EXPERIMENTS     #
# --------------------#

DATA_DIR = os.path.join(REPO_PATH, "DATA")
LOGS_DIR = os.path.join(REPO_PATH, "LOGS")
if not os.path.exists(DATA_DIR):
    os.makedirs(DATA_DIR)
if not os.path.exists(LOGS_DIR):
    os.makedirs(LOGS_DIR)
BUILD_PATH = os.path.join(REPO_PATH, "build")
EXECUTABLE_PATH = os.path.join(BUILD_PATH, "mulisse")

dataset_counter = 0
query_counter = 0
index_counter = 0

COMMAND_LOG_PATH = os.path.join(LOGS_DIR, "command_log.txt")


def run_command_with_logging(args: list[str]):
    with open(COMMAND_LOG_PATH, "a+") as f:
        f.write(f"Running command:\n{' '.join(args)}\n")
        result = subprocess.run(args, stdout=f, stderr=subprocess.STDOUT, cwd=BUILD_PATH)
        if result.returncode != 0:
            f.write(f"Command failed with return code {result.returncode}\n")
        f.write("\n")


for length_setting in tqdm(SettingIterator(length_settings), desc="Length settings"):
    series_len = length_setting["series_len"]
    l_min = int(series_len * length_setting["l_range"][0])
    l_max = int(series_len * length_setting["l_range"][1])

    for dataset_setting in tqdm(SettingIterator(dataset_settings), desc="Dataset settings", leave=False):
        command = dataset_setting["command"]
        num_series = dataset_setting["size"]
        num_channels = dataset_setting["num_channels"]

        data_file = os.path.join(dataset_setting["location"], f"data-{dataset_counter}.bin")
        dataset_counter += 1

        args = [command, "-d", data_file, "-n", str(num_series), "-m", str(series_len)]
        if command == "parse_csv":
            args += ["-l", str(l_min)]
            csvs_dir = os.path.join(CSV_PATH, dataset_setting["location"])
            args += ["-i", *[os.path.join(csvs_dir, f) for f in os.listdir(csvs_dir)]]
        if command == "create_ds":
            args += ["-c", str(num_channels)]

        run_command_with_logging([EXECUTABLE_PATH, *args])
        ffts_required = any(
            method.get("precalculate_ffts", False)
            for method in itertools.chain(
                SettingIterator(index_method_settings),
                SettingIterator(scan_method_settings),
            )
        )
        ffts_file = os.path.join(dataset_setting["location"], f"ffts-{dataset_counter - 1}.bin")
        if ffts_required:
            # fmt: off
            run_command_with_logging([
                EXECUTABLE_PATH, "calc_ffts", "-d", data_file, "-F", ffts_file, "-m", str(series_len), "-c",
                str(num_channels),
            ])
            # fmt: on

        def get_method_args(setting):
            args = ["search"]
            for key, value in setting.items():
                if key in ["raw", "approx", "early_abandon"]:
                    if value:
                        args.append(f"--{key}")
                elif key == "precalculate_ffts":
                    if value:
                        args += ["-F", ffts_file]
                else:
                    args += [f"--{key}", str(value)]
            return args

        for query_setting in tqdm(SettingIterator(query_settings), desc="Query settings", leave=False):
            num_queries = query_setting["size"]
            used_channels = int(num_channels * query_setting["used_channel_ratio"])

            query_file = os.path.join(dataset_setting["location"], f"queries-{query_counter}.txt")
            query_counter += 1
            # fmt: off
            args = [
                "create_qs", "-d", data_file, "-q", query_file, "-c", str(num_channels), "-m", str(series_len),
                "-Q", str(num_queries), "-l", str(l_min), "-L", str(l_max), "-u", str(used_channels),
            ]
            # fmt: on
            run_command_with_logging([EXECUTABLE_PATH, *args])

            shared_args = ["-m", str(series_len), "-c", str(num_channels), "-d", data_file, "-q", query_file]
            for scan_method_setting in tqdm(
                SettingIterator(scan_method_settings), desc="Scan method settings", leave=False
            ):
                args = get_method_args(scan_method_setting) + shared_args
                run_command_with_logging([EXECUTABLE_PATH, *args])

            shared_args = [
                "-m",
                str(series_len),
                "-c",
                str(num_channels),
                "-d",
                data_file,
            ]
            for index_setting in tqdm(SettingIterator(index_settings), desc="Index settings", leave=False):
                index_file = os.path.join(dataset_setting["location"], f"index-{index_counter}.bin")
                index_counter += 1

                # fmt: off
                args = [
                    "index", "-i", index_file, "-l", str(l_min), "-L", str(l_max), "-t", 
                    index_setting.pop("index_type"), *shared_args
                ]
                # fmt: on

                if "num_segments" in index_setting:
                    args += ["-s", str(series_len // index_setting.pop("num_segments"))]
                if "pos_per_env" in index_setting:
                    max_pos_per_env = series_len - l_min + 1
                    args += [
                        "-p",
                        str(int(max_pos_per_env * index_setting.pop("pos_per_env"))),
                    ]

                for key, value in index_setting.items():
                    args += [f"--{key}", str(value)]

                run_command_with_logging([EXECUTABLE_PATH, *args])

                # fmt: off
                shared_args = [
                    "-m", str(series_len), "-c", str(num_channels), "-d", data_file, "-q", query_file, "-i", index_file
                ]
                # fmt: on
                for index_method_setting in tqdm(
                    SettingIterator(index_method_settings), desc="Indexing method settings", leave=False
                ):
                    args = get_method_args(index_method_setting) + shared_args
                    run_command_with_logging([EXECUTABLE_PATH, *args])

        #         os.remove(os.path.join(DATA_DIR, index_file))
        #     os.remove(os.path.join(DATA_DIR, query_file))
        #     if ffts_required:
        #         os.remove(os.path.join(DATA_DIR, ffts_file))
        # os.remove(os.path.join(DATA_DIR, data_file))
