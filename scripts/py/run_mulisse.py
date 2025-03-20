#!/usr/bin/env python3

import argparse
import itertools
import json
import os
import shutil
import subprocess
from concurrent.futures import ProcessPoolExecutor
from typing import Any, Iterator, Optional

import pandas as pd
from pydantic import BaseModel


def check_config_keys(config: dict, required: list[str]):
    for key in required:
        if key not in config:
            raise KeyError(f"Key {key} not found in dictionary.")


Settings = list[dict[str, Any]]


class ParsedConfig(BaseModel):
    length_settings: Settings
    dataset_settings: Settings
    query_set_settings: Settings
    index_settings: Settings
    index_method_settings: Settings
    scan_method_settings: Settings

    def __str__(self) -> str:
        string = ""
        string += "Length settings:\n"
        string += json.dumps(self.length_settings, indent=4) + "\n"
        string += "Dataset settings:\n"
        string += json.dumps(self.dataset_settings, indent=4) + "\n"
        string += "Query set settings:\n"
        string += json.dumps(self.query_set_settings, indent=4) + "\n"
        string += "Index settings:\n"
        string += json.dumps(self.index_settings, indent=4) + "\n"
        string += "Index method settings:\n"
        string += json.dumps(self.index_method_settings, indent=4) + "\n"
        string += "Scan method settings:\n"
        string += json.dumps(self.scan_method_settings, indent=4) + "\n"
        return string


def combine_parsed_configs(parsed_configs: list[ParsedConfig]) -> ParsedConfig:
    def combine_settings(settings_list: list[Settings]) -> Settings:
        combined_settings: Settings = []
        for settings in settings_list:
            combined_settings.extend(settings)
        return combined_settings

    return ParsedConfig(
        length_settings=combine_settings([config.length_settings for config in parsed_configs]),
        dataset_settings=combine_settings([config.dataset_settings for config in parsed_configs]),
        query_set_settings=combine_settings([config.query_set_settings for config in parsed_configs]),
        index_settings=combine_settings([config.index_settings for config in parsed_configs]),
        index_method_settings=combine_settings([config.index_method_settings for config in parsed_configs]),
        scan_method_settings=combine_settings([config.scan_method_settings for config in parsed_configs]),
    )


