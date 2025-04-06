# %%[markdown]
"""
___
# Experiment Analysis

This script is responsible for analyzing the results of experiments. The first section defines helper classes
and functions to facilitate the analysis of experiment results. The second section runs the experiment analyses.

___
## Helper Classes and Functions

This script contains the necessary classes and functions to analyze the results of experiments, including:
- `ExperimentResults`: A class to load and store the results of experiments.
- `Reducer` and derivatives thereof: Classes to reduce the results of experiments.
- `execute_reduction`: A function to execute a reduction on the results of experiments.
"""

# %%

import os
from enum import Enum, auto
from typing import Any

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from pydantic import BaseModel

if os.getcwd().endswith("scripts/py"):
    os.chdir("../..")

from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import IndexSettingsColumn as ISC
from scripts.py.common.columns import IndexStatsColumn as ISTC
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import QuerySetSettingsColumn as QSC
from scripts.py.common.columns import QueryStatsColumn as QSTC
from scripts.py.common.columns import SearchSettingsColumn as SSC
from scripts.py.common.columns import StatsColumnPrefix as SCP
from scripts.py.common.columns import get_stats_col
from scripts.py.common.style import PALETTE
from scripts.py.common.utils import COLS_FOR_METHOD_NAME, define_method_name_col

# %%[markdown]
"""
### Experiment Results
"""

# %%


class ExperimentResultDataframe(Enum):
    DATASETS_COLS = auto()
    QUERY_SETS_COLS = auto()
    INDEXES_COLS = auto()
    METHODS_COLS = auto()
    RUNS_COLS = auto()
    INDEX_STATS_COLS = auto()
    QUERY_STATS_COLS = auto()

    def __str__(self):
        return self.name.lower()


ERD = ExperimentResultDataframe

REQUIRED_COLS = {
    ERD.DATASETS_COLS: [str(DSC.DATASET_FILE)],
    ERD.QUERY_SETS_COLS: [str(QSC.DATASET_FILE)],
    ERD.INDEXES_COLS: [str(ISC.DATASET_FILE), str(ISC.INDEX_FILE)],
    ERD.METHODS_COLS: [str(SSC.DATASET_FILE), str(SSC.INDEX_FILE), str(SSC.ID)],
    ERD.RUNS_COLS: [str(QC.SETTINGS_ID)],
    ERD.INDEX_STATS_COLS: [str(ISTC.INDEX_FILE)],
    ERD.QUERY_STATS_COLS: [str(QSTC.DATASET_FILE)],
}
CSV_FILES = {
    ERD.DATASETS_COLS: "dataset_settings.csv",
    ERD.QUERY_SETS_COLS: "query_set_settings.csv",
    ERD.INDEXES_COLS: "index_settings.csv",
    ERD.METHODS_COLS: "search_settings.csv",
    ERD.RUNS_COLS: "runs.csv",
    ERD.INDEX_STATS_COLS: "index_stats.csv",
    ERD.QUERY_STATS_COLS: "query_stats.csv",
}

MERGED_COL_NAME_SEP = "::"


def get_merged_col_name(df_name: ERD, col_name: str) -> str:
    return f"{str(df_name)}{MERGED_COL_NAME_SEP}{col_name}"


def rename_df_columns(df: pd.DataFrame, df_name: ERD) -> pd.DataFrame:
    return df.rename(columns={col: get_merged_col_name(df_name, col) for col in df.columns})


