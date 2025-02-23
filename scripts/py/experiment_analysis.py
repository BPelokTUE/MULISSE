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
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import QuerySettingsColumn as QSC
from scripts.py.common.columns import QueryStatsColumn as QSTC
from scripts.py.common.style import PALETTE
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
QUERY_STATS_CSV = QSTC.get_csv_name()

REQUIRED_DATASETS_COLS = [str(DSC.DATASET_FILE)]
REQUIRED_INDEXES_COLS = [str(ISC.DATASET_FILE), str(ISC.INDEX_FILE)]
REQUIRED_METHODS_COLS = [str(QSC.DATASET_FILE), str(QSC.INDEX_FILE), str(QSC.ID)]
REQUIRED_RUNS_COLS = [str(QC.SETTINGS_ID)]
REQUIRED_QUERY_STATS_COLS = [str(QSTC.DATASET_FILE)]


class ExperimentResultDataframe(Enum):
    DATASETS_COLS = auto()
    INDEXES_COLS = auto()
    METHODS_COLS = auto()
    RUNS_COLS = auto()
    QUERY_STATS_COLS = auto()

    def __str__(self):
        return self.name.lower()


ERD = ExperimentResultDataframe

MERGED_COL_NAME_SEP = "::"


def get_merged_col_name(df_name: ERD, col_name: str) -> str:
    return f"{str(df_name)}{MERGED_COL_NAME_SEP}{col_name}"


def rename_df_columns(df: pd.DataFrame, df_name: ERD) -> pd.DataFrame:
    return df.rename(columns={col: get_merged_col_name(df_name, col) for col in df.columns})