def parse_config_file(input_config) -> tuple[ParsedConfig, bool, bool]:
    config = json.load(open(input_config))
    # fmt: off
    check_config_keys(
        config,
        required = [
            "csv_data_dirs", "dataset_sizes", "series_lengths", "syn_num_channels", "query_set_sizes",
            "syn_step_stdevs", "l_range_ratios", "used_channel_ratios", "query_noise_stdevs", "search_methods",
            "distance_measures", "search_types", "search_approx", "search_raw"
        ],
    )
    # fmt: on

    def parse_flat_config(config) -> ParsedConfig:
        def get_length_settings() -> Settings:
            return [{"series_len": config["series_lengths"], "l_range": config["l_range_ratios"]}]

        def get_dataset_settings() -> Settings:
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

            separate_csv_datasets = config.get("separate_csv_datasets", False)
            csv_data_paths = [
                os.path.join(local_settings["CSV_PATH"], data_dir) for data_dir in config["csv_data_dirs"]
            ]
            for path in csv_data_paths:
                item = {
                    "command": "parse_csv",
                    "location": os.path.basename(path),
                    "size": config["dataset_sizes"],
                    "num_channels": [len(os.listdir(path))],
                    "dataset_seeds": dataset_seeds,
                }
                if not separate_csv_datasets:
                    dataset_settings.append(item)
                else:
                    item["num_channels"] = 1
                    for csv_dir in os.listdir(path):
                        item["location"] = os.path.join(os.path.basename(path), csv_dir)
                        dataset_settings.append(item.copy())
            return dataset_settings

        def get_query_set_settings() -> Settings:
            return [
                {
                    "size": config["query_set_sizes"],
                    "used_channel_ratio": config["used_channel_ratios"],
                    "noise_stdev": config["query_noise_stdevs"],
                    "query_set_seeds": config.get("query_set_seeds", [0]),
                }
            ]

        def get_index_settings() -> Settings:
            index_settings = []
            if "isax" in config["search_methods"]:
                index_settings.append(
                    {
                        "index_type": "isax",
                        "split_strategy": config.get("isax_split_strategies", []),
                        "breakpoint_strategy": config.get("isax_breakpoint_strategies", []),
                        "leaf_capacity": config.get("isax_leaf_cap_ratios", []),
                        "first_layer_bits": config.get("isax_start_bit_numbers", []),
                        "num_segments": config.get("num_segments", []),
                        "adapt": config.get("adapt_index", []),
                        "inserter_type": config.get("index_inserters", []),
                    }
                )
            if "isax_envelope" in config["search_methods"]:
                index_settings.append(
                    {
                        "index_type": "isax_envelope",
                        "split_strategy": config.get("isax_split_strategies", []),
                        "breakpoint_strategy": config.get("isax_breakpoint_strategies", []),
                        "leaf_capacity": config.get("isax_leaf_cap_ratios", []),
                        "first_layer_bits": config.get("isax_start_bit_numbers", []),
                        "num_segments": config.get("num_segments", []),
                        "pos_per_env": config.get("envelope_size_ratios", []),
                        "adapt": config.get("adapt_index", []),
                        "inserter_type": config.get("index_inserters", []),
                    }
                )
            if "envelope" in config["search_methods"]:
                index_settings.append(
                    {
                        "index_type": "envelope",
                        "num_segments": config.get("num_segments", []),
                        "pos_per_env": config.get("envelope_size_ratios", []),
                        "inserter_type": config.get("index_inserters", []),
                    }
                )
            return index_settings

        def get_method_settings(index_settings: Settings) -> tuple[Settings, Settings]:
            method_settings_base = []
            if "knn" in config["search_types"]:
                method_settings_base.append({"search_type": "knn", "k": config.get("search_ks", [])})
            if "r_range" in config["search_types"]:
                method_settings_base.append({"search_type": "r_range", "r": config.get("search_rs", [])})

            for i, settings in enumerate(method_settings_base):
                method_settings_base[i] = dict(
                    settings, **{"approx": config["search_approx"], "raw": config["search_raw"]}
                )

            def combine_settings(settings1, settings2):
                return [dict(**d1, **d2) for d1 in settings1 for d2 in settings2]

            index_methods = {setting["index_type"] for setting in index_settings}
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
            distance_measures_settings = {
                "ed": {"distance": "ed", "early_abandon": config.get("early_abandon", [])},
                "euclidean": {"distance": "ed", "early_abandon": config.get("early_abandon", [])},
                "mass": {"distance": "mass", "precalculate_ffts": config.get("precalculate_ffts", [])},
            }
            for distance_measure, settings in distance_measures_settings.items():
                if any(d in config["distance_measures"] for d in [distance_measure]):
                    for base_setting in index_method_settings_base:
                        index_method_settings.append(dict(base_setting, **settings))
                    for base_setting in scan_method_settings_base:
                        scan_method_settings.append(dict(base_setting, **settings))

            return index_method_settings, scan_method_settings

        index_settings = get_index_settings()
        index_method_settings, scan_method_settings = get_method_settings(index_settings)
        return ParsedConfig(
            length_settings=get_length_settings(),
            dataset_settings=get_dataset_settings(),
            query_set_settings=get_query_set_settings(),
            index_settings=index_settings,
            index_method_settings=index_method_settings,
            scan_method_settings=scan_method_settings,
        )

    calculate_query_stats = config.get("calculate_query_stats", False)
    calculate_index_stats = config.get("calculate_index_stats", False)

    profiles: set[str] = set()
    for val in config.values():
        if isinstance(val, dict):
            profiles.update(val.keys())

    if len(profiles) == 0:
        return (parse_flat_config(config), calculate_query_stats, calculate_index_stats)

    parsed_configs: list[ParsedConfig] = []
    for profile in profiles:
        profile_config = {}
        for key, val in config.items():
            profile_config[key] = val
            if isinstance(val, dict):
                profile_config[key] = val.get(profile, [])

        parsed_configs.append(parse_flat_config(profile_config))

    return (combine_parsed_configs(parsed_configs), calculate_query_stats, calculate_index_stats)


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

    def __init__(self, settings: Settings):
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
#        Setup        #
# --------------------#