class ExperimentResults(BaseModel):
    logs_dir: str
    datasets_df: pd.DataFrame
    query_sets_df: pd.DataFrame
    indexes_df: pd.DataFrame
    methods_df: pd.DataFrame
    runs_df: pd.DataFrame
    index_stats_df: pd.DataFrame
    query_stats_df: pd.DataFrame

    class Config:
        arbitrary_types_allowed = True

    @classmethod
    def add_extra_cols_for_pruning_ratio(
        cls,
        logs_dir: str,
        extra_cols: dict[ERD, list[str]],
    ):
        extra_cols[ERD.DATASETS_COLS] += [str(DSC.NUM_SERIES), str(DSC.SERIES_LENGTH)]
        extra_cols[ERD.INDEXES_COLS] += [str(ISC.POS_PER_ENV)]
        extra_cols[ERD.METHODS_COLS] += [str(SSC.SEARCH_METHOD)]
        extra_cols[ERD.RUNS_COLS] += [str(QC.ID)]

        runs_header = pd.read_csv(os.path.join(logs_dir, CSV_FILES[ERD.RUNS_COLS]), nrows=0)
        if str(QC.NUM_ENTRIES_EXAMINED) in runs_header.columns:
            extra_cols[ERD.RUNS_COLS] += [str(QC.NUM_ENTRIES_EXAMINED), str(QC.QUERY_LENGTH)]
        else:  # Handle case for backward compatibility
            extra_cols[ERD.INDEXES_COLS] += [str(ISC.L_MIN), str(ISC.L_MAX)]
            extra_cols[ERD.RUNS_COLS] += [str(QC.NUM_TS_EXAMINED)]

    def add_pruning_ratio_column(self):
        merged_df = self.get_merged_df()
        dsc_num_series = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.NUM_SERIES))
        dsc_series_length = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.SERIES_LENGTH))
        ssc_search_method = get_merged_col_name(ERD.METHODS_COLS, str(SSC.SEARCH_METHOD))
        isc_pos_per_env = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.POS_PER_ENV))

        if str(QC.NUM_ENTRIES_EXAMINED) in self.runs_df.columns:
            qc_query_length = get_merged_col_name(ERD.RUNS_COLS, str(QC.QUERY_LENGTH))
            merged_df["num_relevant_entries"] = np.where(
                merged_df[ssc_search_method].str.contains("env"),
                (merged_df[dsc_series_length] - merged_df[qc_query_length] + merged_df[isc_pos_per_env])
                // merged_df[isc_pos_per_env],
                np.where(
                    merged_df[ssc_search_method].str.contains("isax"),
                    merged_df[dsc_series_length] - merged_df[qc_query_length] + 1,
                    1,
                ),
            )
            merged_df["num_relevant_entries"] *= merged_df[dsc_num_series]
            qc_num_entries_examined = get_merged_col_name(ERD.RUNS_COLS, str(QC.NUM_ENTRIES_EXAMINED))
            qc_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.ID))

            merged_df[str(QC.PRUNING_RATIO)] = np.clip(
                1.0 - merged_df[qc_num_entries_examined] / merged_df["num_relevant_entries"], 0.0, 1.0
            )
        else:  # Handle case for backward compatibility
            isc_l_min = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.L_MIN))
            isc_l_max = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.L_MAX))
            qc_num_ts_examined = get_merged_col_name(ERD.RUNS_COLS, str(QC.NUM_TS_EXAMINED))
            qc_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.ID))

            # MULISSE counts one examined series per envelope, iSAX counts one per subsequence
            merged_df["num_start"] = merged_df[dsc_series_length] - merged_df[isc_l_min] + merged_df[isc_pos_per_env]
            merged_df["l_range"] = merged_df[isc_l_max] - merged_df[isc_l_min] + 1
            merged_df["num_subs"] = merged_df["l_range"] * (
                (merged_df[dsc_series_length] - merged_df[isc_l_max] + 1) + (merged_df["l_range"] - 1) / 2
            )

            merged_df["num_series_multiplier"] = np.where(
                merged_df[ssc_search_method].str.contains("isax_envelope"),
                merged_df["num_start"] // merged_df[isc_pos_per_env],
                np.where(merged_df[ssc_search_method].str.contains("isax"), merged_df["num_subs"], 1.0),
            )
            merged_df[str(QC.PRUNING_RATIO)] = 1.0 - merged_df[qc_num_ts_examined] / (
                merged_df[dsc_num_series] * merged_df["num_series_multiplier"]
            )

        merged_df = merged_df[[str(QC.PRUNING_RATIO), qc_id]]
        self.runs_df = self.runs_df.merge(merged_df, left_on=str(QC.ID), right_on=qc_id, how="left")

    @classmethod
    def add_extra_cols_for_amortized_prep_time(
        cls,
        extra_cols: dict[ERD, list[str]],
    ):
        extra_cols[ERD.METHODS_COLS] += [str(SSC.ID), str(SSC.INDEX_FILE), str(SSC.FFTS_FILE)]
        extra_cols[ERD.INDEXES_COLS] += [str(ISC.INDEXING_TIME_S), str(ISC.FFT_CALC_TIME_S)]

    def add_amortized_prep_time_column(self):
        merged_df = self.get_merged_df()

        isc_indexing_time = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEXING_TIME_S))
        isc_fft_calc_time = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.FFT_CALC_TIME_S))
        qc_settings_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.SETTINGS_ID))

        merged_df["num_runs"] = merged_df.groupby(qc_settings_id)[qc_settings_id].transform("count")

        cols_to_drop = [
            col for col in merged_df.columns if col.startswith(str(ERD.RUNS_COLS)) and col != qc_settings_id
        ]
        merged_df = merged_df.drop(columns=cols_to_drop)
        merged_df = merged_df.drop_duplicates()

        merged_df[str(QC.AMORTIZED_PREP_TIME_S)] = (
            merged_df[isc_indexing_time] + merged_df[isc_fft_calc_time]
        ) / merged_df["num_runs"]
        merged_df = merged_df[[str(QC.AMORTIZED_PREP_TIME_S), qc_settings_id]]

        self.runs_df = self.runs_df.merge(merged_df, left_on=str(QC.SETTINGS_ID), right_on=qc_settings_id, how="left")

    def add_leaf_fill_columns(self, leaf_fill_columns: list[str]):
        merged_df = self.get_merged_df()
        isc_leaf_capacity = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.LEAF_CAPACITY))

        for stat in SCP:
            leaf_fill_col = get_stats_col(ISTC.LEAF_FILL_STATS, stat)
            if leaf_fill_col in leaf_fill_columns:
                leaf_size_col = get_stats_col(ISTC.LEAF_SIZE_STATS, stat)
                istc_leaf_size_col = get_merged_col_name(ERD.INDEX_STATS_COLS, leaf_size_col)
                merged_df[leaf_fill_col] = merged_df[istc_leaf_size_col] / merged_df[isc_leaf_capacity]

        isc_index_name = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEX_FILE))
        columns = leaf_fill_columns + [isc_index_name]
        merged_df = merged_df[columns]
        self.index_stats_df = self.index_stats_df.merge(merged_df, left_on=str(ISC.INDEX_FILE), right_on=isc_index_name)

    @classmethod
    def load_csv_if_exists(cls, path: str, cols: list[str]) -> pd.DataFrame:
        if os.path.exists(path):
            return pd.read_csv(path, usecols=cols)
        return pd.DataFrame()

    @classmethod
    def load(
        cls,
        logs_dir: str,
        cols: dict[ERD, list[str]],
    ):  # -> ExperimentResults:
        original_cols = cols.copy()
        cols = {erd: cols[erd] if erd in cols else [] for erd in ERD}
        act_cols = {erd: list(set(REQUIRED_COLS[erd] + cols[erd])) for erd in ERD}
        extra_cols = {erd: [] for erd in ERD}

        # Handle method name column
        if str(SSC.METHOD_NAME) in cols[ERD.METHODS_COLS]:
            extra_cols[ERD.METHODS_COLS] += [str(col) for col in COLS_FOR_METHOD_NAME]
            act_cols[ERD.METHODS_COLS].remove(str(SSC.METHOD_NAME))

        # Handle pruning ratio column
        if str(QC.PRUNING_RATIO) in cols[ERD.RUNS_COLS]:
            cls.add_extra_cols_for_pruning_ratio(logs_dir, extra_cols)
            act_cols[ERD.RUNS_COLS].remove(str(QC.PRUNING_RATIO))

        # Handle amortized prep time column
        if str(QC.AMORTIZED_PREP_TIME_S) in cols[ERD.RUNS_COLS]:
            cls.add_extra_cols_for_amortized_prep_time(extra_cols)
            act_cols[ERD.RUNS_COLS].remove(str(QC.AMORTIZED_PREP_TIME_S))

        # Handle leaf fill stats columns
        leaf_fill_cols = []
        for stat in SCP:
            leaf_fill_col = get_stats_col(ISTC.LEAF_FILL_STATS, stat)
            if leaf_fill_col in cols[ERD.INDEX_STATS_COLS]:
                leaf_fill_cols.append(leaf_fill_col)
                extra_cols[ERD.INDEX_STATS_COLS].append(get_stats_col(ISTC.LEAF_SIZE_STATS, stat))
                act_cols[ERD.INDEX_STATS_COLS].remove(leaf_fill_col)
        if len(leaf_fill_cols) > 0:
            extra_cols[ERD.INDEXES_COLS].append(str(ISC.LEAF_CAPACITY))

        extra_cols = {erd: list(set(extra_cols[erd]) - set(act_cols[erd])) for erd in ERD}
        cols_to_load = {erd: act_cols[erd] + extra_cols[erd] for erd in ERD}
        csv_paths = {erd: os.path.join(logs_dir, CSV_FILES[erd]) for erd in ERD}
        dfs = {erd: cls.load_csv_if_exists(csv_paths[erd], cols=cols_to_load[erd]) for erd in ERD}

        results = cls(
            logs_dir=logs_dir,
            datasets_df=dfs[ERD.DATASETS_COLS],
            query_sets_df=dfs[ERD.QUERY_SETS_COLS],
            indexes_df=dfs[ERD.INDEXES_COLS],
            methods_df=dfs[ERD.METHODS_COLS],
            runs_df=dfs[ERD.RUNS_COLS],
            index_stats_df=dfs[ERD.INDEX_STATS_COLS],
            query_stats_df=dfs[ERD.QUERY_STATS_COLS],
        )

        # Add method name column
        if str(SSC.METHOD_NAME) in cols[ERD.METHODS_COLS]:
            act_cols[ERD.METHODS_COLS].append(str(SSC.METHOD_NAME))
            results.methods_df = define_method_name_col(results.methods_df)

        # Add pruning ratio column
        if str(QC.PRUNING_RATIO) in cols[ERD.RUNS_COLS]:
            act_cols[ERD.RUNS_COLS].append(str(QC.PRUNING_RATIO))
            results.add_pruning_ratio_column()

        # Add amortized prep time column
        if str(QC.AMORTIZED_PREP_TIME_S) in cols[ERD.RUNS_COLS]:
            act_cols[ERD.RUNS_COLS].append(str(QC.AMORTIZED_PREP_TIME_S))
            results.add_amortized_prep_time_column()

        # Add leaf fill columns
        if len(leaf_fill_cols) > 0:
            act_cols[ERD.INDEX_STATS_COLS] += leaf_fill_cols
            results.add_leaf_fill_columns(leaf_fill_cols)

        # Drop extra columns
        results.datasets_df = results.datasets_df.drop(columns=extra_cols[ERD.DATASETS_COLS])
        results.query_sets_df = results.query_sets_df.drop(columns=extra_cols[ERD.QUERY_SETS_COLS])
        results.indexes_df = results.indexes_df.drop(columns=extra_cols[ERD.INDEXES_COLS])
        results.methods_df = results.methods_df.drop(columns=extra_cols[ERD.METHODS_COLS])
        results.runs_df = results.runs_df.drop(columns=extra_cols[ERD.RUNS_COLS])
        results.index_stats_df = results.index_stats_df.drop(columns=extra_cols[ERD.INDEX_STATS_COLS])
        results.query_stats_df = results.query_stats_df.drop(columns=extra_cols[ERD.QUERY_STATS_COLS])

        cols = original_cols
        return results

    def get_merged_df(self):
        dsc_dataset_file = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.DATASET_FILE))

        columns_to_drop = []
        merged_df = rename_df_columns(self.datasets_df, ERD.DATASETS_COLS)

        if os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.QUERY_SETS_COLS])):
            qsc_dataset_file = get_merged_col_name(ERD.QUERY_SETS_COLS, str(QSC.DATASET_FILE))

            merged_df = merged_df.merge(
                rename_df_columns(self.query_sets_df, ERD.QUERY_SETS_COLS),
                left_on=dsc_dataset_file,
                right_on=qsc_dataset_file,
                how="left",
            )

        if os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.METHODS_COLS])):
            ssc_dataset_file = get_merged_col_name(ERD.METHODS_COLS, str(SSC.DATASET_FILE))

            merged_df = merged_df.merge(
                rename_df_columns(self.methods_df, ERD.METHODS_COLS),
                left_on=dsc_dataset_file,
                right_on=ssc_dataset_file,
                how="left",
            )
            columns_to_drop.append(ssc_dataset_file)

            if os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.INDEXES_COLS])):
                ssc_index_file = get_merged_col_name(ERD.METHODS_COLS, str(SSC.INDEX_FILE))
                isc_index_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEX_FILE))
                isc_dataset_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.DATASET_FILE))

                merged_df = merged_df.merge(
                    rename_df_columns(self.indexes_df, ERD.INDEXES_COLS),
                    left_on=ssc_index_file,
                    right_on=isc_index_file,
                    how="outer",
                )
                columns_to_drop.extend([ssc_index_file, isc_dataset_file])

                if os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.INDEX_STATS_COLS])):
                    isc_index_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEX_FILE))
                    istc_index_file = get_merged_col_name(ERD.INDEX_STATS_COLS, str(ISTC.INDEX_FILE))

                    merged_df = merged_df.merge(
                        rename_df_columns(self.index_stats_df, ERD.INDEX_STATS_COLS),
                        left_on=isc_index_file,
                        right_on=istc_index_file,
                        how="left",
                    )
                    columns_to_drop.append(istc_index_file)

            if os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.RUNS_COLS])):
                qc_settings_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.SETTINGS_ID))
                ssc_id = get_merged_col_name(ERD.METHODS_COLS, str(SSC.ID))

                merged_df = merged_df.merge(
                    rename_df_columns(self.runs_df, ERD.RUNS_COLS),
                    left_on=ssc_id,
                    right_on=qc_settings_id,
                    how="left",
                )
                columns_to_drop.append(ssc_id)

        if os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.QUERY_STATS_COLS])):
            qstc_dataset_file = get_merged_col_name(ERD.QUERY_STATS_COLS, str(QSTC.DATASET_FILE))

            merged_df = merged_df.merge(
                rename_df_columns(self.query_stats_df, ERD.QUERY_STATS_COLS),
                left_on=dsc_dataset_file,
                right_on=qstc_dataset_file,
                how="left",
            )
            columns_to_drop.append(qstc_dataset_file)

        return merged_df.drop(columns=columns_to_drop)


