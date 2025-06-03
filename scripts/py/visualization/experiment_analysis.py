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

import copy
import os
import re
from enum import Enum

if True:
    while not os.getcwd().endswith("MULISSE"):
        os.chdir("..")

from scripts.py.common.columns import Column, StatsColumn
from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import DatasetStatsColumn as DSTC
from scripts.py.common.columns import IndexSettingsColumn as ISC
from scripts.py.common.columns import IndexStatsColumn as ISTC
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import QuerySetSettingsColumn as QSC
from scripts.py.common.columns import QueryStatsColumn as QSTC
from scripts.py.common.columns import SearchSettingsColumn as SSC
from scripts.py.common.columns import StatsColumnPrefix as SCP
from scripts.py.visualization.helpers import (
    dict_to_tuples,
    get_col_index,
    iterate_columns,
    merge_univariate_datasets,
    sort_dict,
)
from scripts.py.visualization.plots import (
    FIRST_LAYER_TIME_HATCH,
    METHOD_COLORS,
    METHOD_LABELS,
    ORDERED_DATASETS,
    PQ_TIME_LABELS,
    PQ_TIME_TARGETS,
    PREP_TIME_HATCH,
    TIME_LABELS,
    TIME_TARGETS,
    get_config_label,
    get_config_labels,
    get_y_label,
    plot_bars,
    plot_heat_map,
    plot_lines,
)
from scripts.py.visualization.reduction import ERD, ExperimentResults, MeanReducer, Reducer, execute_reduction
from scripts.py.visualization.style import COLD_TO_HOT_COLORS, PALETTE