class ExperimentResults(BaseModel):
    logs_dir: str
    datasets_df: pd.DataFrame
    indexes_df: pd.DataFrame
    methods_df: pd.DataFrame
    runs_df: pd.DataFrame
    query_stats_df: pd.DataFrame

    class Config:
        arbitrary_types_allowed = True

    @classmethod
    def load_csv_if_exists(cls, path: str) -> pd.DataFrame:
        if os.path.exists(path):
            return pd.read_csv(path)
        return pd.DataFrame()

    @classmethod
    def load(
        cls,
        logs_dir: str,
        datasets_cols: list[str] = [],
        indexes_cols: list[str] = [],
        methods_cols: list[str] = [],
        runs_cols: list[str] = [],
        query_stats_cols: list[str] = [],
    ):  # -> ExperimentResults:
        act_datasets_cols = list(set(REQUIRED_DATASETS_COLS + datasets_cols))
        act_indexes_cols = list(set(REQUIRED_INDEXES_COLS + indexes_cols))
        act_methods_cols = list(set(REQUIRED_METHODS_COLS + methods_cols))
        act_runs_cols = list(set(REQUIRED_RUNS_COLS + runs_cols))
        act_query_stats_cols = list(set(REQUIRED_QUERY_STATS_COLS + query_stats_cols))

        extra_datasets_cols = []
        extra_indexes_cols = []
        extra_methods_cols = []
        extra_runs_cols = []
        extra_query_stats_cols = []

        # Handle method name column
        if str(QSC.METHOD_NAME) in methods_cols:
            extra_methods_cols += [str(col) for col in COLS_FOR_METHOD_NAME]
            act_methods_cols.remove(str(QSC.METHOD_NAME))

        # Handle pruning ratio column
        if str(QC.PRUNING_RATIO) in runs_cols:
            extra_datasets_cols += [str(DSC.NUM_SERIES), str(DSC.SERIES_LENGTH)]
            extra_indexes_cols += [str(ISC.L_MIN), str(ISC.POS_PER_ENV)]
            extra_methods_cols += [str(QSC.SEARCH_METHOD)]
            extra_runs_cols += [str(QC.NUM_TS_EXAMINED), str(QC.ID)]
            act_runs_cols.remove(str(QC.PRUNING_RATIO))

        extra_datasets_cols = list(set(extra_datasets_cols) - set(act_datasets_cols))
        extra_indexes_cols = list(set(extra_indexes_cols) - set(act_indexes_cols))
        extra_methods_cols = list(set(extra_methods_cols) - set(act_methods_cols))
        extra_runs_cols = list(set(extra_runs_cols) - set(act_runs_cols))
        extra_query_stats_cols = list(set(extra_query_stats_cols) - set(act_query_stats_cols))

        results = cls(
            logs_dir=logs_dir,
            datasets_df=cls.load_csv_if_exists(os.path.join(logs_dir, DATASETS_CSV)),
            indexes_df=cls.load_csv_if_exists(os.path.join(logs_dir, INDEXES_CSV)),
            methods_df=cls.load_csv_if_exists(os.path.join(logs_dir, METHODS_CSV)),
            runs_df=cls.load_csv_if_exists(os.path.join(logs_dir, RUNS_CSV)),
            query_stats_df=cls.load_csv_if_exists(os.path.join(logs_dir, QUERY_STATS_CSV)),
        )

        # Add method name column
        if str(QSC.METHOD_NAME) in methods_cols:
            act_methods_cols.append(str(QSC.METHOD_NAME))
            results.methods_df = define_method_name_col(results.methods_df)

        # Add pruning ratio column
        if str(QC.PRUNING_RATIO) in runs_cols:
            merged_df = results.get_merged_df()

            dsc_num_series = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.NUM_SERIES))
            dsc_series_length = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.SERIES_LENGTH))
            isc_l_min = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.L_MIN))
            isc_pos_per_env = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.POS_PER_ENV))
            qc_num_ts_examined = get_merged_col_name(ERD.RUNS_COLS, str(QC.NUM_TS_EXAMINED))
            qc_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.ID))
            qsc_search_method = get_merged_col_name(ERD.METHODS_COLS, str(QSC.SEARCH_METHOD))

            # Handle the fact that iSAX counts one series for each envelope examined
            merged_df["num_series_multiplier"] = np.where(
                merged_df[qsc_search_method].str.contains("isax"),
                (merged_df[dsc_series_length] - merged_df[isc_l_min] + merged_df[isc_pos_per_env])
                // merged_df[isc_pos_per_env],
                1.0,
            )
            merged_df[str(QC.PRUNING_RATIO)] = 1.0 - merged_df[qc_num_ts_examined] / (
                merged_df[dsc_num_series] * merged_df["num_series_multiplier"]
            )
            merged_df = merged_df[[str(QC.PRUNING_RATIO), qc_id]]
            results.runs_df = results.runs_df.merge(merged_df, left_on=str(QC.ID), right_on=qc_id, how="left")

        # Drop extra columns
        results.datasets_df = results.datasets_df.drop(columns=extra_datasets_cols)
        results.indexes_df = results.indexes_df.drop(columns=extra_indexes_cols)
        results.methods_df = results.methods_df.drop(columns=extra_methods_cols)
        results.runs_df = results.runs_df.drop(columns=extra_runs_cols)

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

        columns_to_drop = []
        merged_df = rename_df_columns(self.datasets_df, ERD.DATASETS_COLS)

        if os.path.exists(os.path.join(self.logs_dir, INDEXES_CSV)):
            merged_df = merged_df.merge(
                rename_df_columns(self.indexes_df, ERD.INDEXES_COLS),
                left_on=dsc_dataset_file,
                right_on=isc_dataset_file,
                how="left",
            )
            columns_to_drop.append(isc_dataset_file)
        if os.path.exists(os.path.join(self.logs_dir, METHODS_CSV)):
            merged_df = merged_df.merge(
                rename_df_columns(self.methods_df, ERD.METHODS_COLS),
                left_on=[dsc_dataset_file, isc_index_file],
                right_on=[qsc_dataset_file, qsc_index_file],
                how="left",
            )
            columns_to_drop.extend([qsc_dataset_file, qsc_index_file])
            if os.path.exists(os.path.join(self.logs_dir, RUNS_CSV)):
                merged_df = merged_df.merge(
                    rename_df_columns(self.runs_df, ERD.RUNS_COLS),
                    left_on=qc_id,
                    right_on=qc_settings_id,
                    how="left",
                )
                columns_to_drop.append(qc_id)
        if os.path.exists(os.path.join(self.logs_dir, QUERY_STATS_CSV)):
            qstc_dataset_file = get_merged_col_name(ERD.QUERY_STATS_COLS, str(QSTC.DATASET_FILE))
            merged_df = merged_df.merge(
                rename_df_columns(self.query_stats_df, ERD.QUERY_STATS_COLS),
                left_on=dsc_dataset_file,
                right_on=qstc_dataset_file,
                how="left",
            )
            columns_to_drop.append(qstc_dataset_file)

        return merged_df.drop(columns=columns_to_drop)


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
ReductionResult = list[tuple[list, Any]]