## %%[markdown]
"""
### Reducers
"""


# %%
class Reducer(BaseModel):
    class Config:
        arbitrary_types_allowed = True

    def __call__(self, value: np.ndarray) -> float:
        raise NotImplementedError


class MeanReducer(Reducer):
    def __call__(self, value: np.ndarray) -> float:
        return np.mean(value)


class RobustMeanReducer(Reducer):
    discard_quantile: float = 0.05

    def __call__(self, value: np.ndarray) -> float:
        quantiles = np.quantile(value, [self.discard_quantile, 1 - self.discard_quantile])
        return np.mean(value[(value >= quantiles[0]) & (value <= quantiles[1])])


class StdReducer(Reducer):
    def __call__(self, value: np.ndarray) -> float:
        return np.std(value)


class MinReducer(Reducer):
    def __call__(self, value: np.ndarray) -> float:
        return np.min(value)


class MaxReducer(Reducer):
    def __call__(self, value: np.ndarray) -> float:
        return np.max(value)


# %%[markdown]
"""
### Reduction executor function
"""

# %%

Targets = list[tuple[ERD, str, Reducer]]
Groups = list[tuple[ERD, str]]
ReductionResult = dict[tuple, list[float]]


def execute_reduction(
    experiments: list[ExperimentResults], targets: Targets, groups: Groups, na_replacement: Any = 0
) -> ReductionResult:
    """
    Executes a reduction on the given experiment results. The reduction result is a dictionary mapping the group
    tuples to a list of reduced values for each target.

    :param experiments: The list of experiment results to reduce.
    :param targets: The list of targets to reduce.
    :param groups: The list of groups to reduce.
    :return: The reduction result.
    """

    merged_targets = {get_merged_col_name(target_df, target_col): reducer for target_df, target_col, reducer in targets}
    merged_groups = [get_merged_col_name(group, group_col) for group, group_col in groups]
    reduction_result = {}

    merged_df = pd.concat([experiment.get_merged_df() for experiment in experiments], ignore_index=True)
    merged_df = merged_df.fillna(na_replacement)

    grouped = merged_df.groupby(merged_groups)

    for group_keys, group_df in grouped:
        if not isinstance(group_keys, tuple):
            group_keys = (group_keys,)
        for target, reducer in merged_targets.items():
            reduced_value = reducer(group_df[target])
            if group_keys not in reduction_result:
                reduction_result[group_keys] = []
            reduction_result[group_keys].append(reduced_value)

    return reduction_result


# %%[markdown]
"""
### Bar plot function
"""

# %%

