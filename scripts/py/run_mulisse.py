#!/usr/bin/env python3

import argparse
import itertools
import json
import os
import subprocess
from typing import Any, Iterator


def require_keys(d: dict, keys: list[str]):
    for key in keys:
        if key not in d:
            raise KeyError(f"Key {key} not found in dictionary.")


if __name__ == "__main__":
    # --------------------#
    # ARGUMENT PARSING    #
    # --------------------#

    local_settings_path = os.path.join("scripts", "local_settings.json")
    if not os.path.exists(local_settings_path):
        raise FileNotFoundError(
            f"Local settings file {local_settings_path} not found. Make sure the script is run from the root of the repository."
        )
    local_settings = json.load(open(local_settings_path))
    require_keys(local_settings, ["DEFAULT_RUN_CONFIG", "CSV_PATH", "REPO_PATH"])

    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input_config", default=local_settings["DEFAULT_RUN_CONFIG"])
    parser.add_argument("-d", "--no_cleanup", "--dirty", action="store_true", help="Do not remove generated data files")
    parser.add_argument("-s", "--print_settings", action="store_true", help="Print settings")
    parser.add_argument("-b", "--progress_bar", action="store_true", help="Show progress bar")
    input_args = parser.parse_args()

    if input_args.progress_bar:
        from tqdm import tqdm

    if not os.path.exists(input_args.input_config):
        raise FileNotFoundError(f"Config file {input_args.input_config} not found.")

    config = json.load(open(input_args.input_config))
    # fmt: off
    require_keys(
        config,
        [
            "csv_data_dirs", "dataset_sizes", "series_lengths", "syn_num_channels", "query_set_sizes",
            "syn_step_stdevs", "l_range_ratios", "used_channel_ratios", "query_noise_stdevs", "search_methods",
            "isax_split_strategies", "isax_breakpoint_strategies", "isax_leaf_capacities", "isax_start_bit_numbers",
            "num_segments", "envelope_size_ratios", "distance_measures", "early_abandon", "precalculate_ffts", 
            "search_types", "search_ks", "search_rs", "search_approx", "search_raw"
        ],
    )
    # fmt: on

    def file_cleanup(file: str):
        if input_args.no_cleanup:
            return

        data_path = os.path.join(DATA_DIR, file)
        if os.path.exists(data_path):
            os.remove(data_path)

    # --------------------#
    # LENGTH SETTINGS     #
    # --------------------#

    length_settings = [{"series_len": config["series_lengths"], "l_range": config["l_range_ratios"]}]

    # --------------------#
    # DATASET SETTINGS    #
    # --------------------#

    dataset_seeds = config.get("dataset_seeds", [0])
    dataset_settings = [
        {
            "command": "create_ds",
            "location": "synthetic",
            "size": config["dataset_sizes"],
            "num_channels": config["syn_num_channels"],
            "step_stdev": config["syn_step_stdevs"],
            "dataset_seeds": dataset_seeds,
        }
    ]
    csv_data_paths = [os.path.join(local_settings["CSV_PATH"], data_dir) for data_dir in config["csv_data_dirs"]]
    for path in csv_data_paths:
        dataset_settings.append(
            {
                "command": "parse_csv",
                "location": os.path.basename(path),
                "size": config["dataset_sizes"],
                "num_channels": [len(os.listdir(path))],
                "dataset_seeds": dataset_seeds,
            }
        )

    # --------------------#
    # QUERY SET SETTINGS  #
    # --------------------#

    query_set_settings = [
        {
            "size": config["query_set_sizes"],
            "used_channel_ratio": config["used_channel_ratios"],
            "noise_stdev": config["query_noise_stdevs"],
            "query_set_seeds": config.get("query_set_seeds", [0]),
        }
    ]
    calculate_query_stats = config.get("calculate_query_stats", False)

    # --------------------#
    # INDEX SETTINGS      #
    # --------------------#

    index_settings = []
    isax_index_methods = ["isax", "isax_envelope"]
    non_isax_index_methods: list[str] = []
    index_methods = non_isax_index_methods + isax_index_methods

    isax_methods_in_config = [t for t in config["search_methods"] if t in isax_index_methods]
    index_settings.append(
        {
            "index_type": isax_methods_in_config,
            "split_strategy": config["isax_split_strategies"],
            "breakpoint_strategy": config["isax_breakpoint_strategies"],
            "leaf_capacity": config["isax_leaf_capacities"],
            "first_layer_bits": config["isax_start_bit_numbers"],
            "num_segments": config["num_segments"],
            "pos_per_env": config["envelope_size_ratios"],
        }
    )

    non_isax_indexes = [t for t in config["search_methods"] if t in non_isax_index_methods]
    if len(non_isax_indexes) > 0:
        index_settings.append(
            {
                "index_type": non_isax_indexes,
                "num_segments": config["num_segments"],
                "pos_per_env": config["envelope_size_ratios"],
            }
        )

    # -------------------#
    # SEARCH SETTINGS    #
    # -------------------#

    method_settings_base = []
    if "knn" in config["search_types"]:
        method_settings_base.append({"search_type": "knn", "k": config["search_ks"]})
    if "r_range" in config["search_types"]:
        method_settings_base.append({"search_type": "r_range", "r": config["search_rs"]})

    for i, settings in enumerate(method_settings_base):
        method_settings_base[i] = dict(settings, **{"approx": config["search_approx"], "raw": config["search_raw"]})

    def combine_settings(settings1, settings2):
        return [dict(**d1, **d2) for d1 in settings1 for d2 in settings2]

    index_method_settings_base = [
        {"method_type": [method for method in config["search_methods"] if method in index_methods]}
    ]
    index_method_settings_base = combine_settings(index_method_settings_base, method_settings_base)
    scan_method_settings_base = [
        {"method_type": [method for method in config["search_methods"] if method not in index_methods]}
    ]
    scan_method_settings_base = combine_settings(scan_method_settings_base, method_settings_base)

    index_method_settings = []
    scan_method_settings = []

    if any(d in config["distance_measures"] for d in ["ed", "euclidean"]):
        ed_settings = {"distance": "ed", "early_abandon": config["early_abandon"]}
        for base_setting in index_method_settings_base:
            index_method_settings.append(dict(base_setting, **ed_settings))
        for base_setting in scan_method_settings_base:
            scan_method_settings.append(dict(base_setting, **ed_settings))

    if any(d in config["distance_measures"] for d in ["mass"]):
        mass_settings = {"distance": "mass", "precalculate_ffts": config["precalculate_ffts"]}
        for base_setting in index_method_settings_base:
            index_method_settings.append(dict(base_setting, **mass_settings))
        for base_setting in scan_method_settings_base:
            scan_method_settings.append(dict(base_setting, **mass_settings))

    if input_args.print_settings:
        settings_dict = {
            "Length": length_settings,
            "Dataset": dataset_settings,
            "Query": query_set_settings,
            "Index": index_settings,
            "Index method": index_method_settings,
            "Scan method": scan_method_settings,
        }
        for name, s in settings_dict.items():
            print(f"{name} settings:")
            print(json.dumps(s, indent=4))
            print()

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

        def iterate(self, desc: str = "", leave=True):
            if input_args.progress_bar:
                return tqdm(self, desc=desc, leave=leave)
            return self.__iter__()

    # --------------------#
    # RUN EXPERIMENTS     #
    # --------------------#

    DATA_DIR = os.path.join(local_settings["REPO_PATH"], "DATA")
    LOGS_DIR = os.path.join(local_settings["REPO_PATH"], "LOGS")
    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)
    if not os.path.exists(LOGS_DIR):
        os.makedirs(LOGS_DIR)
    BUILD_PATH = os.path.join(local_settings["REPO_PATH"], "build")
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

    for length_setting in SettingIterator(length_settings).iterate(desc="Length settings"):
        series_len = length_setting["series_len"]
        l_min = int(series_len * length_setting["l_range"][0])
        l_max = int(series_len * length_setting["l_range"][1])

        for dataset_setting in SettingIterator(dataset_settings).iterate(desc="Dataset settings", leave=False):
            command = dataset_setting["command"]
            num_series = dataset_setting["size"]
            num_channels = dataset_setting["num_channels"]
            seed = dataset_setting["dataset_seeds"]

            data_file = os.path.join(dataset_setting["location"], f"data-{dataset_counter}.bin")
            dataset_counter += 1

            args = [command, "-d", data_file, "-n", str(num_series), "-m", str(series_len), "-S", str(seed)]
            if command == "parse_csv":
                args += ["-l", str(l_min)]
                csvs_dir = os.path.join(local_settings["CSV_PATH"], dataset_setting["location"])
                args += ["-i", *[os.path.join(csvs_dir, f) for f in os.listdir(csvs_dir)]]
            if command == "create_ds":
                args += ["-c", str(num_channels)]
                args += ["-s", str(dataset_setting["step_stdev"])]

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

            for query_setting in SettingIterator(query_set_settings).iterate(desc="Query settings", leave=False):
                num_queries = query_setting["size"]
                used_channels = int(num_channels * query_setting["used_channel_ratio"])
                noise_stdev = query_setting["noise_stdev"]
                seed = query_setting["query_set_seeds"]

                query_file = os.path.join(dataset_setting["location"], f"queries-{query_counter}.txt")
                query_counter += 1
                # fmt: off
                args = [
                    "create_qs", "-d", data_file, "-q", query_file, "-c", str(num_channels), "-m", str(series_len),
                    "-Q", str(num_queries), "-l", str(l_min), "-L", str(l_max), "-u", str(used_channels), "--noise",
                    str(noise_stdev), "-S", str(seed)
                ]
                # fmt: on
                run_command_with_logging([EXECUTABLE_PATH, *args])

                if calculate_query_stats:
                    # fmt: off
                    args = [
                        "calc_q_stats", "-d", data_file, "-q", query_file, "-c", str(num_channels), "-m", str(series_len),
                        "--noise", str(noise_stdev)
                    ]
                    # fmt: on
                    run_command_with_logging([EXECUTABLE_PATH, *args])

                for scan_method_setting in SettingIterator(scan_method_settings).iterate(
                    desc="Scan method settings", leave=False
                ):
                    # fmt: off
                    args = get_method_args(scan_method_setting) + [
                        "-m", str(series_len), "-c", str(num_channels), "-d", data_file, "-q", query_file
                    ]
                    # fmt: on
                    run_command_with_logging([EXECUTABLE_PATH, *args])

                for index_setting in SettingIterator(index_settings).iterate(desc="Index settings", leave=False):
                    index_file = os.path.join(dataset_setting["location"], f"index-{index_counter}.bin")
                    index_counter += 1
                    index_setting_copy = index_setting.copy()

                    # fmt: off
                    args = [
                        "index", "-i", index_file, "-l", str(l_min), "-L", str(l_max), "-t", 
                        index_setting_copy.pop("index_type"), "-m", str(series_len), "-c", str(num_channels), "-d", data_file 
                    ]
                    # fmt: on

                    if "num_segments" in index_setting_copy:
                        args += ["-s", str(series_len // index_setting_copy.pop("num_segments"))]
                    if "pos_per_env" in index_setting_copy:
                        max_pos_per_env = series_len - l_min + 1
                        args += [
                            "-p",
                            str(int(max_pos_per_env * index_setting_copy.pop("pos_per_env"))),
                        ]

                    for key, value in index_setting_copy.items():
                        args += [f"--{key}", str(value)]

                    run_command_with_logging([EXECUTABLE_PATH, *args])

                    for index_method_setting in SettingIterator(index_method_settings).iterate(
                        desc="Indexing method settings", leave=False
                    ):
                        # fmt: off
                        args = get_method_args(index_method_setting) + [
                            "-m", str(series_len), "-c", str(num_channels), "-d", data_file, "-q", query_file, "-i", index_file
                        ]
                        # fmt: on
                        run_command_with_logging([EXECUTABLE_PATH, *args])

                    file_cleanup(index_file)
                file_cleanup(query_file)
            file_cleanup(data_file)
            file_cleanup(ffts_file)