local_settings_path = os.path.join("scripts", "local_settings.json")
if not os.path.exists(local_settings_path):
    raise FileNotFoundError(
        f"Local settings file {local_settings_path} not found. Make sure the script is run from the root of the repository."
    )
local_settings = json.load(open(local_settings_path))
check_config_keys(local_settings, ["DEFAULT_RUN_CONFIG", "CSV_PATH", "REPO_PATH"])

DATA_DIR = os.path.join(local_settings["REPO_PATH"], "DATA")
LOGS_DIR = os.path.join(local_settings["REPO_PATH"], "LOGS")
BUILD_PATH = os.path.join(local_settings["REPO_PATH"], "build")
EXECUTABLE_PATH = os.path.join(BUILD_PATH, "mulisse")

dataset_counter = 0
query_counter = 0
index_counter = 0

COMMAND_LOG_NAME = "command_log.txt"
COMMAND_LOG_PATH = os.path.join(LOGS_DIR, COMMAND_LOG_NAME)


def run_command_with_logging(
    args: list[str], timeout: Optional[int] = None, command_log_path: str = COMMAND_LOG_PATH
) -> bool:
    with open(command_log_path, "a+") as f:
        f.write(f"Running command:\n{' '.join(args)}\n")
        try:
            result = subprocess.run(args, stdout=f, stderr=subprocess.STDOUT, cwd=BUILD_PATH, timeout=timeout)
            if result.returncode != 0:
                f.write(f"Command failed with return code {result.returncode}\n")
                return False
        except subprocess.TimeoutExpired:
            f.write(f"Command timed out after {timeout} seconds\n")
            return False
        f.write("\n")
        return True


SEARCH_SETTINGS_CSV = "search_settings.csv"
RUNS_CSV = "runs.csv"


def add_logs_to_logs_dir(logs_to_add: str):
    # Merge command logs
    with open(COMMAND_LOG_PATH, "a+") as f_base:
        with open(os.path.join(logs_to_add, COMMAND_LOG_NAME), "r") as f_new:
            f_base.write(f_new.read())

    # Merge search settings
    search_settings_id_base = 0
    search_settings_path = os.path.join(LOGS_DIR, SEARCH_SETTINGS_CSV)
    search_settings_exists = os.path.exists(search_settings_path)
    with open(search_settings_path, "a+") as f_base:
        new_search_settings = pd.read_csv(os.path.join(logs_to_add, SEARCH_SETTINGS_CSV))
        if search_settings_exists:
            search_settings_id_base = (
                pd.read_csv(os.path.join(LOGS_DIR, SEARCH_SETTINGS_CSV), usecols=["id"])["id"].max() + 1
            )
            new_search_settings["id"] += search_settings_id_base
            new_search_settings.to_csv(f_base, index=False, header=False, mode="a")
        else:
            new_search_settings.to_csv(f_base, index=False, header=True)

    # Merge runs
    runs_id_base = 0
    runs_path = os.path.join(LOGS_DIR, RUNS_CSV)
    runs_exists = os.path.exists(runs_path)
    with open(runs_path, "a+") as f_base:
        new_runs = pd.read_csv(os.path.join(logs_to_add, RUNS_CSV))
        new_runs["settings_id"] += search_settings_id_base
        if runs_exists:
            runs_id_base = pd.read_csv(os.path.join(LOGS_DIR, RUNS_CSV), usecols=["id"])["id"].max() + 1
            new_runs["id"] += runs_id_base
            new_runs.to_csv(f_base, index=False, header=False, mode="a")
        else:
            new_runs.to_csv(f_base, index=False, header=True)

    # Delete added logs dir
    shutil.rmtree(logs_to_add)


