#!/usr/bin/env python3

import argparse
import itertools
import json
import math
import os
import shutil
import subprocess
from concurrent.futures import ProcessPoolExecutor
from time import time
from typing import Any, Iterator, Optional

import pandas as pd
from pydantic import BaseModel

# Config keys
CK_CSV_DATA_DIRS = "csv_data_dirs"
CK_DATASET_SIZES = "dataset_sizes"
CK_SERIES_LENGTHS = "series_lengths"
CK_LG_SEGMENTATION_STRATEGIES = "lg_segmentation_strategies"
CK_SEGMENTATION_STRATEGIES = "segmentation_strategies"
CK_NUM_SEGMENTS = "num_segments"
CK_NUM_CHANNELS = "num_channels"
CK_SYN_NUM_CHANNELS = "syn_num_channels"
CK_QUERY_SET_SIZES = "query_set_sizes"
CK_SYN_STEP_STDEVS = "syn_step_stdevs"
CK_L_RANGE_RATIOS = "l_range_ratios"
CK_USED_CHANNEL_RATIOS = "used_channel_ratios"
CK_EXACT_QUERY_LENGTH_SETS = "exact_query_length_sets"
CK_QUERY_NOISE_STDEVS = "query_noise_stdevs"
CK_SEARCH_METHODS = "search_methods"
CK_INDEX_INSERTERS = "index_inserters"
CK_ISAX_BREAKPOINT_STRATEGIES = "isax_breakpoint_strategies"
CK_ISAX_START_BIT_NUMBERS = "isax_start_bit_numbers"
CK_DISTANCE_MEASURES = "distance_measures"
CK_SEARCH_TYPES = "search_types"
CK_SEARCH_APPROX = "search_approx"
CK_SEARCH_RAW = "search_raw"
CK_DATASET_SEEDS = "dataset_seeds"
CK_QUERY_SET_SEEDS = "query_set_seeds"
CK_SEPARATE_CSV_DATASETS = "separate_csv_datasets"
CK_CALCULATE_QUERY_STATS = "calculate_query_stats"
CK_CALCULATE_INDEX_STATS = "calculate_index_stats"
CK_ADAPT_INDEX = "adapt_index"
CK_ISAX_BREAKPOINTS_FILE = "isax_breakpoints_file"
CK_ISAX_PREFER_FIRST_IN_EM = "isax_prefer_first_in_em"
CK_ISAX_SPLIT_STRATEGIES = "isax_split_strategies"
CK_ISAX_LEAF_CAP_RATIOS = "isax_leaf_cap_ratios"
CK_ISAX_NUM_BITS_LIMITS = "isax_num_bits_limits"
CK_ENVELOPE_SIZE_RATIOS = "envelope_size_ratios"
CK_LENGTH_GROUP_SIZE_RATIOS = "length_group_size_ratios"
CK_SEARCH_KS = "search_ks"
CK_SEARCH_RS = "search_rs"
CK_MAX_LEAVES_TO_VISIT = "max_leaves_to_visit"
CK_EARLY_ABANDON = "early_abandon"
CK_SORT_QUERY = "sort_query"
CK_PRECALCULATE_FFTS = "precalculate_ffts"
CK_PRIORITY_QUEUE = "priority_queue"