METHOD_COLORS = {
    "sequential_scan-ed": PALETTE["Yellows"][2],
    "base_ed-ed": PALETTE["Yellows"][3],
    "sequential_scan-ed-early": PALETTE["Yellows"][4],
    "sequential_scan-mass": PALETTE["Oranges"][0],
    "base_mass-mass": PALETTE["Oranges"][1],
    "sequential_scan-mass-ffts": PALETTE["Oranges"][2],
    "ulisse_single-ed-early": PALETTE["Greys"][1],
    "ulisse_parallel-ed-early": PALETTE["Greys"][3],
    "isax_envelope-ed-early": PALETTE["Blues"][0],
    "isax_envelope-mass": PALETTE["Blues"][2],
    "isax_envelope-mass-ffts": PALETTE["Blues"][4],
    "isax-ed": PALETTE["Reds"][1],
    "isax-ed-early": PALETTE["Reds"][1],
    "isax-mass": PALETTE["Reds"][2],
    "isax-mass-ffts": PALETTE["Reds"][4],
    "envelope-ed": PALETTE["Purples"][0],
    "envelope-ed-early": PALETTE["Purples"][0],
    "sax_envelope-ed": PALETTE["Purples"][2],
    "sax_envelope-ed-early": PALETTE["Purples"][2],
    "envelope-mass": PALETTE["Purples"][3],
    "envelope-mass-ffts": PALETTE["Purples"][3],
    "sax_envelope-mass": PALETTE["Purples"][6],
    "sax_envelope-mass-ffts": PALETTE["Purples"][6],
    "isax_env_w_env-ed": PALETTE["Greens"][0],
    "isax_env_w_env-ed-early": PALETTE["Greens"][0],
    "isax_env_w_sax_env-ed": PALETTE["Greens"][2],
    "isax_env_w_sax_env-ed-early": PALETTE["Greens"][2],
    "isax_env_w_env-mass": PALETTE["Greens"][3],
    "isax_env_w_env-mass-ffts": PALETTE["Greens"][3],
    "isax_env_w_sax_env-mass": PALETTE["Greens"][6],
    "isax_env_w_sax_env-mass-ffts": PALETTE["Greens"][6],
}
METHOD_LABELS = {
    "sequential_scan-ed": "BF",
    "base_ed-ed": "ED, EAb (C)",
    "sequential_scan-ed-early": "ED, EAb",
    "base_mass-mass": "MASS (C)",
    "sequential_scan-mass": "MASS, no pre.",
    "sequential_scan-mass-ffts": "MASS",
    "ulisse_single-ed-early": "ULISSE single",
    "ulisse_parallel-ed-early": "ULISSE parallel",
    "isax_envelope-ed": "MULISSE (ED)",
    "isax_envelope-ed-early": "MULISSE (ED, EAb)",
    "isax_envelope-mass": "MULISSE (MASS, no pre.)",
    "isax_envelope-mass-ffts": "MULISSE (MASS)",
    "isax-ed": "iSAX (ED)",
    "isax-ed-early": "iSAX (ED, EAb)",
    "isax-mass": "iSAX (MASS, no pre.)",
    "isax-mass-ffts": "iSAX (MASS)",
    "envelope-ed": "Envelope (ED)",
    "envelope-ed-early": "Envelope (ED, EAb)",
    "envelope-mass": "Envelope (MASS, no pre.)",
    "envelope-mass-ffts": "Envelope (MASS)",
    "sax_envelope-ed": "SAX Env (ED)",
    "sax_envelope-ed-early": "SAX Env (ED, EAb)",
    "sax_envelope-mass": "SAX Env (MASS, no pre.)",
    "sax_envelope-mass-ffts": "SAX Env (MASS)",
    "isax_env_w_env-ed": "iSAX+Env (ED)",
    "isax_env_w_env-ed-early": "iSAX+Env (ED, EAb)",
    "isax_env_w_env-mass": "iSAX+Env (MASS, no pre.)",
    "isax_env_w_env-mass-ffts": "iSAX+Env (MASS)",
    "isax_env_w_sax_env-ed": "iSAX+SAX Env (ED)",
    "isax_env_w_sax_env-ed-early": "iSAX+SAX Env (ED, EAb)",
    "isax_env_w_sax_env-mass": "iSAX+SAX Env (MASS, no pre.)",
    "isax_env_w_sax_env-mass-ffts": "iSAX+SAX Env (MASS)",
}
DATASET_ORDER = [
    "weather",
    "stocks",
    "random_walk",
    "synthetic",
]

# %%


def plot_bars(
    reduction_result: ReductionResult,
    color_group_ind: int,
    color_map: dict = METHOD_COLORS,
    label_map: dict = METHOD_LABELS,
    x_labels: dict[tuple, str] = {},
    y_label: str = "",
    y_lim: tuple[float, float] = None,
    scale: str = "linear",
    bar_width_inches: float = 0.4,
    legend_max_cols: int = 4,
    title: str = None,
    hatches: list[str] = None,
    hatch_labels: list[str] = None,
):
    """
    Plot bars for the given reduction result.

    :param reduction_result: The reduction result to plot.
    :param color_group_ind: The index of the group to use for coloring the bars.
    :param color_map: The color map to use for coloring the bars.
    :param label_map: The label map to use for labeling the bars.
    :param x_labels: The labels for the x-axis for each group.
    :param y_label: The label for the y-axis.
    :param y_range: The range to use for the y-axis. If `None`, the range is automatically determined.
    :param scale: The scale to use for the y-axis.
    :param bar_width_inches: The width of the bars in inches.
    :param title: The title of the plot.
    :param hatches: The hatches to use for the bars. If `None`, no hatches are used.
    :param hatch_labels: The labels for the hatches. If `None`, no hatch labels are used.
    """

    bar_groups: dict[tuple, tuple[Any, list[float]]] = {}
    num_bars = 0
    for group, target_values in reduction_result.items():
        bar_group_key = tuple([group[i] for i in range(len(group)) if i != color_group_ind])
        if bar_group_key not in bar_groups:
            bar_groups[bar_group_key] = []
        bar_groups[bar_group_key].append((group[color_group_ind], target_values))
        num_bars += 1

    fig, ax = plt.subplots()
    ax.set_xlim(0.0, 1.0)
    bar_width = 1.0 / (num_bars + len(bar_groups))

    x_start = bar_width / 2
    x_ticks = []
    x_tick_labels = []
    seen_labels = set()
    seen_hatches = set()

    for bar_group_key, bars in bar_groups.items():
        bars_values = [bar[1] for bar in bars]

        colors = []
        for bar in bars:
            color = color_map[bar[0]]
            colors.append(color)
            label = label_map[bar[0]]
            if label not in seen_labels:
                seen_labels.add(label)
                ax.bar(0, 0, color=color, label=label, edgecolor="black")

        for b_ind, bar_values in enumerate(bars_values):
            bar_start = 0
            for v_ind, value in enumerate(bar_values):
                hatch = hatches[v_ind] if hatches is not None and len(hatches) >= v_ind else None
                # fmt: off
                ax.bar(
                    b_ind * bar_width + x_start, height=value, bottom=bar_start, width=bar_width, align="edge",
                    color=colors[b_ind], edgecolor="black", hatch=hatch
                )
                # fmt: on
                bar_start += value

                if hatch is not None and hatch not in seen_hatches:
                    seen_hatches.add(v_ind)

        x_ticks.append(x_start + len(bars) * bar_width / 2)
        x_tick_labels.append(x_labels.get(bar_group_key, ""))
        x_start += (len(bars) + 1) * bar_width

    for h_ind in seen_hatches:
        ax.bar(0, 0, color="white", edgecolor="black", hatch=hatches[h_ind], label=hatch_labels[h_ind])

    def get_legend_num_cols(num_labels):
        return min(num_labels, legend_max_cols)

    def get_legend_y_coord(num_labels):
        legend_num_cols = get_legend_num_cols(num_labels)
        legend_num_rows = (num_labels + legend_num_cols - 1) // legend_num_cols
        return 1.050 + 0.075 * legend_num_rows

    num_labels = len(seen_labels) + len(seen_hatches)
    legend_num_cols = get_legend_num_cols(num_labels)
    legend_y_coord = get_legend_y_coord(num_labels)
    ax.legend(loc="upper center", bbox_to_anchor=(0.5, legend_y_coord), ncol=legend_num_cols)

    ax.set_yscale(scale)
    ax.yaxis.grid(True)
    ax.set_ylabel(y_label)
    ax.set_ylim(y_lim)
    ax.set_xticks(x_ticks)
    ax.set_xticklabels(x_tick_labels)
    ax.set_title(title)

    fig.set_size_inches((num_bars + len(bar_groups)) * bar_width_inches, 6)
    plt.show()