# %%[markdown]
"""
## Experiment functions

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

# %%


class TargetArgs(Enum):
    QUERY_TIME = {"targets_dict": {ERD.RUNS_COLS: [QC.TOTAL_TIME_S]}}
    COMBINED_TIME = {
        "targets_dict": {ERD.RUNS_COLS: TIME_TARGETS},
        "hatches": ["", PREP_TIME_HATCH],
        "hatch_labels": TIME_LABELS,
    }
    MINDIST_TIME = {
        "targets_dict": {ERD.RUNS_COLS: PQ_TIME_TARGETS},
        "hatches": ["", FIRST_LAYER_TIME_HATCH],
        "hatch_labels": PQ_TIME_LABELS,
    }
    INDEX_TIME = {"targets_dict": {ERD.RUNS_COLS: [QC.AMORTIZED_PREP_TIME_S]}}
    INDEX_SIZE = {"targets_dict": {ERD.INDEXES_COLS: [ISC.SIZE_ON_DISK_B]}}
    INDEX_INFO = {"targets_dict": {ERD.INDEXES_COLS: [ISC.SIZE_ON_DISK_B], ERD.RUNS_COLS: [QC.AMORTIZED_PREP_TIME_S]}}
    PRUNING_RATIO = {"targets_dict": {ERD.RUNS_COLS: [QC.PRUNING_RATIO]}}
    ABANDONING_RATE = {"targets_dict": {ERD.RUNS_COLS: [QC.ABANDONING_RATE]}}


def visualize_experiments(
    # Logs directory
    logs_dirs: list[str],
    # Reduction
    groups_dict: dict[ERD, list[Column]],
    targets_dict: dict[ERD, list[Column]] = {ERD.RUNS_COLS: [QC.TOTAL_TIME_S]},
    reducer: Reducer = MeanReducer(),
    num_query_intervals: int = 1,
    # Separating plots
    merge_csv_datasets: bool = False,
    separate_plots_dict: dict[tuple, set] = {},
    # Filtering
    regex_dict: dict[Column, str] = {},
    # Plotting
    x_scale: str = "linear",
    y_scale: str = "linear",
    title_base: str = "",
    # Bar plots
    hatches=None,
    hatch_labels=None,
    bar_plot_label_padding: bool = True,
    bar_width_inches: float = 0.4,
    bar_plot_color_attr: Column | None = SSC.METHOD_NAME,
    bar_plot_color_map: dict[str, str] = METHOD_COLORS,
    # Line plots
    line_plot_x_attr: Column | None = None,
    line_plot_included_cols: set[Column] | None = None,
    # Heat maps
    heat_map_x_attr: Column | None = None,
    heat_map_y_attr: Column | None = None,
    heat_map_included_cols: set[Column] | None = None,
):
    if num_query_intervals > 1:
        groups_dict[ERD.RUNS_COLS] = [QC.QUERY_INTERVAL]
    columns = groups_dict.copy()
    for erd, target_cols in targets_dict.items():
        columns[erd] = target_cols + groups_dict.get(erd, [])

    add_runs = any([len(d.get(ERD.RUNS_COLS, [])) > 0 for d in [groups_dict, targets_dict]])
    add_dataset_stats = any([len(d.get(ERD.DATASET_STATS_COLS, [])) > 0 for d in [groups_dict, targets_dict]])
    add_query_stats = any([len(d.get(ERD.QUERY_STATS_COLS, [])) > 0 for d in [groups_dict, targets_dict]])
    add_index_stats = any([len(d.get(ERD.INDEX_STATS_COLS, [])) > 0 for d in [groups_dict, targets_dict]])

    results_list = [
        ExperimentResults.load(
            logs_dir=logs_dir,
            cols=columns,
            num_query_intervals=num_query_intervals,
            add_runs=add_runs,
            add_dataset_stats=add_dataset_stats,
            add_query_stats=add_query_stats,
            add_index_stats=add_index_stats,
        )
        for logs_dir in logs_dirs
    ]

    targets = [(erd, target_col, reducer) for erd, target_cols in targets_dict.items() for target_col in target_cols]
    groups = dict_to_tuples(groups_dict)
    reduced_values = execute_reduction(results_list, targets, groups)

    if ERD.RUNS_COLS in targets_dict and QC.KEEP_RATE in targets_dict[ERD.RUNS_COLS]:
        reduced_values = {key: value for key, value in reduced_values.items() if value[0] < 1.0}

    ds_index = get_col_index(DSC.DATASET_FILE, groups)
    if ds_index != -1:
        if merge_csv_datasets:
            reduced_values = merge_univariate_datasets(reduced_values, ds_index)
        else:
            reduced_values = {
                (*key[:ds_index], key[ds_index].rsplit("/", 1)[0], *key[ds_index + 1 :]): values
                for key, values in reduced_values.items()
            }

    groups_list = [col for col in iterate_columns(groups_dict)]
    for col, regex in regex_dict.items():
        if col in groups_list:
            ind = groups_list.index(col)
            reduced_values = {key: values for key, values in reduced_values.items() if re.search(regex, str(key[ind]))}

    padding_rows = (
        max([label.count("\n") + 1 for label in get_config_labels(reduced_values, groups_dict).values()])
        if bar_plot_label_padding
        else 0
    )

    def create_plots(
        title_key: list,
        title_columns: list[Column],
        remaining_separate_plots_dict: dict[tuple, set],
        reduced_values_subset: dict[tuple, list[float]],
    ):
        if len(remaining_separate_plots_dict) > 0:
            sp_cols, sp_accepted_vals = next(iter(remaining_separate_plots_dict.items()))
            col_inds = [get_col_index(col, groups) for col in sp_cols]
            all_vals = {tuple([key[col_ind] for col_ind in col_inds]) for key in reduced_values_subset}
            for val in all_vals:
                if len(sp_accepted_vals) == 0 or val in sp_accepted_vals:
                    new_title_key = title_key.copy()
                    new_title_columns = title_columns.copy()
                    for i, col in enumerate(sp_cols):
                        new_title_key.append(val[i])
                        new_title_columns.append(col)

                    next_remaining_separate_plots_dict = copy.deepcopy(remaining_separate_plots_dict)
                    next_remaining_separate_plots_dict.pop(sp_cols)

                    next_reduced_values_subset = {
                        key: values
                        for key, values in reduced_values_subset.items()
                        if all(key[col_ind] == val[i] for i, col_ind in enumerate(col_inds))
                    }
                    create_plots(
                        new_title_key, new_title_columns, next_remaining_separate_plots_dict, next_reduced_values_subset
                    )
        else:
            y_label = get_y_label(targets)
            y_lim = (
                (0, 1.05)
                if any(col in [QC.PRUNING_RATIO, QC.KEEP_RATE] for col in targets_dict.get(ERD.RUNS_COLS, []))
                else None
            )
            title = f"{title_base}: " if title_base else ""
            title += get_config_label(tuple(title_key), title_columns, sep=", ", max_line_length=64)

            if bar_plot_color_attr is not None:
                bar_plot_color_attr_ind = get_col_index(bar_plot_color_attr, groups)
                if bar_plot_color_attr == SSC.METHOD_NAME:
                    method_keys_list = list(METHOD_LABELS.keys())
                    reduced_values_subset = sort_dict(
                        reduced_values_subset,
                        lambda x: (
                            method_keys_list.index(x[0][bar_plot_color_attr_ind]),
                            *x[0][:bar_plot_color_attr_ind],
                            *x[0][1 + bar_plot_color_attr_ind :],
                        ),
                    )

                plot_bars(
                    reduced_values_subset,
                    bar_plot_color_attr_ind,
                    x_labels=get_config_labels(
                        reduced_values_subset,
                        groups_dict,
                        discard_cols={bar_plot_color_attr},
                        padding_rows=padding_rows,
                    ),
                    y_label=y_label,
                    y_lim=y_lim,
                    title=title,
                    hatches=hatches,
                    hatch_labels=hatch_labels,
                    y_scale=y_scale,
                    bar_width_inches=bar_width_inches,
                    color_map=bar_plot_color_map,
                )

            if line_plot_x_attr is not None:
                plot_lines(
                    reduced_values_subset,
                    get_col_index(line_plot_x_attr, groups),
                    legend=get_config_labels(
                        reduced_values_subset,
                        groups_dict,
                        discard_cols={line_plot_x_attr},
                        include_cols=line_plot_included_cols,
                    ),
                    x_label=str(line_plot_x_attr).replace("_", " ").capitalize(),
                    y_label=y_label,
                    x_scale=x_scale,
                    y_scale=y_scale,
                    y_lim=y_lim,
                    title=title,
                    mark_minimum=True,
                )

            if heat_map_x_attr is not None and heat_map_y_attr is not None:
                plot_heat_map(
                    reduced_values_subset,
                    get_col_index(heat_map_x_attr, groups),
                    get_col_index(heat_map_y_attr, groups),
                    title=f"{y_label} for {title}",
                    subtitles=get_config_labels(
                        reduced_values_subset,
                        groups_dict,
                        discard_cols={heat_map_x_attr, heat_map_y_attr},
                        include_cols=heat_map_included_cols,
                    ),
                    x_label=str(heat_map_x_attr).replace("_", " ").capitalize(),
                    y_label=str(heat_map_y_attr).replace("_", " ").capitalize(),
                    color_map=COLD_TO_HOT_COLORS,
                )

    create_plots([], [], separate_plots_dict, reduced_values)


# %%[markdown]
"""
End of helper functions
"""


# %%[markdown]
"""
### Experiment: Effect of number of channels and dataset
"""


# %%


def experiment_num_channels_and_dataset(target_args: TargetArgs):
    visualize_experiments(
        logs_dirs=[
            "EXPERIMENT_LOGS/num_channels/LOGS_few_channels_config",
            "EXPERIMENT_LOGS/num_channels/LOGS_many_channels_config",
        ],
        groups_dict={
            ERD.DATASETS_COLS: [DSC.NUM_CHANNELS, DSC.DATASET_FILE],
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
        },
        **target_args.value,
        regex_dict={
            SSC.METHOD_NAME: r"^(sequential_scan|isax_envelope|isax_env_w_env|isax_env_w_sax_env).*",
        },
        bar_plot_label_padding=False,
    )


# %%

experiment_num_channels_and_dataset(TargetArgs.COMBINED_TIME)
experiment_num_channels_and_dataset(TargetArgs.PRUNING_RATIO)

# %%[markdown]
"""
### Experiment: Envelope size parametrization
"""


def experiment_envelope_parametrization(
    logs_dir: str = "EXPERIMENT_LOGS/envelope_size/LOGS_envelope_size_3",
    method_name_re: str = r".*",
    bar_width_inches: float = 0.4,
    num_query_intervals: int = 1,
    target_args: TargetArgs = TargetArgs.QUERY_TIME,
):
    visualize_experiments(
        logs_dirs=[logs_dir],
        groups_dict={
            ERD.INDEXES_COLS: [ISC.L_MIN, ISC.L_MAX, ISC.POS_PER_ENV],
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
        },
        regex_dict={SSC.METHOD_NAME: method_name_re},
        bar_width_inches=bar_width_inches,
        num_query_intervals=num_query_intervals,
        separate_plots_dict={
            (DSC.DATASET_FILE,): [("synthetic",)],
            (ISC.L_MIN, ISC.L_MAX): [(1024, 2048)],
        },
        **target_args.value,
    )


# %%

for target_arg in [TargetArgs.QUERY_TIME, TargetArgs.PRUNING_RATIO]:
    experiment_envelope_parametrization(target_args=target_arg)

# %%[markdown]
"""
### Experiment: Relative contrast
"""


# %%
NOISE_COLORS = {
    0.1: PALETTE["Greys"][0],
    0.5: PALETTE["Greys"][2],
    1.0: PALETTE["Greys"][4],
}
NOISE_LABELS = {val: f"Noise={val}" for val in NOISE_COLORS.keys()}


def experiment_relative_contrast(
    target_col: Column,
    y_label: str,
    query_noise_levels=list(NOISE_LABELS.keys()),
    y_scale: str = "linear",
    logs_dir="EXPERIMENT_LOGS/relative_contrast/LOGS_relative_contrast_config",
    remove_top=0.00,
    datasets_to_show=["weather", "synthetic"],
):
    groups_dict = {
        ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.NUM_CHANNELS, DSC.SD],
        ERD.QUERY_STATS_COLS: [QSTC.QUERY_NOISE],
    }
    columns = {**groups_dict, ERD.QUERY_STATS_COLS: [target_col] + groups_dict.get(ERD.QUERY_STATS_COLS, [])}
    rc_results = ExperimentResults.load(logs_dir=logs_dir, cols=columns, add_runs=False, add_index_stats=False)

    targets = [(ERD.QUERY_STATS_COLS, target_col, MeanReducer())]
    # TODO: not handled by visualize_experiments yet
    if remove_top > 0:
        rc_results.query_stats_df = rc_results.query_stats_df[
            rc_results.query_stats_df[target_col] < rc_results.query_stats_df[target_col].quantile(1 - remove_top)
        ]
    # end

    groups = dict_to_tuples(groups_dict)
    reduced_values = execute_reduction([rc_results], targets, groups)
    reduced_values = {group: values for group, values in reduced_values.items() if group[3] in query_noise_levels}

    reduced_values = {
        (dataset.split("/", 1)[0], num_channels, sd, noise): value
        for (dataset, num_channels, sd, noise), value in reduced_values.items()
    }
    if datasets_to_show is not None:
        reduced_values = {group: values for group, values in reduced_values.items() if group[0] in datasets_to_show}

    reduced_values = sort_dict(reduced_values, lambda x: (x[0][1], ORDERED_DATASETS.index(x[0][0]), x[0][2]))

    bar_width_inches = 0.9 / len(query_noise_levels)
    plot_bars(
        reduced_values,
        3,
        NOISE_COLORS,
        NOISE_LABELS,
        x_labels=get_config_labels(reduced_values, groups_dict, {QSTC.QUERY_NOISE}),
        y_label=y_label,
        y_scale=y_scale,
        bar_width_inches=bar_width_inches,
    )


# %%
experiment_relative_contrast(QSTC.RC_USING_MAX, "RC using max", query_noise_levels=[0.1, 0.5, 1.0])
experiment_relative_contrast(QSTC.RC_USING_MEAN, "RC using mean", query_noise_levels=[0.1, 0.5, 1.0])
experiment_relative_contrast(
    StatsColumn(QSTC.DIST_STATS, SCP.STD), "Std. dev. of distance to query", query_noise_levels=[0.1, 0.5, 1.0]
)
experiment_relative_contrast(
    StatsColumn(QSTC.DIST_STATS, SCP.MAX), "Maximum distance to query", query_noise_levels=[0.1, 0.5, 1.0]
)
experiment_relative_contrast(
    StatsColumn(QSTC.DIST_STATS, SCP.MIN), "Minimum distance to query", query_noise_levels=[0.1, 0.5, 1.0]
)
experiment_relative_contrast(
    StatsColumn(QSTC.DIST_STATS, SCP.MEAN), "Mean distance to query", query_noise_levels=[0.1, 0.5, 1.0]
)

# %%[markdown]
"""
### Experiment: Univariate parametrization
"""


# %%


def experiment_univariate_parametrization(
    target_args_dict: dict,
    y_scale: str = "linear",
    logs_dir: str = "EXPERIMENT_LOGS/univariate_param/LOGS_univariate_param_3",
    use_adapt_to_dataset: bool = False,
    merge_datasets: bool = True,
    reducer: Reducer = MeanReducer(),
):
    groups_dict = {
        ERD.DATASETS_COLS: [DSC.DATASET_FILE],
        ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
        ERD.METHODS_COLS: [SSC.METHOD_NAME],
        ERD.INDEXES_COLS: [
            ISC.FIRST_LAYER_NUM_BITS,
            ISC.POS_PER_ENV,
            ISC.LEAF_CAPACITY,
        ],
    }
    if use_adapt_to_dataset:
        groups_dict[ERD.INDEXES_COLS].append(ISC.ADAPT_TO_DATASET)

    visualize_experiments(
        logs_dirs=[logs_dir],
        groups_dict=groups_dict,
        separate_plots_dict={
            (DSC.DATASET_FILE,): [],
            (QSC.L_MIN, QSC.L_MAX): [],
        },
        merge_csv_datasets=merge_datasets,
        reducer=reducer,
        y_scale=y_scale,
        **target_args_dict,
    )


# %%

for target_args, y_scale in zip([TargetArgs.COMBINED_TIME, TargetArgs.PRUNING_RATIO], ["log", "linear"]):
    experiment_univariate_parametrization(target_args.value, y_scale)

# %%

for istc_col in [ISTC.LEAF_HEIGHT_STATS]:
    for stat in [SCP.MEAN, SCP.STD]:
        experiment_univariate_parametrization({"targets_dict": {ERD.INDEX_STATS_COLS: [StatsColumn(istc_col, stat)]}})

# %%[markdown]
"""
### Experiment: ULISSE comparison
"""


# %%
def experiment_ulisse_comparison(
    target_col: str = QC.TOTAL_TIME_S,
    logs_dir: str = "EXPERIMENT_LOGS/base_compare/LOGS_base_compare_final",
    reducer: Reducer = MeanReducer(),
    max_ulisse_pruning_ratio: float = 1.0,
    only_important: bool = False,
    bars_by_query_length: bool = False,
    search_cols: list[SSC] = [SSC.SORT_QUERY, SSC.USE_PRIORITY_QUEUE],
    index_cols: list[ISC] = [ISC.BREAKPOINT_STRATEGY, ISC.SPLIT_STRATEGY, ISC.NUM_BITS_LIMIT],
):
    if only_important and bars_by_query_length:
        raise ValueError("Cannot use both only_important and bars_by_query_length")

    groups_dict = {
        ERD.METHODS_COLS: [SSC.METHOD_NAME, *search_cols],
        ERD.INDEXES_COLS: index_cols,
        ERD.RUNS_COLS: [QC.PRUNING_RATIO, QC.QUERY_ID],
    }

    if bars_by_query_length:
        groups_dict[ERD.RUNS_COLS].append(QC.QUERY_LENGTH)

    targets_dict = {ERD.RUNS_COLS: [target_col]}
    columns = groups_dict.copy()
    columns[ERD.RUNS_COLS] += targets_dict[ERD.RUNS_COLS]

    results = ExperimentResults.load(logs_dir=logs_dir, cols=columns)

    # TODO: not handled by visualize_experiments yet
    if max_ulisse_pruning_ratio < 1.0:
        ulisse_settings_id = results.methods_df[results.methods_df[str(SSC.METHOD_NAME)].str.startswith("ulisse")]
        ulisse_settings_id = ulisse_settings_id[str(SSC.ID)].values[0]
        queries_to_keep = results.runs_df[
            (results.runs_df[str(QC.SETTINGS_ID)] == ulisse_settings_id)
            & (results.runs_df[str(QC.PRUNING_RATIO)] <= max_ulisse_pruning_ratio)
        ][str(QC.QUERY_ID)].values
        results.runs_df = results.runs_df[results.runs_df[str(QC.QUERY_ID)].isin(queries_to_keep)]
    groups_dict.pop(ERD.RUNS_COLS)

    if bars_by_query_length:
        groups_dict[ERD.RUNS_COLS] = [QC.QUERY_LENGTH]
    # end

    targets = [(ERD.RUNS_COLS, target_col, reducer) for target_col in targets_dict[ERD.RUNS_COLS]]
    groups = dict_to_tuples(groups_dict)
    reduced_values = execute_reduction([results], targets, groups)

    # TODO: not handled by visualize_experiments yet
    if only_important:
        important_run_keys = [
            ("sequential_scan-ed-early", 0, 0, 0, 0, 0),
            ("sequential_scan-mass", 0, 0, 0, 0, 0),
            ("isax_envelope-ed-early", 0, 0, "equiprobable", "entropy_maximizing", 8),
            ("isax_envelope-mass", 0, 0, "equiprobable", "entropy_maximizing", 8),
            ("isax_env_w_env-ed-early", 0, 1, "equiprobable", "entropy_maximizing", 8),
            ("isax_env_w_env-mass", 0, 1, "equiprobable", "entropy_maximizing", 8),
            ("envelope-ed-early", 0, 1, 0, 0, 0),
            ("envelope-mass", 0, 1, 0, 0, 0),
            ("isax_env_w_sax_env-ed-early", 1, 0, "fixed", "ulisse_closest_to_mean", 8),
            ("ulisse_single-ed-early", 1, 0, "fixed", "ulisse_closest_to_mean", 8),
            ("ulisse_parallel-ed-early", 1, 0, "fixed", "ulisse_closest_to_mean", 8),
        ]
        reduced_values = {key: reduced_values[key] for key in important_run_keys}
    # end

    method_labels_keys = list(METHOD_LABELS.keys())
    reduced_values = sort_dict(reduced_values, lambda x: method_labels_keys.index(x[0][0]))

    plot_bars(
        reduced_values,
        0,
        x_labels=get_config_labels(reduced_values, groups_dict),
        y_label=get_y_label(targets),
        y_scale="linear",
    )


# %%

experiment_ulisse_comparison(
    target_col=QC.KEEP_RATE,
    # logs_dir="EXPERIMENT_LOGS/base_compare/LOGS_5M",
    # logs_dir="EXPERIMENT_LOGS/base_compare/LOGS_5M_node",
    logs_dir="EXPERIMENT_LOGS/base_compare/LOGS_base_compare_raw_5M",
    # max_ulisse_pruning_ratio=0.0,
    bars_by_query_length=True,
    # reducer=MaxReducer(),
    search_cols=[],
    index_cols=[],
)

# %%
experiment_ulisse_comparison(
    target_col=QC.TOTAL_TIME_S, logs_dir="EXPERIMENT_LOGS/base_compare/LOGS_100K", only_important=True
)

# %%[markdown]
"""
### Experiment: Length-based grouping results
"""


# %%
def experiment_length_based_grouping(target_args: TargetArgs):
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/length_grouping/LOGS_env_size_param"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            ERD.INDEXES_COLS: [ISC.POS_PER_ENV, ISC.L_PER_GROUP],
        },
        separate_plots_dict={
            (DSC.DATASET_FILE,): [],
            (QSC.L_MIN, QSC.L_MAX): [],
        },
        num_query_intervals=1,
        merge_csv_datasets=True,
        **target_args.value,
    )


# %%

for target_args in [TargetArgs.COMBINED_TIME, TargetArgs.PRUNING_RATIO]:
    experiment_length_based_grouping(target_args=target_args)


# %%[markdown]
"""
Experiment: Segmentation strategy
"""

# %%


def experiment_segmentation_strategy(target_args_dict: dict, reducer: Reducer):
    visualize_experiments(
        logs_dirs=["LOGS"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.INDEXES_COLS: [
                ISC.MAX_WIDTH_CHANGE,
                ISC.LEAF_CAPACITY,
                ISC.NUM_ENVELOPES,
                ISC.NUM_LEN_GROUPS,
                ISC.NUM_SEGMENTS,
            ],
        },
        separate_plots_dict={
            (DSC.DATASET_FILE,): [],
        },
        # regex_dict={ISC.SCORE_BASED_CHSS_SAMPLE_SIZE: r"^(50|0)"},
        num_query_intervals=1,
        merge_csv_datasets=True,
        y_scale="linear",
        bar_plot_color_attr=SSC.METHOD_NAME,
        # bar_plot_color_map={
        #     "single": PALETTE["Oranges"][2],
        #     "score_based": PALETTE["Blues"][4],
        #     "multi": PALETTE["Greens"][4],
        # },
        # line_plot_x_attr=ISC.POS_PER_ENV,
        # line_plot_included_cols={DSC.DATASET_FILE},
        # x_scale="log",
        # heat_map_x_attr=ISC.NUM_ENVELOPES,
        # heat_map_y_attr=ISC.NUM_LEN_GROUPS,
        # heat_map_included_cols={ISC.NUM_SEGMENTS},
        reducer=reducer,
        **target_args_dict,
    )


for target_args_dict, reducer in [
    # (TargetArgs.COMBINED_TIME.value, MeanReducer()),
    (TargetArgs.PRUNING_RATIO.value, MeanReducer()),
    (TargetArgs.INDEX_SIZE.value, MeanReducer()),
]:
    experiment_segmentation_strategy(target_args_dict=target_args_dict, reducer=reducer)

# %%[markdown]
"""
Experiment: Index size
"""

# %%


def experiment_index_size(target_args: TargetArgs):
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/tree_envelope/LOGS_index_size"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.NUM_SERIES],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            ERD.INDEXES_COLS: [ISC.POS_PER_ENV, ISC.L_PER_GROUP],
        },
        separate_plots_dict={
            (DSC.DATASET_FILE, DSC.NUM_SERIES): [],
            (QSC.L_MIN, QSC.L_MAX): [],
        },
        num_query_intervals=1,
        merge_csv_datasets=True,
        **target_args.value,
    )


for target_args in [TargetArgs.COMBINED_TIME, TargetArgs.INDEX_INFO]:
    experiment_index_size(target_args=target_args)