# Runner keys
RK_SERIES_LEN = "series_len"
RK_L_RANGE = "l_range"
RK_COMMAND = "command"
RK_LOCATION = "location"
RK_SIZE = "size"
RK_LG_SEGMENTATION_STRATEGY = "lg_segmentation_strategy"
RK_SEGMENTATION_STRATEGY = "segmentation_strategy"
RK_NUM_SEGMENTS = "num_segments"
RK_NUM_CHANNELS = "num_channels"
RK_STEP_STDEV = "step_stdev"
RK_USED_CHANNEL_RATIO = "used_channel_ratio"
RK_EXACT_QUERY_LENGTHS = "exact_query_lengths"
RK_NOISE_STDEV = "noise_stdev"
RK_INSERTER_TYPE = "inserter_type"
RK_BREAKPOINT_STRATEGY = "breakpoint_strategy"
RK_DATASET_SEED = "dataset_seed"
RK_QUERY_SET_SEED = "query_set_seed"
RK_FIRST_LAYER_BITS = "first_layer_bits"
RK_ADAPT = "adapt"
RK_ISAX_BREAKPOINTS_FILE = "isax_breakpoints_file"
RK_ISAX_PREFER_FIRST_IN_EM = "isax_prefer_first_in_em"
RK_SPLIT_STRATEGY = "split_strategy"
RK_LEAF_CAPACITY = "leaf_capacity"
RK_ISAX_NUM_BITS_LIMIT = "isax_num_bits_limit"
RK_POS_PER_ENV = "pos_per_env"
RK_LENS_PER_GROUP = "lens_per_group"
RK_INDEX_TYPE = "index_type"
RK_K = "k"
RK_R = "r"
RK_SEARCH_TYPE = "search_type"
RK_APPROX = "approx"
RK_RAW = "raw"
RK_MAX_LEAVES_TO_VISIT = "max_leaves_to_visit"
RK_METHOD_TYPE = "method_type"
RK_DISTANCE = "distance"
RK_EARLY_ABANDON = "early_abandon"
RK_SORT_QUERY = "sort_query"
RK_PRECALCULATE_FFTS = "precalculate_ffts"
RK_PRIORITY_QUEUE = "priority_queue"

# MULISSE subcommands
SUB_CREATE_DS = "create_ds"
SUB_PARSE_CSV = "parse_csv"
SUB_CALC_FFTS = "calc_ffts"
SUB_CREATE_QS = "create_qs"
SUB_CALC_Q_STATS = "calc_q_stats"
SUB_INDEX = "index"
SUB_CALC_I_STATS = "calc_i_stats"

# Data locations
LOC_SYNTHETIC = "synthetic"
LOC_STOCKS = "stocks"
LOC_WEATHER = "weather"

# Search methods
METHOD_ISAX = "isax"
METHOD_ISAX_ENVELOPE = "isax_envelope"
METHOD_SAX_ENVELOPE = "sax_envelope"
METHOD_ENVELOPE = "envelope"
METHOD_TREE_ENVELOPE = "tree_envelope"
METHOD_ISAX_ENV_W_ENV = "isax_env_w_env"
METHOD_ISAX_ENV_W_SAX_ENV = "isax_env_w_sax_env"

ENVELOPE_METHODS = [
    METHOD_ISAX_ENVELOPE,
    METHOD_SAX_ENVELOPE,
    METHOD_ENVELOPE,
    METHOD_ISAX_ENV_W_ENV,
    METHOD_ISAX_ENV_W_SAX_ENV,
    METHOD_TREE_ENVELOPE,
]
METHODS_W_FLAT_INDEX = [
    METHOD_SAX_ENVELOPE,
    METHOD_ISAX_ENV_W_SAX_ENV,
    METHOD_ENVELOPE,
    METHOD_ISAX_ENV_W_ENV,
]


# Search types
TYPE_KNN = "knn"
TYPE_R_RANGE = "r_range"

# Distance measures
DIST_ED = "ed"
DIST_EUCLIDEAN = "euclidean"
DIST_MASS = "mass"

# Local settings keys
LS_DEFAULT_RUN_CONFIG = "DEFAULT_RUN_CONFIG"
LS_CSV_PATH = "CSV_PATH"
LS_REPO_PATH = "REPO_PATH"

# Out csv column names
COL_ID = "id"
COL_SETTINGS_ID = "settings_id"

# File names
COMMAND_LOG_NAME = "command_log.txt"
LOCAL_SETTINGS_NAME = "local_settings.json"
SEARCH_SETTINGS_CSV = "search_settings.csv"
RUNS_CSV = "runs.csv"
CHECK_RESULTS_SCRIPT_PATH = "../scripts/py/check_results.py"


def check_config_keys(config: dict, required: list[str]):
    for key in required:
        if key not in config:
            raise KeyError(f"Key {key} not found in dictionary.")


Settings = list[dict[str, Any]]
SettingProfiles = dict[str, Settings]


class ParsedConfig(BaseModel):
    length_settings: Settings | SettingProfiles
    dataset_settings: Settings | SettingProfiles
    query_set_settings: Settings | SettingProfiles
    index_settings: Settings | SettingProfiles
    index_method_settings: Settings | SettingProfiles
    scan_method_settings: Settings | SettingProfiles

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