# %%[markdown]
"""
Misc. helpers
"""

# %%
TIME_TARGETS = [str(QC.TOTAL_TIME_S), str(QC.AMORTIZED_PREP_TIME_S)]
TIME_LABELS = ["Search time", "Prep. time"]
PREP_TIME_HATCH = "/////"
TOTAL_TIME_Y_LABEL = "Total time (S)"
PRUNING_RATIO_Y_LABEL = "Pruning ratio"


def simplify_method_name(reduction_result: ReductionResult, method_name_ind: int) -> ReductionResult:
    result = {}
    for group, target_values in reduction_result.items():
        method_name = group[method_name_ind]
        group_list = list(group)

        method_max_len = 0
        method_name = group_list[method_name_ind]
        simple_method_name = ""
        for method in METHOD_LABELS:
            if method in method_name and len(method) > method_max_len:
                method_max_len = len(method)
                simple_method_name = method

        group_list[method_name_ind] = simple_method_name
        result[tuple(group_list)] = target_values
    return result


def sort_dict(d: dict, key_func: callable) -> dict:
    return {k: v for k, v in sorted(d.items(), key=key_func)}


def dict_to_tuples(d: dict[ERD, list[str]]) -> list[tuple[ERD, str]]:
    tuples = []
    for key, values in d.items():
        for value in values:
            tuples.append((key, value))
    return tuples


def get_col_index(col: str, tuples: list[tuple[ERD, str]]) -> int:
    for i, (_, col_name) in enumerate(tuples):
        if col_name == col:
            return i
    return -1


# %%[markdown]
"""
## Running the Analyses

With the helper functions and classes defined above, the analysis of experiment results can commence.
Experimental result analysis generally involves the following steps:
1. Load the experiment results, with only the necessary columns (any columns required for joining the data frames will
   always be included automatically).
2. Define the target columns to reduce, the functions to reduce with and the group columns to reduce by. For example,
   if the average total time is required for each method and dataset combination, one should use the following:
```py
targets = [(ERD.RUNS_COLS, str(QC.TOTAL_TIME_S), MeanReducer())]
groups = [
    (ERD.DATASETS_COLS, str(DSC.DATASET_FILE)),
    (ERD.METHODS_COLS, str(SSC.ID)),
]
```
3. Run the reduction, and do analysis on the reduced values.
"""

# %%[markdown]
"""
### Experiment: Effect of number of channels and dataset
"""


# %%


def experiment_num_channels_and_dataset(
    target_cols: str | list[str], y_label: str, target_labels: list[str] = None, y_scale: str = "log"
):
    if isinstance(target_cols, str):
        target_cols = [target_cols]

    groups_dict = {
        ERD.DATASETS_COLS: [str(DSC.NUM_CHANNELS), str(DSC.DATASET_FILE)],
        ERD.METHODS_COLS: [str(SSC.METHOD_NAME)],
    }
    columns = {**groups_dict, ERD.RUNS_COLS: target_cols}
    few_channels_results = ExperimentResults.load(
        logs_dir="EXPERIMENT_LOGS/num_channels/LOGS_few_channels_config", cols=columns
    )
    many_channels_results = ExperimentResults.load(
        logs_dir="EXPERIMENT_LOGS/num_channels/LOGS_many_channels_config", cols=columns
    )

    targets = [(ERD.RUNS_COLS, target_col, MeanReducer()) for target_col in target_cols]
    groups = dict_to_tuples(groups_dict)
    mean_values = execute_reduction([few_channels_results, many_channels_results], targets, groups)
    mean_values = simplify_method_name(mean_values, 2)

    methods_to_show = [
        "sequential_scan-ed",
        "sequential_scan-mass-ffts",
        "isax_envelope-ed-early",
        "isax_envelope-mass-ffts",
    ]
    mean_values_to_show = {group: values for group, values in mean_values.items() if group[2] in methods_to_show}
    mean_values_to_show = {
        (num_channels, dataset.split("/", 1)[0], method): value
        for (num_channels, dataset, method), value in mean_values_to_show.items()
    }
    x_labels = {
        (num_channels, dataset): f"{dataset}\nC = {num_channels}"
        for (num_channels, dataset, _), _ in mean_values_to_show.items()
    }
    mean_values_to_show = sort_dict(
        mean_values_to_show, lambda x: (DATASET_ORDER.index(x[0][1]), x[0][0], methods_to_show.index(x[0][2]))
    )

    plot_bars(
        mean_values_to_show,
        2,
        x_labels=x_labels,
        y_label=y_label,
        scale=y_scale,
        hatches=["", PREP_TIME_HATCH] if target_labels is not None else None,
        hatch_labels=target_labels,
    )


# %%

experiment_num_channels_and_dataset(TIME_TARGETS, TOTAL_TIME_Y_LABEL, target_labels=TIME_LABELS)
experiment_num_channels_and_dataset(str(QC.PRUNING_RATIO), PRUNING_RATIO_Y_LABEL, y_scale="linear")

# %%[markdown]
"""
### Experiment: Envelope size parametrization
"""