def execute_reduction(
    experiments: list[ExperimentResults], targets: Targets, groups: Groups, na_replacement: Any = 0
) -> dict[str, ReductionResult]:
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
    reduction_result = {target: [] for target in merged_targets}

    for experiment in experiments:
        merged_df = experiment.get_merged_df()
        merged_df = merged_df.fillna(na_replacement)
        grouped = merged_df.groupby(merged_groups)

        for group_keys, group_df in grouped:
            if not isinstance(group_keys, tuple):
                group_keys = (group_keys,)
            for target, reducer in merged_targets.items():
                reduced_value = reducer(group_df[target])
                reduction_result[target].append((list(group_keys), reduced_value))

    return reduction_result


# %%[markdown]
"""
### Bar plot function
"""

# %%


def plot_bars(
    reduction_result: ReductionResult,
    color_group_ind: int,
    color_map: dict,
    label_map: dict,
    x_labels: dict[tuple, str],
    y_label: str,
    scale: str = "linear",
    bar_width_inches: float = 0.4,
):
    """
    Plot bars for the given reduction result.

    :param reduction_result: The reduction result to plot.
    :param color_group_ind: The index of the group to use for coloring the bars.
    :param color_map: The color map to use for coloring the bars.
    :param label_map: The label map to use for labeling the bars.
    :param x_labels: The labels for the x-axis for each group.
    :param y_label: The label for the y-axis.
    :param scale: The scale to use for the y-axis.
    :param bar_width_inches: The width of the bars in inches.
    """

    bar_groups = {}
    num_bars = 0
    for group, target in reduction_result:
        bar_group_key = tuple([group[i] for i in range(len(group)) if i != color_group_ind])
        if bar_group_key not in bar_groups:
            bar_groups[bar_group_key] = []
        bar_groups[bar_group_key].append((group[color_group_ind], target))
        num_bars += 1

    fig, ax = plt.subplots()
    ax.set_xlim(0.0, 1.0)
    bar_width = 1.0 / (num_bars + len(bar_groups))

    x_start = bar_width / 2
    x_ticks = []
    x_tick_labels = []
    for i, (bar_group_key, bars) in enumerate(bar_groups.items()):
        values = [bar[1] for bar in bars]
        colors = [color_map[bar[0]] for bar in bars]
        x = np.arange(0, len(bars)) * bar_width + x_start

        labels = None
        if i == 0:
            labels = [label_map[bar[0]] for bar in bars]
        ax.bar(x, values, bar_width, align="edge", color=colors, edgecolor="black", label=labels)

        x_ticks.append(x_start + len(bars) * bar_width / 2)
        x_tick_labels.append(x_labels[bar_group_key])
        x_start += (len(bars) + 1) * bar_width

    ax.legend(loc="upper center", bbox_to_anchor=(0.5, 1.125), ncol=len(label_map))
    ax.set_yscale(scale)
    ax.yaxis.grid(True)
    ax.set_ylabel(y_label)
    ax.set_xticks(x_ticks)
    ax.set_xticklabels(x_tick_labels)

    fig.set_size_inches((num_bars + len(bar_groups)) * bar_width_inches, 6)
    plt.show()


# %%[markdown]
"""
Misc. helpers
"""

# %%
METHOD_COLORS = {
    "sequential_scan-ed": PALETTE["Greens"][1],
    "sequential_scan-mass-ffts": PALETTE["Oranges"][2],
    "isax_envelope-ed-early": PALETTE["Blues"][4],
    "isax_envelope-mass": PALETTE["Blues"][2],
    "isax_envelope-mass-ffts": PALETTE["Blues"][1],
}
METHOD_LABELS = {
    "sequential_scan-ed": "BF",
    "sequential_scan-mass-ffts": "MASS",
    "isax_envelope-ed-early": "MULISSE (ED)",
    "isax_envelope-mass": "MULISSE (MASS - no pre.)",
    "isax_envelope-mass-ffts": "MULISSE (MASS)",
}