def combine_parsed_configs(parsed_configs: dict[str, ParsedConfig]) -> ParsedConfig:
    def combine_settings(setting_profiles) -> Settings | SettingProfiles:
        first_profile = next(iter(setting_profiles))
        first_settings = setting_profiles[first_profile]

        if any(len(settings) != len(first_settings) for settings in setting_profiles.values()):
            return setting_profiles

        for i in range(len(first_settings)):
            for key, value in first_settings[i].items():
                for profile, settings in setting_profiles.items():
                    if key not in settings[i] or settings[i][key] != value:
                        return setting_profiles
        return first_settings

    return ParsedConfig(
        length_settings=combine_settings({key: config.length_settings for key, config in parsed_configs.items()}),
        dataset_settings=combine_settings({key: config.dataset_settings for key, config in parsed_configs.items()}),
        query_set_settings=combine_settings({key: config.query_set_settings for key, config in parsed_configs.items()}),
        index_settings=combine_settings({key: config.index_settings for key, config in parsed_configs.items()}),
        index_method_settings=combine_settings(
            {key: config.index_method_settings for key, config in parsed_configs.items()}
        ),
        scan_method_settings=combine_settings(
            {key: config.scan_method_settings for key, config in parsed_configs.items()}
        ),
    )


def parse_config_file(input_config) -> tuple[ParsedConfig, bool, bool]:
    config = json.load(open(input_config))
    # fmt: off
    check_config_keys(
        config,
        required = [
            CK_CSV_DATA_DIRS, CK_DATASET_SIZES, CK_SERIES_LENGTHS, CK_SYN_NUM_CHANNELS, CK_QUERY_SET_SIZES,
            CK_SYN_STEP_STDEVS, CK_L_RANGE_RATIOS, CK_USED_CHANNEL_RATIOS, CK_QUERY_NOISE_STDEVS, CK_SEARCH_METHODS,
            CK_DISTANCE_MEASURES, CK_SEARCH_TYPES, CK_SEARCH_APPROX, CK_SEARCH_RAW
        ],
    )
    # fmt: on

    def parse_flat_config(config) -> ParsedConfig:
        def get_length_settings() -> Settings:
            return [{RK_SERIES_LEN: config[CK_SERIES_LENGTHS], RK_L_RANGE: config[CK_L_RANGE_RATIOS]}]

        def get_dataset_settings() -> Settings:
            dataset_seeds = config.get(CK_DATASET_SEEDS, [0])
            dataset_settings = [
                {
                    RK_COMMAND: SUB_CREATE_DS,
                    RK_LOCATION: LOC_SYNTHETIC,
                    RK_SIZE: config[CK_DATASET_SIZES],
                    RK_NUM_CHANNELS: config[CK_SYN_NUM_CHANNELS],
                    RK_STEP_STDEV: config[CK_SYN_STEP_STDEVS],
                    RK_DATASET_SEED: dataset_seeds,
                }
            ]

            separate_csv_datasets = config.get(CK_SEPARATE_CSV_DATASETS, False)
            csv_data_paths = [
                os.path.join(local_settings[LS_CSV_PATH], data_dir) for data_dir in config[CK_CSV_DATA_DIRS]
            ]
            for path in csv_data_paths:
                item = {
                    RK_COMMAND: SUB_PARSE_CSV,
                    RK_LOCATION: os.path.basename(path),
                    RK_SIZE: config[CK_DATASET_SIZES],
                    RK_NUM_CHANNELS: [len(os.listdir(path))],
                    RK_DATASET_SEED: dataset_seeds,
                }
                if not separate_csv_datasets:
                    dataset_settings.append(item)
                else:
                    item[RK_NUM_CHANNELS] = 1
                    for csv_dir in os.listdir(path):
                        item[RK_LOCATION] = os.path.join(os.path.basename(path), csv_dir)
                        dataset_settings.append(item.copy())
            return dataset_settings

        def get_query_set_settings() -> Settings:
            setting = {
                RK_SIZE: config[CK_QUERY_SET_SIZES],
                RK_USED_CHANNEL_RATIO: config[CK_USED_CHANNEL_RATIOS],
                RK_NOISE_STDEV: config[CK_QUERY_NOISE_STDEVS],
                RK_QUERY_SET_SEED: config.get(CK_QUERY_SET_SEEDS, [0]),
            }
            if CK_EXACT_QUERY_LENGTH_SETS in config:
                setting[RK_EXACT_QUERY_LENGTHS] = config[CK_EXACT_QUERY_LENGTH_SETS]
            return [setting]

        def get_index_settings() -> Settings:
            common_settings = {
                RK_RAW: config[CK_SEARCH_RAW],
                RK_LG_SEGMENTATION_STRATEGY: config.get(CK_LG_SEGMENTATION_STRATEGIES, ["single"]),
                RK_SEGMENTATION_STRATEGY: config.get(CK_SEGMENTATION_STRATEGIES, ["uniform"]),
                RK_NUM_SEGMENTS: config.get(CK_NUM_SEGMENTS, []),
                RK_INSERTER_TYPE: config.get(CK_INDEX_INSERTERS, []),
                RK_LENS_PER_GROUP: config.get(CK_LENGTH_GROUP_SIZE_RATIOS, [0]),
            }
            sax_settings = {
                **common_settings,
                RK_BREAKPOINT_STRATEGY: config.get(CK_ISAX_BREAKPOINT_STRATEGIES, []),
                RK_FIRST_LAYER_BITS: config.get(CK_ISAX_START_BIT_NUMBERS, []),
                RK_ADAPT: config.get(CK_ADAPT_INDEX, [False]),
                RK_ISAX_BREAKPOINTS_FILE: config.get(CK_ISAX_BREAKPOINTS_FILE, [""]),
                RK_ISAX_PREFER_FIRST_IN_EM: config.get(CK_ISAX_PREFER_FIRST_IN_EM, [False]),
            }
            isax_settings = {
                **sax_settings,
                RK_SPLIT_STRATEGY: config.get(CK_ISAX_SPLIT_STRATEGIES, []),
                RK_LEAF_CAPACITY: config.get(CK_ISAX_LEAF_CAP_RATIOS, []),
                RK_ISAX_NUM_BITS_LIMIT: config.get(CK_ISAX_NUM_BITS_LIMITS, [0]),
            }
            envelope_settings = {**common_settings, RK_POS_PER_ENV: config.get(CK_ENVELOPE_SIZE_RATIOS, [])}
            tree_envelope_settings = {
                **envelope_settings,
                **sax_settings,
                RK_LEAF_CAPACITY: config.get(CK_ISAX_LEAF_CAP_RATIOS, []),
            }
            isax_envelope_settings = {**isax_settings, **envelope_settings}

            index_settings = []
            if METHOD_ISAX in config[CK_SEARCH_METHODS]:
                index_settings.append({RK_INDEX_TYPE: METHOD_ISAX, **isax_settings})
            if METHOD_ISAX_ENVELOPE in config[CK_SEARCH_METHODS]:
                index_settings.append({RK_INDEX_TYPE: METHOD_ISAX_ENVELOPE, **isax_envelope_settings})
            if METHOD_SAX_ENVELOPE in config[CK_SEARCH_METHODS]:
                index_settings.append({RK_INDEX_TYPE: METHOD_SAX_ENVELOPE, **sax_settings, **envelope_settings})
            if METHOD_ENVELOPE in config[CK_SEARCH_METHODS]:
                index_settings.append({RK_INDEX_TYPE: METHOD_ENVELOPE, **envelope_settings})
            if METHOD_TREE_ENVELOPE in config[CK_SEARCH_METHODS]:
                index_settings.append({RK_INDEX_TYPE: METHOD_TREE_ENVELOPE, **tree_envelope_settings})
            if METHOD_ISAX_ENV_W_ENV in config[CK_SEARCH_METHODS]:
                index_settings.append({RK_INDEX_TYPE: METHOD_ISAX_ENV_W_ENV, **isax_envelope_settings})
            if METHOD_ISAX_ENV_W_SAX_ENV in config[CK_SEARCH_METHODS]:
                index_settings.append({RK_INDEX_TYPE: METHOD_ISAX_ENV_W_SAX_ENV, **isax_envelope_settings})

            return index_settings

        def get_method_settings(index_settings: Settings) -> tuple[Settings, Settings]:
            method_settings_base = []
            if TYPE_KNN in config[CK_SEARCH_TYPES]:
                method_settings_base.append({RK_SEARCH_TYPE: TYPE_KNN, RK_K: config.get(CK_SEARCH_KS, [])})
            if TYPE_R_RANGE in config[CK_SEARCH_TYPES]:
                method_settings_base.append({RK_SEARCH_TYPE: TYPE_R_RANGE, RK_R: config.get(CK_SEARCH_RS, [])})

            for i, settings in enumerate(method_settings_base):
                method_settings_base[i] = dict(
                    settings,
                    **{
                        RK_APPROX: config[CK_SEARCH_APPROX],
                        RK_RAW: config[CK_SEARCH_RAW],
                        RK_MAX_LEAVES_TO_VISIT: config.get(CK_MAX_LEAVES_TO_VISIT, [0]),
                    },
                )

            def combine_settings(settings1, settings2):
                return [dict(**d1, **d2) for d1 in settings1 for d2 in settings2]

            index_methods = {setting[RK_INDEX_TYPE] for setting in index_settings}
            index_method_settings_base = [
                {RK_METHOD_TYPE: [method for method in config[CK_SEARCH_METHODS] if method in index_methods]}
            ]
            index_method_settings_base = combine_settings(index_method_settings_base, method_settings_base)
            scan_method_settings_base = [
                {RK_METHOD_TYPE: [method for method in config[CK_SEARCH_METHODS] if method not in index_methods]}
            ]
            scan_method_settings_base = combine_settings(scan_method_settings_base, method_settings_base)

            index_method_settings = []
            scan_method_settings = []

            ed_settings = {
                RK_DISTANCE: DIST_ED,
                RK_EARLY_ABANDON: config.get(CK_EARLY_ABANDON, []),
                RK_SORT_QUERY: config.get(CK_SORT_QUERY, [False]),
            }
            distance_measures_settings = {
                DIST_ED: ed_settings,
                DIST_EUCLIDEAN: ed_settings,
                DIST_MASS: {RK_DISTANCE: DIST_MASS, RK_PRECALCULATE_FFTS: config.get(CK_PRECALCULATE_FFTS, [])},
            }
            for distance_measure, settings in distance_measures_settings.items():
                if distance_measure in config[CK_DISTANCE_MEASURES]:
                    for base_setting in index_method_settings_base:
                        if any(method in base_setting[RK_METHOD_TYPE] for method in METHODS_W_FLAT_INDEX):
                            base_setting[RK_PRIORITY_QUEUE] = config.get(CK_PRIORITY_QUEUE, [True])
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

    calculate_query_stats = config.get(CK_CALCULATE_QUERY_STATS, False)
    calculate_index_stats = config.get(CK_CALCULATE_INDEX_STATS, False)

    profiles: set[str] = set()
    for val in config.values():
        if isinstance(val, dict):
            profiles.update(val.keys())

    if len(profiles) == 0:
        return (parse_flat_config(config), calculate_query_stats, calculate_index_stats)

    parsed_configs: dict[str, ParsedConfig] = {}
    for profile in profiles:
        profile_config = {}
        for key, val in config.items():
            if isinstance(val, dict):
                if profile in val:
                    profile_config[key] = val[profile]
            else:
                profile_config[key] = val

        parsed_configs[profile] = parse_flat_config(profile_config)

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

    def __init__(self, settings: Settings | SettingProfiles, profile: str = ""):
        self.setting_profiles: SettingProfiles
        if isinstance(settings, dict):
            if len(profile) == 0:
                self.setting_profiles = settings
            else:
                self.setting_profiles = {profile: settings[profile]}
        else:  # if isinstance(settings, list)
            self.setting_profiles = {profile: settings}
        self.profiles = list(self.setting_profiles.keys())

        for profile, settings in self.setting_profiles.items():
            for setting in settings:
                for key, value in setting.items():
                    if not isinstance(value, list):
                        setting[key] = [value]

        self.all_combinations = []
        for profile, settings in self.setting_profiles.items():
            for setting in settings:
                keys, values = zip(*setting.items())
                for combination in itertools.product(*values):
                    self.all_combinations.append((profile, dict(zip(keys, combination))))

    def __len__(self) -> int:
        return len(self.all_combinations)

    def __iter__(self) -> Iterator[tuple[str, dict[str, Any]]]:
        return iter(self.all_combinations)

    def iterate(self, desc: str = "", leave=True):
        if input_args.progress_bar:
            return tqdm(self, desc=desc, leave=leave)
        return self.__iter__()