def experiment_envelope_parametrization(
    target_cols: list[str] | str,
    y_label: str,
    y_scale: str = "log",
    target_labels: list[str] = None,
    logs_dir="EXPERIMENT_LOGS/envelope_size/LOGS_envelope_size_param",
):
    if isinstance(target_cols, str):
        target_cols = [target_cols]
    group_dict = {
        ERD.INDEXES_COLS: [str(ISC.L_MIN), str(ISC.L_MAX), str(ISC.POS_PER_ENV)],
        ERD.METHODS_COLS: [str(SSC.METHOD_NAME)],
        ERD.DATASETS_COLS: [str(DSC.DATASET_FILE)],
    }
    columns = {**group_dict, ERD.RUNS_COLS: target_cols}
    parametrization_results = ExperimentResults.load(logs_dir=logs_dir, cols=columns)

    targets = [(ERD.RUNS_COLS, target_col, MeanReducer()) for target_col in target_cols]
    groups = dict_to_tuples(group_dict)
    mean_values = execute_reduction([parametrization_results], targets, groups)
    mean_values = simplify_method_name(mean_values, 3)
    mean_values = {(*group[:4], group[4].rsplit("/")[0]): values for group, values in mean_values.items()}

    def get_x_label(key: tuple):
        l_min, l_max, pos_per_env, _, _ = key
        return f"l_min={l_min}\nl_max={l_max}\nPPE={pos_per_env}"

    datasets = {group[4] for group in mean_values}
    for dataset in datasets:
        mean_values_ds = {group: values for group, values in mean_values.items() if group[4] == dataset}
        mean_values_ds = sort_dict(mean_values_ds, lambda x: (x[0][4], *x[0][:3], x[0][3]))
        x_labels = {(*group[:3], group[4]): get_x_label(group) for group in mean_values_ds}

        plot_bars(
            mean_values_ds,
            3,
            x_labels=x_labels,
            y_label=y_label,
            scale=y_scale,
            title=dataset,
            hatches=["", PREP_TIME_HATCH] if target_labels is not None else None,
            hatch_labels=target_labels,
        )


# %%

for i in range(1, 4):
    print(f"Experiment {i}:")
    exp_logs_dir = "EXPERIMENT_LOGS/envelope_size/LOGS_envelope_size" + (f"_{i}" if i > 1 else "")
    print("Total time:")
    experiment_envelope_parametrization(
        TIME_TARGETS, TOTAL_TIME_Y_LABEL, target_labels=TIME_LABELS, logs_dir=exp_logs_dir
    )
    print("Pruning ratio:")
    experiment_envelope_parametrization(
        str(QC.PRUNING_RATIO), PRUNING_RATIO_Y_LABEL, y_scale="linear", logs_dir=exp_logs_dir
    )

# %%[markdown]
"""
### Experiment: Relative contrast
"""


# %%
NOISE_COLORS = {
    0.1: PALETTE["Pinks"][0],
    0.5: PALETTE["Pinks"][2],
    1.0: PALETTE["Pinks"][4],
}
NOISE_LABELS = {val: f"Noise={val}" for val in NOISE_COLORS.keys()}


def experiment_relative_contrast(
    target_col: str,
    y_label: str,
    query_noise_levels=list(NOISE_LABELS.keys()),
    y_scale: str = "linear",
    target_labels: list[str] = None,
    logs_dir="EXPERIMENT_LOGS/relative_contrast/LOGS_relative_contrast_config",
    remove_top=0.00,
    datasets_to_show=["weather", "synthetic"],
):
    columns = {
        ERD.DATASETS_COLS: [str(DSC.DATASET_FILE), str(DSC.NUM_CHANNELS), str(DSC.SD)],
        ERD.QUERY_STATS_COLS: [str(QSTC.QUERY_NOISE), target_col],
    }
    rc_results = ExperimentResults.load(logs_dir=logs_dir, cols=columns)

    targets = [(ERD.QUERY_STATS_COLS, target_col, MeanReducer())]
    if remove_top > 0:
        rc_results.query_stats_df = rc_results.query_stats_df[
            rc_results.query_stats_df[target_col] < rc_results.query_stats_df[target_col].quantile(1 - remove_top)
        ]

    groups = [
        (ERD.DATASETS_COLS, str(DSC.DATASET_FILE)),
        (ERD.DATASETS_COLS, str(DSC.NUM_CHANNELS)),
        (ERD.DATASETS_COLS, str(DSC.SD)),
        (ERD.QUERY_STATS_COLS, str(QSTC.QUERY_NOISE)),
    ]
    mean_values = execute_reduction([rc_results], targets, groups)
    mean_values = {group: values for group, values in mean_values.items() if group[3] in query_noise_levels}

    mean_values = {
        (dataset.split("/", 1)[0], num_channels, sd, noise): value
        for (dataset, num_channels, sd, noise), value in mean_values.items()
    }
    if datasets_to_show is not None:
        mean_values = {group: values for group, values in mean_values.items() if group[0] in datasets_to_show}

    mean_values = sort_dict(mean_values, lambda x: (x[0][1], DATASET_ORDER.index(x[0][0]), x[0][2]))
    x_labels = {
        (dataset, num_channels, sd): f"{dataset}\nC={num_channels}\nStep={sd}"
        for dataset, num_channels, sd, _ in mean_values
    }

    bar_width_inches = 0.9 / len(query_noise_levels)
    plot_bars(
        mean_values,
        3,
        NOISE_COLORS,
        NOISE_LABELS,
        x_labels,
        y_label=y_label,
        scale=y_scale,
        bar_width_inches=bar_width_inches,
    )


# %%
experiment_relative_contrast(str(QSTC.RC_USING_MAX), "RC using max", query_noise_levels=[0.1, 0.5, 1.0])
experiment_relative_contrast(str(QSTC.RC_USING_MEAN), "RC using mean", query_noise_levels=[0.1, 0.5, 1.0])
experiment_relative_contrast(
    get_stats_col(QSTC.DIST_STATS, SCP.STD), "Std. dev. of distance to query", query_noise_levels=[0.1, 0.5, 1.0]
)
experiment_relative_contrast(
    get_stats_col(QSTC.DIST_STATS, SCP.MAX), "Maximum distance to query", query_noise_levels=[0.1, 0.5, 1.0]
)
experiment_relative_contrast(
    get_stats_col(QSTC.DIST_STATS, SCP.MIN), "Minimum distance to query", query_noise_levels=[0.1, 0.5, 1.0]
)
experiment_relative_contrast(
    get_stats_col(QSTC.DIST_STATS, SCP.MEAN), "Mean distance to query", query_noise_levels=[0.1, 0.5, 1.0]
)

# %%[markdown]
"""
Experiment: Comparison of different methods
"""


# %%
def experiment_compare_methods(
    target_cols: str | list[str],
    y_label,
    y_lim=None,
    y_scale="log",
    target_labels: list[str] = None,
    logs_dirs=["LOGS"],
):
    if isinstance(target_cols, str):
        target_cols = [target_cols]
    group_dict = {
        ERD.DATASETS_COLS: [str(DSC.DATASET_FILE)],
        ERD.QUERY_SETS_COLS: [str(QSC.L_MIN), str(QSC.L_MAX)],
        ERD.METHODS_COLS: [str(SSC.METHOD_NAME)],
        ERD.INDEXES_COLS: [str(ISC.POS_PER_ENV), str(ISC.FIRST_LAYER_NUM_BITS)],
    }
    columns = {**group_dict, ERD.RUNS_COLS: target_cols}
    results_list = [ExperimentResults.load(logs_dir=logs_dir, cols=columns) for logs_dir in logs_dirs]
    targets = [(ERD.RUNS_COLS, target_col, MeanReducer()) for target_col in target_cols]
    groups = dict_to_tuples(group_dict)
    mean_values = execute_reduction(results_list, targets, groups)
    mean_values = simplify_method_name(mean_values, 3)

    def get_label(key: list[str]) -> str:
        dataset, _, pos_per_env, first_layer_num_bits = key
        ppe_str = f"PPE={int(pos_per_env)}" if pos_per_env is not None and pos_per_env > 0 else ""
        bits_str = (
            f"Bits={int(first_layer_num_bits)}" if first_layer_num_bits is not None and first_layer_num_bits > 0 else ""
        )
        return f"{dataset}\n{ppe_str}\n{bits_str}"

    l_ranges = {(key[1], key[2]) for key in mean_values}
    for l_min, l_max in l_ranges:
        mean_values_l_range = {
            (key[0], *key[3:]): val for key, val in mean_values.items() if (key[1], key[2]) == (l_min, l_max)
        }
        mean_values_l_range = {(key[0].split("/", 1)[0], *key[1:]): value for key, value in mean_values_l_range.items()}
        mean_values_l_range = sort_dict(mean_values_l_range, lambda x: (DATASET_ORDER.index(x[0][0]), *x[0][1:]))

        x_labels = {(key[0], *key[2:]): get_label(key) for key in mean_values_l_range}
        plot_bars(
            mean_values_l_range,
            1,
            x_labels=x_labels,
            y_label=y_label,
            y_lim=y_lim,
            scale=y_scale,
            title=f"l_min={l_min}, l_max={l_max}",
            legend_max_cols=3,
            hatches=["", PREP_TIME_HATCH] if target_labels is not None else None,
            hatch_labels=target_labels,
        )