def remove_index_name(method_name: str, index_prefix: str = "index") -> str:
    return method_name.rsplit(f"-{index_prefix}", 1)[0]


def remove_index_name_from_reduction_result(
    reduction_result: ReductionResult, method_name_ind: int, index_prefix: str = "index"
) -> ReductionResult:
    result = []
    for group, target in reduction_result:
        method_name = group[method_name_ind]
        group[method_name_ind] = remove_index_name(method_name, index_prefix)
        result.append((group, target))
    return result


def calculate_pruning_ratio():
    pass


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
def experiment_num_channels_and_dataset(target_col: str, y_label: str, y_scale: str = "log"):
    columns = {
        str(ERD.DATASETS_COLS): [str(DSC.NUM_CHANNELS), str(DSC.DATASET_FILE)],
        str(ERD.METHODS_COLS): [str(QSC.METHOD_NAME)],
        str(ERD.RUNS_COLS): [target_col],
    }
    few_channels_results = ExperimentResults.load(logs_dir="EXPERIMENT_LOGS/LOGS_few_channels_config", **columns)
    many_channels_results = ExperimentResults.load(logs_dir="EXPERIMENT_LOGS/LOGS_many_channels_config", **columns)

    targets = [(ERD.RUNS_COLS, target_col, MeanReducer())]
    groups = [
        (ERD.DATASETS_COLS, str(DSC.NUM_CHANNELS)),
        (ERD.DATASETS_COLS, str(DSC.DATASET_FILE)),
        (ERD.METHODS_COLS, str(QSC.METHOD_NAME)),
    ]
    reduction_result = execute_reduction([few_channels_results, many_channels_results], targets, groups)
    mean_values = reduction_result[get_merged_col_name(ERD.RUNS_COLS, target_col)]
    mean_values = remove_index_name_from_reduction_result(mean_values, 2)

    methods_to_show = [
        "sequential_scan-ed",
        "sequential_scan-mass-ffts",
        "isax_envelope-ed-early",
        "isax_envelope-mass-ffts",
    ]
    dataset_order = [
        "weather",
        "stocks",
        "random_walk",
        "synthetic",
    ]

    mean_values_to_show = [entry for entry in mean_values if entry[0][2] in methods_to_show]
    mean_values_to_show = [
        ([num_channels, dataset.split("/", 1)[0], method], value)
        for (num_channels, dataset, method), value in mean_values_to_show
    ]
    x_labels = {
        (num_channels, dataset): f"{dataset}\nC = {num_channels}"
        for (num_channels, dataset, _), _ in mean_values_to_show
    }
    mean_values_to_show.sort(key=lambda x: (dataset_order.index(x[0][1]), x[0][0], methods_to_show.index(x[0][2])))

    plot_bars(mean_values_to_show, 2, METHOD_COLORS, METHOD_LABELS, x_labels, y_label=y_label, scale=y_scale)


# %%

experiment_num_channels_and_dataset(str(QC.TOTAL_TIME_S), "Total time (S)")
experiment_num_channels_and_dataset(str(QC.NUM_TS_EXAMINED), "Number of TS examined")
experiment_num_channels_and_dataset(str(QC.PRUNING_RATIO), "Pruning ratio", y_scale="linear")

# %%[markdown]
"""
### Experiment: Envelope size parametrization
"""


def experiment_envelope_parametrization(
    target_col: str, y_label: str, y_scale: str = "log", logs_dir="EXPERIMENT_LOGS/LOGS_envelope_size_parametrization"
):
    columns = {
        str(ERD.INDEXES_COLS): [str(ISC.L_MIN), str(ISC.L_MAX), str(ISC.POS_PER_ENV)],
        str(ERD.METHODS_COLS): [str(QSC.METHOD_NAME)],
        str(ERD.RUNS_COLS): [target_col],
    }
    parametrization_results = ExperimentResults.load(logs_dir=logs_dir, **columns)

    targets = [(ERD.RUNS_COLS, target_col, MeanReducer())]
    groups = [
        (ERD.INDEXES_COLS, str(ISC.L_MIN)),
        (ERD.INDEXES_COLS, str(ISC.L_MAX)),
        (ERD.INDEXES_COLS, str(ISC.POS_PER_ENV)),
        (ERD.METHODS_COLS, str(QSC.METHOD_NAME)),
    ]
    reduction_result = execute_reduction([parametrization_results], targets, groups)
    mean_values = reduction_result[get_merged_col_name(ERD.RUNS_COLS, target_col)]
    mean_values = remove_index_name_from_reduction_result(mean_values, 3)

    mean_values.sort(key=lambda x: (x[0][0], x[0][2]))
    x_labels = {
        (l_min, l_max, pos_per_env): f"l_min={l_min}\nl_max={l_max}\nPPE={pos_per_env}"
        for (l_min, l_max, pos_per_env, _), _ in mean_values
    }

    plot_bars(mean_values, 3, METHOD_COLORS, METHOD_LABELS, x_labels, y_label=y_label, scale=y_scale)


