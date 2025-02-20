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

import numpy as np
import pandas as pd
from pydantic import BaseModel

if os.getcwd().endswith("scripts/py"):
    os.chdir("../..")

from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import IndexSettingsColumn as ISC
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import QuerySettingsColumn as QSC
from scripts.py.common.utils import COLS_FOR_METHOD_NAME, define_method_name_col

# %%[markdown]
"""
### Experiment Results
"""

# %%
DATASETS_CSV = DSC.get_csv_name()
INDEXES_CSV = ISC.get_csv_name()
METHODS_CSV = QSC.get_csv_name()
RUNS_CSV = QC.get_csv_name()

REQUIRED_DATASETS_COLS = [str(DSC.DATASET_FILE)]
REQUIRED_INDEXES_COLS = [str(ISC.DATASET_FILE), str(ISC.INDEX_FILE)]
REQUIRED_METHODS_COLS = [str(QSC.DATASET_FILE), str(QSC.INDEX_FILE), str(QSC.ID)]
REQUIRED_RUNS_COLS = [str(QC.SETTINGS_ID)]


class ExperimentResultDataframe(Enum):
    DATASETS_COLS = auto()
    INDEXES_COLS = auto()
    METHODS_COLS = auto()
    RUNS_COLS = auto()

    def __str__(self):
        return self.name.lower()


ERD = ExperimentResultDataframe

MERGED_COL_NAME_SEP = "::"


def get_merged_col_name(df_name: ERD, col_name: str) -> str:
    return f"{str(df_name)}{MERGED_COL_NAME_SEP}{col_name}"


def rename_df_columns(df: pd.DataFrame, df_name: ERD) -> pd.DataFrame:
    return df.rename(columns={col: get_merged_col_name(df_name, col) for col in df.columns})


class ExperimentResults(BaseModel):
    datasets_df: pd.DataFrame
    indexes_df: pd.DataFrame
    methods_df: pd.DataFrame
    runs_df: pd.DataFrame

    class Config:
        arbitrary_types_allowed = True

    @classmethod
    def load(
        cls,
        logs_dir: str,
        datasets_cols: list[str] = [],
        indexes_cols: list[str] = [],
        methods_cols: list[str] = [],
        runs_cols: list[str] = [],
    ):  # -> ExperimentResults:
        act_datasets_cols = list(set(REQUIRED_DATASETS_COLS + datasets_cols))
        act_indexes_cols = list(set(REQUIRED_INDEXES_COLS + indexes_cols))
        act_methods_cols = list(set(REQUIRED_METHODS_COLS + methods_cols))
        act_runs_cols = list(set(REQUIRED_RUNS_COLS + runs_cols))

        # Handle method name column
        cols_for_method_name = [str(col) for col in COLS_FOR_METHOD_NAME]
        if str(QSC.METHOD_NAME) in methods_cols:
            act_methods_cols.remove(str(QSC.METHOD_NAME))

        results = cls(
            datasets_df=pd.read_csv(os.path.join(logs_dir, DATASETS_CSV), usecols=act_datasets_cols),
            indexes_df=pd.read_csv(os.path.join(logs_dir, INDEXES_CSV), usecols=act_indexes_cols),
            methods_df=pd.read_csv(
                os.path.join(logs_dir, METHODS_CSV), usecols=act_methods_cols + cols_for_method_name
            ),
            runs_df=pd.read_csv(os.path.join(logs_dir, RUNS_CSV), usecols=act_runs_cols),
        )

        # Add method name column
        if str(QSC.METHOD_NAME) in methods_cols:
            act_methods_cols.append(str(QSC.METHOD_NAME))
            results.methods_df = define_method_name_col(results.methods_df, act_methods_cols)

        method_cols_to_drop = [col for col in results.methods_df.columns if col not in act_methods_cols]
        results.methods_df = results.methods_df.drop(columns=method_cols_to_drop)

        return results

    def get_merged_df(self):
        dsc_dataset_file = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.DATASET_FILE))
        isc_dataset_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.DATASET_FILE))
        isc_index_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEX_FILE))
        qsc_dataset_file = get_merged_col_name(ERD.METHODS_COLS, str(QSC.DATASET_FILE))
        qsc_index_file = get_merged_col_name(ERD.METHODS_COLS, str(QSC.INDEX_FILE))
        qc_settings_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.SETTINGS_ID))
        qc_id = get_merged_col_name(ERD.METHODS_COLS, str(QSC.ID))

        merged_df = rename_df_columns(self.datasets_df, ERD.DATASETS_COLS).merge(
            rename_df_columns(self.indexes_df, ERD.INDEXES_COLS),
            left_on=dsc_dataset_file,
            right_on=isc_dataset_file,
            how="left",
        )
        merged_df = merged_df.merge(
            rename_df_columns(self.methods_df, ERD.METHODS_COLS),
            left_on=[dsc_dataset_file, isc_index_file],
            right_on=[qsc_dataset_file, qsc_index_file],
            how="left",
        )
        merged_df = merged_df.merge(
            rename_df_columns(self.runs_df, ERD.RUNS_COLS),
            left_on=qc_id,
            right_on=qc_settings_id,
            how="left",
        )

        return merged_df.drop(columns=[isc_dataset_file, qsc_dataset_file, qsc_index_file, qc_settings_id])