# %%
pure_isax_logs = ["EXPERIMENT_LOGS/pure/LOGS_pure_isax"]
pure_envelope_logs = ["EXPERIMENT_LOGS/pure/LOGS_pure_envelope"]

for logs_dirs in [pure_isax_logs, pure_envelope_logs]:
    # experiment_compare_methods(
    #     TIME_TARGETS, TOTAL_TIME_Y_LABEL, target_labels=TIME_LABELS, logs_dirs=logs_dirs, y_scale="log"
    # )
    experiment_compare_methods(str(QC.TOTAL_TIME_S), TOTAL_TIME_Y_LABEL, logs_dirs=logs_dirs, y_scale="log")
    experiment_compare_methods(
        str(QC.PRUNING_RATIO), PRUNING_RATIO_Y_LABEL, logs_dirs=logs_dirs, y_lim=(0, 1), y_scale="linear"
    )


# %%
def merge_univariate_datasets(mean_values):
    merged_mean_values = {}
    for group, values in mean_values.items():
        dataset = group[0].split("/", 1)[0]
        merged_group = (dataset, *group[1:])
        if merged_group not in merged_mean_values:
            merged_mean_values[merged_group] = [[] for _ in range(len(values))]
        for i, value in enumerate(values):
            merged_mean_values[merged_group][i].append(value)

    for group, values_lists in merged_mean_values.items():
        merged_mean_values[group] = [np.mean(values) for values in values_lists]

    return merged_mean_values


def experiment_univariate_parametrization(
    targets_dict: dict[ERD, list[str]],
    logs_dirs: list[str],
    y_label: str,
    use_adapt_to_dataset: bool = False,
    y_scale: str = "log",
    merge_dataset: bool = True,
    hatches=None,
    hatch_labels=None,
    datasets_to_show=None,
    l_ranges_to_show=None,
    reducer=MeanReducer(),
):
    groups_dict = {
        ERD.DATASETS_COLS: [str(DSC.DATASET_FILE)],
        ERD.QUERY_SETS_COLS: [str(QSC.L_MIN), str(QSC.L_MAX)],
        ERD.METHODS_COLS: [str(SSC.METHOD_NAME)],
        ERD.INDEXES_COLS: [
            str(ISC.FIRST_LAYER_NUM_BITS),
            str(ISC.POS_PER_ENV),
            str(ISC.LEAF_CAPACITY),
        ],
    }
    if use_adapt_to_dataset:
        groups_dict[ERD.INDEXES_COLS].append(str(ISC.ADAPT_TO_DATASET))

    columns = {**groups_dict, **targets_dict}
    results_list = [ExperimentResults.load(logs_dir=logs_dir, cols=columns) for logs_dir in logs_dirs]

    targets = [(csv, target, reducer) for csv, target in dict_to_tuples(targets_dict)]
    groups = dict_to_tuples(groups_dict)
    mean_values = execute_reduction(results_list, targets, groups)
    method_name_ind = get_col_index(str(SSC.METHOD_NAME), groups)
    mean_values = simplify_method_name(mean_values, method_name_ind)

    dataset_order = [key[0] for key in mean_values]
    if merge_dataset:
        mean_values = merge_univariate_datasets(mean_values)
        dataset_order = DATASET_ORDER

    def get_x_label(key: tuple):
        if use_adapt_to_dataset:
            _dataset, _l_min, _l_max, method, first_layer_bits, pos_per_env, leaf_capacity, adapt = key
        else:
            _dataset, _l_min, _l_max, method, first_layer_bits, pos_per_env, leaf_capacity = key
        bits_str = f"Bits={int(first_layer_bits)}" if first_layer_bits is not None and first_layer_bits > 0 else ""
        ppe_str = f"PPE={int(pos_per_env)}" if pos_per_env is not None and pos_per_env > 0 else ""
        leaf_str = f"LC={int(leaf_capacity)}" if leaf_capacity is not None and leaf_capacity > 0 else ""
        adapt_str = "Adapt" if use_adapt_to_dataset and "isax" in method and adapt == 1 else ""
        return "\n".join([s for s in [bits_str, ppe_str, leaf_str, adapt_str] if s])

    datasets = {group[0] for group in mean_values}
    if datasets_to_show is not None:
        datasets = {dataset for dataset in datasets if dataset in datasets_to_show}

    for dataset in datasets:
        l_ranges = {(key[1], key[2]) for key in mean_values if key[0] == dataset}
        if l_ranges_to_show is not None:
            l_ranges = {l_range for l_range in l_ranges if l_range in l_ranges_to_show}

        for l_range in l_ranges:
            mean_values_ds = {
                group: values for group, values in mean_values.items() if group[0] == dataset and group[1:3] == l_range
            }
            mean_values_ds = sort_dict(mean_values_ds, lambda x: (dataset_order.index(x[0][0]), *x[0][1:]))
            x_labels = {(*key[:3], *key[4:]): get_x_label(key) for key in mean_values_ds}

            plot_bars(
                mean_values_ds,
                method_name_ind,
                x_labels=x_labels,
                y_label=y_label,
                scale=y_scale,
                bar_width_inches=0.4,
                title=f"{dataset}: l_min={l_range[0]}, l_max={l_range[1]}",
                hatches=hatches,
                hatch_labels=hatch_labels,
            )


# %%

# logs_dirs = ["EXPERIMENT_LOGS/univariate_param/LOGS_univariate_param_2_old"]
logs_dirs = ["EXPERIMENT_LOGS/univariate_param/LOGS_univariate_param_2"]
# logs_dirs = ["EXPERIMENT_LOGS/univariate_param/LOGS_univariate_param_ppe"]
# logs_dirs = ["EXPERIMENT_LOGS/univariate_param/LOGS_univariate_param_ie_lc"]
# logs_dirs = ["EXPERIMENT_LOGS/adapting/LOGS_adapting_index_2"]

merge_datasets = True
use_adapt_to_dataset = True
show_indexing_time = False
datasets_to_show = ["weather"]
l_ranges_to_show = [(256, 1024)]