# --------------------#
#        Setup        #
# --------------------#

local_settings_path = LOCAL_SETTINGS_NAME
if not os.path.exists(local_settings_path):
    raise FileNotFoundError(
        f"Local settings file {local_settings_path} not found. Make sure the script is run from the root of the repository."
    )
local_settings = json.load(open(local_settings_path))
check_config_keys(local_settings, [LS_DEFAULT_RUN_CONFIG, LS_CSV_PATH, LS_REPO_PATH])

DATA_DIR = os.path.join(local_settings[LS_REPO_PATH], "DATA")
LOGS_DIR = os.path.join(local_settings[LS_REPO_PATH], "LOGS")
BUILD_PATH = os.path.join(local_settings[LS_REPO_PATH], "build")
EXECUTABLE_PATH = os.path.join(BUILD_PATH, "mulisse")

dataset_counter = 0
query_counter = 0
index_counter = 0

COMMAND_LOG_PATH = os.path.join(LOGS_DIR, COMMAND_LOG_NAME)


def run_command_with_logging(
    args: list[str], timeout: Optional[int] = None, command_log_path: str = COMMAND_LOG_PATH
) -> bool:
    with open(command_log_path, "a+") as f:
        start_time = time()
        f.write(f"Running command:\n{' '.join(args)}\n")
        try:
            result = subprocess.run(args, stdout=f, stderr=subprocess.STDOUT, cwd=BUILD_PATH, timeout=timeout)
            if result.returncode != 0:
                f.write(f"Command failed with return code {result.returncode}\n")
                return False
        except subprocess.TimeoutExpired:
            f.write(f"Command timed out after {timeout} seconds\n")
            return False
        f.write(f"\nTook: {time() - start_time:.2f} seconds\n")
        return True


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
                pd.read_csv(os.path.join(LOGS_DIR, SEARCH_SETTINGS_CSV), usecols=[COL_ID])[COL_ID].max() + 1
            )
            new_search_settings[COL_ID] += search_settings_id_base
            new_search_settings.to_csv(f_base, index=False, header=False, mode="a")
        else:
            new_search_settings.to_csv(f_base, index=False, header=True)

    # Merge runs
    runs_id_base = 0
    runs_path = os.path.join(LOGS_DIR, RUNS_CSV)
    runs_exists = os.path.exists(runs_path)
    with open(runs_path, "a+") as f_base:
        new_runs = pd.read_csv(os.path.join(logs_to_add, RUNS_CSV))
        new_runs[COL_SETTINGS_ID] += search_settings_id_base
        if runs_exists:
            runs_id_base = pd.read_csv(os.path.join(LOGS_DIR, RUNS_CSV), usecols=[COL_ID])[COL_ID].max() + 1
            new_runs[COL_ID] += runs_id_base
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
            if os.path.isfile(data_path):
                os.remove(data_path)
            else:
                shutil.rmtree(data_path)

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

    start_time = time()

    with ProcessPoolExecutor() as executor:
        for l_profile, length_setting in SettingIterator(length_settings).iterate(desc="Length settings"):
            series_len = length_setting[RK_SERIES_LEN]
            l_min = int(series_len * length_setting[RK_L_RANGE][0])
            l_max = int(series_len * length_setting[RK_L_RANGE][1])

            for d_profile, dataset_setting in SettingIterator(dataset_settings, l_profile).iterate(
                desc="Dataset settings", leave=False
            ):
                command = dataset_setting[RK_COMMAND]
                num_series = dataset_setting[RK_SIZE]
                num_channels = dataset_setting[RK_NUM_CHANNELS]
                seed = dataset_setting[RK_DATASET_SEED]

                data_file = os.path.join(dataset_setting[RK_LOCATION], f"data-{dataset_counter}.bin")
                dataset_counter += 1

                args = [command, "-d", data_file, "-n", str(num_series), "-m", str(series_len), "-S", str(seed)]
                if command == SUB_PARSE_CSV:
                    args += ["-l", str(l_min), "-L", str(l_max)]
                    csv_location = os.path.join(local_settings[LS_CSV_PATH], dataset_setting[RK_LOCATION])
                    if os.path.isdir(csv_location):
                        args += ["-i", *[os.path.join(csv_location, f) for f in os.listdir(csv_location)]]
                    else:
                        args += ["-i", csv_location]
                if command == SUB_CREATE_DS:
                    args += ["-c", str(num_channels)]
                    args += ["-s", str(dataset_setting[RK_STEP_STDEV])]

                if not run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout):
                    continue
                ffts_required = any(
                    method.get(RK_PRECALCULATE_FFTS, False)
                    for _profile, method in itertools.chain(
                        SettingIterator(index_method_settings, d_profile),
                        SettingIterator(scan_method_settings, d_profile),
                    )
                )
                ffts_calculated = False
                ffts_file = os.path.join(dataset_setting[RK_LOCATION], f"ffts-{dataset_counter - 1}.bin")
                if ffts_required:
                    # fmt: off
                    ffts_calculated = run_command_with_logging([
                        EXECUTABLE_PATH, SUB_CALC_FFTS, "-d", data_file, "-F", ffts_file, "-m", str(series_len), "-c",
                        str(num_channels), 
                    ], timeout=input_args.timeout)
                    # fmt: on

                def get_method_args(setting):
                    args = ["search"]
                    for key, value in setting.items():
                        if key in [RK_RAW, RK_APPROX, RK_EARLY_ABANDON, RK_SORT_QUERY]:
                            if value:
                                args.append(f"--{key}")
                        elif key == RK_PRIORITY_QUEUE:
                            if not value:
                                args.append("--no_pq")
                        elif key == RK_PRECALCULATE_FFTS:
                            if value:
                                args += ["-F", ffts_file]
                        else:
                            args += [f"--{key}", str(value)]
                    return args

                for q_profile, query_setting in SettingIterator(query_set_settings, d_profile).iterate(
                    desc="Query settings", leave=False
                ):
                    num_queries = query_setting[RK_SIZE]
                    used_channels = int(num_channels * query_setting[RK_USED_CHANNEL_RATIO])
                    noise_stdev = query_setting[RK_NOISE_STDEV]
                    seed = query_setting[RK_QUERY_SET_SEED]
                    exact_query_lengths = query_setting.get(RK_EXACT_QUERY_LENGTHS, [])

                    query_file = os.path.join(dataset_setting[RK_LOCATION], f"queries-{query_counter}.txt")
                    query_counter += 1
                    # fmt: off
                    args = [
                        SUB_CREATE_QS, "-d", data_file, "-q", query_file, "-c", str(num_channels), "-m", str(series_len),
                        "-Q", str(num_queries), "-u", str(used_channels), "--noise", str(noise_stdev), "-S", str(seed)
                    ]
                    if len(exact_query_lengths) > 0:
                        args += ["-e", *[str(int(q_len)) for q_len in exact_query_lengths]]
                    else:
                        args += ["-l", str(l_min), "-L", str(l_max)]
                    # fmt: on
                    queries_created = run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout)

                    if queries_created and calculate_query_stats:
                        # fmt: off
                        args = [
                            SUB_CALC_Q_STATS, "-d", data_file, "-q", query_file, "-c", str(num_channels), "-m", str(series_len),
                            "--noise", str(noise_stdev)
                        ]
                        # fmt: on
                        run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout)

                    if queries_created:
                        futures = []
                        logs_dirs = []
                        for m_ind, (sm_profile, scan_method_setting) in enumerate(
                            SettingIterator(scan_method_settings, q_profile).iterate(
                                desc="Scan method settings", leave=False
                            )
                        ):
                            if scan_method_setting.get(RK_PRECALCULATE_FFTS, False) and not ffts_calculated:
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

                    for i_profile, index_setting in SettingIterator(index_settings, q_profile).iterate(
                        desc="Index settings", leave=False
                    ):
                        index_method = index_setting[RK_INDEX_TYPE]
                        index_file = os.path.join(dataset_setting[RK_LOCATION], f"index-{index_method}-{index_counter}")
                        index_counter += 1
                        index_setting_copy = index_setting.copy()

                        # fmt: off
                        args = [
                            SUB_INDEX, "-i", index_file, "-l", str(l_min), "-L", str(l_max), "-t", 
                            index_setting_copy.pop(RK_INDEX_TYPE), "-m", str(series_len), "-c", str(num_channels), "-d", data_file 
                        ]
                        # fmt: on

                        l_range = l_max - l_min + 1
                        lens_per_group = 0
                        num_l_groups = 0

                        if index_setting_copy.pop(RK_RAW, False):
                            args += ["--raw"]
                        if RK_LENS_PER_GROUP in index_setting_copy:
                            lens_per_group = max(1, int(math.ceil(l_range * index_setting_copy.pop(RK_LENS_PER_GROUP))))
                            if lens_per_group > 0:
                                args += ["-g", str(lens_per_group)]
                                num_l_groups = (l_range + lens_per_group - 1) // lens_per_group
                        if RK_LG_SEGMENTATION_STRATEGY in index_setting_copy:
                            args += ["-G", index_setting_copy.pop(RK_LG_SEGMENTATION_STRATEGY)]
                        if RK_SEGMENTATION_STRATEGY in index_setting_copy:
                            args += ["-S", index_setting_copy.pop(RK_SEGMENTATION_STRATEGY)]
                        if RK_NUM_SEGMENTS in index_setting_copy:
                            args += ["-s", str(index_setting_copy.pop(RK_NUM_SEGMENTS))]
                        pos_per_env = 1
                        if RK_POS_PER_ENV in index_setting_copy:
                            max_pos_per_env = series_len - l_min + 1
                            pos_per_env = max(1, int(max_pos_per_env * index_setting_copy.pop(RK_POS_PER_ENV)))
                            args += ["-p", str(pos_per_env)]
                        if RK_LEAF_CAPACITY in index_setting_copy:
                            num_entries = num_series
                            if index_method == METHOD_ISAX:
                                num_entries = l_range * ((series_len - l_max + 1) + (l_range - 1) / 2) * num_series
                            elif index_method in ENVELOPE_METHODS:
                                num_entries = ((series_len - l_min + pos_per_env) // pos_per_env) * num_series
                            leaf_capacity = int(index_setting_copy.pop(RK_LEAF_CAPACITY) * num_entries)
                            leaf_capacity = max(1, leaf_capacity)
                            args += ["-C", str(leaf_capacity)]
                        if index_setting_copy.pop(RK_ADAPT, False):
                            args += ["--adapt"]
                        if RK_ISAX_BREAKPOINTS_FILE in index_setting_copy:
                            breakpoints_file = index_setting_copy.pop(RK_ISAX_BREAKPOINTS_FILE, "")
                            if len(breakpoints_file) > 0:
                                args += ["--breakpoints", breakpoints_file]
                        if index_setting_copy.pop(RK_ISAX_PREFER_FIRST_IN_EM, False):
                            args += ["--prefer_first_in_em"]
                        if RK_ISAX_NUM_BITS_LIMIT in index_setting_copy:
                            num_bits_limit = index_setting_copy.pop(RK_ISAX_NUM_BITS_LIMIT)
                            if num_bits_limit > 0:
                                args += ["--num_bits_limit", str(num_bits_limit)]

                        for key, value in index_setting_copy.items():
                            args += [f"--{key}", str(value)]

                        if run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout):
                            if calculate_index_stats:
                                # fmt: off
                                args = [
                                    SUB_CALC_I_STATS, "-i", index_file, "-c", str(num_channels), "-t", index_method,
                                ]
                                # fmt: on
                                if num_l_groups > 0:
                                    args += ["-g", str(num_l_groups)]
                                run_command_with_logging([EXECUTABLE_PATH, *args], timeout=input_args.timeout)

                            if queries_created:
                                futures = []
                                logs_dirs = []
                                for m_ind, (im_profile, index_method_setting) in enumerate(
                                    SettingIterator(index_method_settings, i_profile).iterate(
                                        desc="Indexing method settings", leave=False
                                    )
                                ):
                                    if index_method_setting[RK_METHOD_TYPE] != index_method:
                                        continue
                                    # fmt: off
                                    args = get_method_args(index_method_setting) + [
                                        "-m", str(series_len), "-c", str(num_channels), "-d", data_file, "-q",
                                        query_file, "-i", index_file
                                    ]
                                    # fmt: on
                                    if lens_per_group > 0:
                                        args += ["-g", str(lens_per_group), "-l", str(l_min), "-L", str(l_max)]

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
    run_command_with_logging([CHECK_RESULTS_SCRIPT_PATH, "-l", LOGS_DIR])

    with open(COMMAND_LOG_PATH, "a+") as f:
        f.write(f"\nTotal time: {time() - start_time:.2f} seconds\n")
        f.write(f"Total datasets created: {dataset_counter}\n")
        f.write(f"Total queries created: {query_counter}\n")
        f.write(f"Total indexes created: {index_counter}\n")
        f.write(f"Total runs: {dataset_counter * query_counter * index_counter}\n")