# %%[markdown]
"""
### Reducers
"""


# %%
class Reducer(BaseModel):
    class Config:
        arbitrary_types_allowed = True

    def __call__(self, value: Any):
        raise NotImplementedError


class MeanReducer(Reducer):
    def __call__(self, value: Any):
        return np.mean(value)


# %%[markdown]
"""
### Reduction executor function
"""

# %%

Targets = list[tuple[ERD, str, Reducer]]
Groups = list[tuple[ERD, str]]


def execute_reduction(
    experiments: list[ExperimentResults], targets: Targets, groups: Groups
) -> dict[str, tuple[list[list], list]]:
    """
    Executes a reduction on the given experiment results. The reduction result is a dictionary mapping each
    target to a tuple of two lists. The first list contains the list of values of the group columns, and the second
    contains the reduced values of the target column.

    :param experiments: The list of experiment results to reduce.
    :param targets: The list of targets to reduce.
    :param groups: The list of groups to reduce.
    :return: The reduction result.
    """

    merged_targets = {get_merged_col_name(target_df, target_col): reducer for target_df, target_col, reducer in targets}
    merged_groups = [get_merged_col_name(group, group_col) for group, group_col in groups]
    reduction_result = {target: ([], []) for target in merged_targets}

    for experiment in experiments:
        merged_df = experiment.get_merged_df()
        grouped = merged_df.groupby(merged_groups)

        for group_keys, group_df in grouped:
            if not isinstance(group_keys, tuple):
                group_keys = (group_keys,)
            for target, reducer in merged_targets.items():
                reduced_value = reducer(group_df[target].dropna())
                reduction_result[target][0].append(list(group_keys))
                reduction_result[target][1].append(reduced_value)

    return reduction_result


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
    (ERD.METHODS_COLS, str(QSC.ID)),
]
```
3. Run the reduction, and do analysis on the reduced values.
"""

# %%[markdown]
"""
### Experiment: Effect of number of channels and dataset
"""

# %%
columns = {
    str(ERD.DATASETS_COLS): [str(DSC.NUM_CHANNELS), str(DSC.DATASET_FILE)],
    str(ERD.METHODS_COLS): [str(QSC.METHOD_NAME)],
    str(ERD.RUNS_COLS): [str(QC.TOTAL_TIME_S)],
}
few_channels_results = ExperimentResults.load(logs_dir="LOGS_few", **columns)
many_channels_results = ExperimentResults.load(logs_dir="LOGS_many", **columns)

# %%
targets = [(ERD.RUNS_COLS, str(QC.TOTAL_TIME_S), MeanReducer())]
groups = [
    (ERD.DATASETS_COLS, str(DSC.NUM_CHANNELS)),
    (ERD.DATASETS_COLS, str(DSC.DATASET_FILE)),
    (ERD.METHODS_COLS, str(QSC.METHOD_NAME)),
]
mean_times = execute_reduction([few_channels_results, many_channels_results], targets, groups)

# %%

for target, (group_values, reduced_values) in mean_times.items():
    print(f"Target: {target}")
    for group_value, reduced_value in zip(group_values, reduced_values):
        print(f"Group: {group_value}\nReduced value: {reduced_value}")

# %%