# %%

print("Experiment 1:")
experiment_envelope_parametrization(str(QC.TOTAL_TIME_S), "Total time (S)")
experiment_envelope_parametrization(str(QC.PRUNING_RATIO), "Pruning ratio", y_scale="linear")

print("Experiment 2:")
exp_2_logs_dir = "EXPERIMENT_LOGS/LOGS_envelope_size_parametrization_2"
experiment_envelope_parametrization(str(QC.TOTAL_TIME_S), "Total time (S)", logs_dir=exp_2_logs_dir)
experiment_envelope_parametrization(str(QC.PRUNING_RATIO), "Pruning ratio", y_scale="linear", logs_dir=exp_2_logs_dir)

# %%[markdown]
"""
### Experiment: Relative contrast
"""


# %%
NOISE_COLORS = {
    0.1: PALETTE["Purples"][1],
    0.5: PALETTE["Purples"][4],
    1.0: PALETTE["Purples"][6],
}
NOISE_LABELS = {val: f"Noise={val}" for val in NOISE_COLORS.keys()}


def experiment_relative_contrast(
    target_col: str, y_label: str, query_noise_levels=list(NOISE_LABELS.keys()), y_scale: str = "linear"
):
    columns = {
        str(ERD.DATASETS_COLS): [str(DSC.DATASET_FILE), str(DSC.NUM_CHANNELS), str(DSC.SD)],
        str(ERD.QUERY_STATS_COLS): [str(QSTC.QUERY_NOISE), target_col],
    }
    rc_results = ExperimentResults.load(logs_dir="LOGS_rc", **columns)

    targets = [(ERD.QUERY_STATS_COLS, target_col, MeanReducer())]
    groups = [
        (ERD.DATASETS_COLS, str(DSC.DATASET_FILE)),
        (ERD.DATASETS_COLS, str(DSC.NUM_CHANNELS)),
        (ERD.DATASETS_COLS, str(DSC.SD)),
        (ERD.QUERY_STATS_COLS, str(QSTC.QUERY_NOISE)),
    ]
    reduction_result = execute_reduction([rc_results], targets, groups)
    mean_values = reduction_result[get_merged_col_name(ERD.QUERY_STATS_COLS, target_col)]
    mean_values = [entry for entry in mean_values if entry[0][3] in query_noise_levels]

    mean_values = [
        ([dataset.split("/", 1)[0], num_channels, sd, noise], value)
        for (dataset, num_channels, sd, noise), value in mean_values
    ]
    dataset_order = [
        "weather",
        "stocks",
        "synthetic",
    ]
    mean_values.sort(key=lambda x: (x[0][1], dataset_order.index(x[0][0]), x[0][2]))
    x_labels = {
        (dataset, num_channels, sd): f"{dataset}\nC={num_channels}\nStep={sd}"
        for (dataset, num_channels, sd, _), _ in mean_values
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
    str(QSTC.DIST_STD_DEV), "Std. dev. of distance to query", query_noise_levels=[0.1, 0.5, 1.0]
)
experiment_relative_contrast(str(QSTC.MAX_DIST), "Maximum distance to query", query_noise_levels=[0.1, 0.5, 1.0])
experiment_relative_contrast(str(QSTC.MIN_DIST), "Minimum distance to query", query_noise_levels=[0.1, 0.5, 1.0])
experiment_relative_contrast(str(QSTC.MEAN_DIST), "Mean distance to query", query_noise_levels=[0.1, 0.5, 1.0])