hatches = None
hatch_labels = None
targets_dict = {ERD.RUNS_COLS: [str(QC.TOTAL_TIME_S)]}
if show_indexing_time:
    hatches = ["", PREP_TIME_HATCH]
    hatch_labels = TIME_LABELS
    targets_dict = {ERD.RUNS_COLS: [str(QC.TOTAL_TIME_S), str(QC.AMORTIZED_PREP_TIME_S)]}

# %%

reducers = {
    "Mean": MeanReducer(),
    "Std": StdReducer(),
    "Min": MinReducer(),
    "Max": MaxReducer(),
}

for key, reducer in reducers.items():
    experiment_univariate_parametrization(
        targets_dict,
        logs_dirs,
        f"{key} {TOTAL_TIME_Y_LABEL}",
        hatches=hatches,
        hatch_labels=hatch_labels,
        merge_dataset=merge_datasets,
        use_adapt_to_dataset=use_adapt_to_dataset,
        l_ranges_to_show=l_ranges_to_show,
        datasets_to_show=datasets_to_show,
        reducer=reducer,
    )

# %%

for col in [QC.PRUNING_RATIO, QC.NUM_ENTRIES_EXAMINED]:
    experiment_univariate_parametrization(
        {ERD.RUNS_COLS: [str(col)]},
        logs_dirs,
        str(col).replace("_", " ").capitalize(),
        y_scale="linear" if col == QC.PRUNING_RATIO else "log",
        merge_dataset=merge_datasets,
        use_adapt_to_dataset=use_adapt_to_dataset,
        l_ranges_to_show=l_ranges_to_show,
        datasets_to_show=datasets_to_show,
    )

# %%

for istc_col in [ISTC.LEAF_HEIGHT_STATS]:
    y_label_prefix = str(istc_col).replace("_", " ").capitalize()
    for stat in [SCP.MEAN, SCP.STD]:
        experiment_univariate_parametrization(
            {ERD.INDEX_STATS_COLS: [get_stats_col(istc_col, stat)]},
            logs_dirs,
            f"{y_label_prefix} {str(stat)}",
            merge_dataset=merge_datasets,
            use_adapt_to_dataset=use_adapt_to_dataset,
            y_scale="linear",
            l_ranges_to_show=l_ranges_to_show,
            datasets_to_show=datasets_to_show,
        )

# %%


def experiment_ulisse_comparison(
    logs_dir: str = "EXPERIMENT_LOGS/base_compare/LOGS_base_compare_final",
    target_col: QC = QC.TOTAL_TIME_S,
    reducer: Reducer = MeanReducer(),
    max_ulisse_pruning_ratio: float = 1.0,
    only_important: bool = False,
):
    groups_dict = {
        ERD.METHODS_COLS: [str(SSC.METHOD_NAME), str(SSC.SORT_QUERY), str(SSC.USE_PRIORITY_QUEUE)],
        ERD.INDEXES_COLS: [
            str(ISC.BREAKPOINT_STRATEGY),
            str(ISC.SPLIT_STRATEGY),
            str(ISC.NUM_BITS_LIMIT),
        ],
        ERD.RUNS_COLS: [str(QC.PRUNING_RATIO), str(QC.QUERY_ID)],
    }

    targets_dict = {ERD.RUNS_COLS: [str(target_col)]}
    columns = groups_dict.copy()
    columns[ERD.RUNS_COLS] += targets_dict[ERD.RUNS_COLS]

    results = ExperimentResults.load(logs_dir=logs_dir, cols=columns)

    if max_ulisse_pruning_ratio < 1.0:
        ulisse_settings_id = results.methods_df[results.methods_df[str(SSC.METHOD_NAME)].str.startswith("ulisse")]
        ulisse_settings_id = ulisse_settings_id[str(SSC.ID)].values[0]
        queries_to_keep = results.runs_df[
            (results.runs_df[str(QC.SETTINGS_ID)] == ulisse_settings_id)
            & (results.runs_df[str(QC.PRUNING_RATIO)] <= max_ulisse_pruning_ratio)
        ][str(QC.QUERY_ID)].values
        results.runs_df = results.runs_df[results.runs_df[str(QC.QUERY_ID)].isin(queries_to_keep)]
    groups_dict.pop(ERD.RUNS_COLS)

    targets = [(ERD.RUNS_COLS, target_col, reducer) for target_col in targets_dict[ERD.RUNS_COLS]]
    groups = dict_to_tuples(groups_dict)
    reduced_values = execute_reduction([results], targets, groups)
    reduced_values = simplify_method_name(reduced_values, 0)

    if only_important:
        important_run_keys = [
            ("isax_envelope-mass", 0, 0, "equiprobable", "entropy_maximizing", 8),
            ("isax_env_w_env-mass", 0, 1, "equiprobable", "entropy_maximizing", 8),
            ("envelope-mass", 0, 1, 0, 0, 0),
            ("isax_env_w_sax_env-ed-early", 1, 0, "fixed", "ulisse_closest_to_mean", 8),
            ("ulisse_single-ed-early", 1, 0, "fixed", "ulisse_closest_to_mean", 8),
            ("ulisse_parallel-ed-early", 1, 0, "fixed", "ulisse_closest_to_mean", 8),
            ("sequential_scan-ed-early", 0, 0, 0, 0, 0),
            ("sequential_scan-mass", 0, 0, 0, 0, 0),
        ]
        reduced_values = {key: reduced_values[key] for key in important_run_keys}

    method_labels_keys = list(METHOD_LABELS.keys())
    reduced_values = sort_dict(reduced_values, lambda x: method_labels_keys.index(x[0][0]))

    def get_x_label(key: tuple):
        method, sort_query, use_pq, breakpoint_strat, split_strat, num_bits = key

        sort_str = "Sort\n" if sort_query else ""
        pq_str = "PQ\n" if use_pq else ""

        if "isax" not in method:
            return sort_str + pq_str

        breakpoint_str = breakpoint_strat if len(breakpoint_strat) <= 5 else f"{breakpoint_strat[:5]}."
        breakpoint_str = breakpoint_str.capitalize()
        split_str = "".join([s[0].upper() for s in split_strat.split("_")])
        num_bits_str = f"{int(num_bits)} bits" if num_bits > 0 else ""

        return f"{sort_str}{pq_str}{breakpoint_str}\n{split_str}\n{num_bits_str}"

    x_labels = {(*key[1:],): get_x_label(key) for key in reduced_values}
    y_labels = {
        QC.TOTAL_TIME_S: TOTAL_TIME_Y_LABEL,
        QC.ABANDONING_RATE: "Abandoning rate",
        QC.PRUNING_RATIO: PRUNING_RATIO_Y_LABEL,
    }

    plot_bars(
        reduced_values,
        0,
        x_labels=x_labels,
        y_label=y_labels[target_col],
        scale="linear",
    )


# %%

# experiment_ulisse_comparison(max_ulisse_pruning_ratio=0.0)
experiment_ulisse_comparison(
    target_col=QC.TOTAL_TIME_S,
    logs_dir="EXPERIMENT_LOGS/base_compare/LOGS_base_compare_final",
    only_important=True,
)