if __name__ == "__main__":
    # --------------------#
    # ARGUMENT PARSING    #
    # --------------------#

    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input_config", default=local_settings["DEFAULT_RUN_CONFIG"])
    parser.add_argument("-d", "--no_cleanup", "--dirty", action="store_true", help="Do not remove generated data files")
    parser.add_argument("-p", "--print_settings", action="store_true", help="Print settings")
    parser.add_argument("-b", "--progress_bar", action="store_true", help="Show progress bar")
    parser.add_argument("-t", "--timeout", type=int, help="Timeout for each command in seconds. Default is no timeout.")
    input_args = parser.parse_args()

    if input_args.progress_bar:
        from tqdm import tqdm

    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)
    if not os.path.exists(LOGS_DIR):
        os.makedirs(LOGS_DIR)
    if not os.path.exists(input_args.input_config):
        raise FileNotFoundError(f"Config file {input_args.input_config} not found.")

    def file_cleanup(file: str):
        if input_args.no_cleanup:
            return

        data_path = os.path.join(DATA_DIR, file)
        if os.path.exists(data_path):
            os.remove(data_path)

    parsed_config, calculate_query_stats, calculate_index_stats = parse_config_file(input_args.input_config)

    if input_args.print_settings:
        print(parsed_config)

    length_settings = parsed_config.length_settings
    dataset_settings = parsed_config.dataset_settings
    query_set_settings = parsed_config.query_set_settings
    index_settings = parsed_config.index_settings
    index_method_settings = parsed_config.index_method_settings
    scan_method_settings = parsed_config.scan_method_settings

    # --------------------#
    # RUN EXPERIMENTS     #
    # --------------------#

    with ProcessPoolExecutor() as executor:
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
                    args += ["-l", str(l_min), "-L", str(l_max)]
                    csv_location = os.path.join(local_settings["CSV_PATH"], dataset_setting["location"])
                    if os.path.isdir(csv_location):
                        args += ["-i", *[os.path.join(csv_location, f) for f in os.listdir(csv_location)]]
                    else:
                        args += ["-i", csv_location]
                if command == "create_ds":
                    args += ["-c", str(num_channels)]
                    args += ["-s", str(dataset_setting["step_stdev"])]

                if not run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout):
                    continue
                ffts_required = any(
                    method.get("precalculate_ffts", False)
                    for method in itertools.chain(
                        SettingIterator(index_method_settings),
                        SettingIterator(scan_method_settings),
                    )
                )
                ffts_calculated = False
                ffts_file = os.path.join(dataset_setting["location"], f"ffts-{dataset_counter - 1}.bin")
                if ffts_required:
                    # fmt: off
                    ffts_calculated = run_command_with_logging([
                        EXECUTABLE_PATH, "calc_ffts", "-d", data_file, "-F", ffts_file, "-m", str(series_len), "-c",
                        str(num_channels), 
                    ], timeout=input_args.timeout)
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
                    queries_created = run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout)

                    if queries_created and calculate_query_stats:
                        # fmt: off
                        args = [
                            "calc_q_stats", "-d", data_file, "-q", query_file, "-c", str(num_channels), "-m", str(series_len),
                            "--noise", str(noise_stdev)
                        ]
                        # fmt: on
                        run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout)

                    if queries_created:
                        futures = []
                        logs_dirs = []
                        for m_ind, scan_method_setting in enumerate(
                            SettingIterator(scan_method_settings).iterate(desc="Scan method settings", leave=False)
                        ):
                            if scan_method_setting.get("precalculate_ffts", False) and not ffts_calculated:
                                continue
                            args = get_method_args(scan_method_setting)
                            args += ["-m", str(series_len), "-c", str(num_channels), "-d", data_file, "-q", query_file]
                            # Save results into separate log file
                            logs_dirs.append(f"{LOGS_DIR}_{m_ind}")
                            args += ["--logs", logs_dirs[-1]]
                            os.makedirs(logs_dirs[-1], exist_ok=True)

                            futures.append(
                                executor.submit(
                                    run_command_with_logging,
                                    [EXECUTABLE_PATH, *args],
                                    timeout=input_args.timeout,
                                    command_log_path=os.path.join(logs_dirs[-1], COMMAND_LOG_NAME),
                                )
                            )

                        # Wait for all futures to complete
                        for logs_dir, future in zip(logs_dirs, futures):
                            future.result()
                            add_logs_to_logs_dir(logs_dir)

                    for index_setting in SettingIterator(index_settings).iterate(desc="Index settings", leave=False):
                        index_method = index_setting["index_type"]
                        index_file = os.path.join(
                            dataset_setting["location"], f"index-{index_method}-{index_counter}.bin"
                        )
                        index_counter += 1
                        index_setting_copy = index_setting.copy()

                        # fmt: off
                        args = [
                            "index", "-i", index_file, "-l", str(l_min), "-L", str(l_max), "-t", 
                            index_setting_copy.pop("index_type"), "-m", str(series_len), "-c", str(num_channels), "-d", data_file 
                        ]
                        # fmt: on

                        pos_per_env = 1
                        if "num_segments" in index_setting_copy:
                            args += ["-s", str(series_len // index_setting_copy.pop("num_segments"))]
                        if "pos_per_env" in index_setting_copy:
                            max_pos_per_env = series_len - l_min + 1
                            pos_per_env = int(max_pos_per_env * index_setting_copy.pop("pos_per_env"))
                            args += ["-p", str(pos_per_env)]
                        if "leaf_capacity" in index_setting_copy:
                            num_entries = num_series
                            if index_method == "isax":
                                l_range = l_max - l_min + 1
                                num_entries = l_range * ((series_len - l_max + 1) + (l_range - 1) / 2) * num_series
                            elif index_method == "isax_envelope":
                                num_entries = ((series_len - l_min + pos_per_env) // pos_per_env) * num_series
                            leaf_capacity = int(index_setting_copy.pop("leaf_capacity") * num_entries)
                            leaf_capacity = max(1, leaf_capacity)
                            args += ["-C", str(leaf_capacity)]
                        if index_setting_copy.pop("adapt", False):
                            args += ["--adapt"]

                        for key, value in index_setting_copy.items():
                            args += [f"--{key}", str(value)]

                        if run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout):
                            if calculate_index_stats:
                                args = ["calc_i_stats", "-i", index_file, "-c", str(num_channels), "-t", index_method]
                                run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout)

                            if queries_created:
                                relevant_search_settings = index_method_settings.copy()
                                for i in range(len(relevant_search_settings)):
                                    relevant_search_settings[i]["method_type"] = [index_method]

                                futures = []
                                logs_dirs = []
                                for m_ind, index_method_setting in enumerate(
                                    SettingIterator(relevant_search_settings).iterate(
                                        desc="Indexing method settings", leave=False
                                    )
                                ):
                                    # fmt: off
                                    args = get_method_args(index_method_setting) + [
                                        "-m", str(series_len), "-c", str(num_channels), "-d", data_file, "-q", query_file, "-i", index_file
                                    ]
                                    # fmt: on
                                    logs_dirs.append(f"{LOGS_DIR}_{m_ind}")
                                    args += ["--logs", logs_dirs[-1]]
                                    os.makedirs(logs_dirs[-1], exist_ok=True)

                                    futures.append(
                                        executor.submit(
                                            run_command_with_logging,
                                            [EXECUTABLE_PATH, *args],
                                            timeout=input_args.timeout,
                                            command_log_path=os.path.join(logs_dirs[-1], COMMAND_LOG_NAME),
                                        )
                                    )

                            # Wait for all futures to complete
                            for logs_dir, future in zip(logs_dirs, futures):
                                future.result()
                                add_logs_to_logs_dir(logs_dir)

                        file_cleanup(index_file)
                    file_cleanup(query_file)
                file_cleanup(data_file)
                file_cleanup(ffts_file)

    # Run check
    run_command_with_logging(["../scripts/py/check_results.py", "-l", LOGS_DIR])
