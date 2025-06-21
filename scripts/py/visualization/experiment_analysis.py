# %%
import os

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
from scripts.py.visualization.helpers import dict_to_tuples, sort_dict
from scripts.py.visualization.plots import METHOD_LABELS, ORDERED_DATASETS, get_config_labels, get_y_label, plot_bars
from scripts.py.visualization.reduction import ERD, ExperimentResults, MeanReducer, Reducer, execute_reduction
from scripts.py.visualization.style import PALETTE
from scripts.py.visualization.wrapper import TargetArgs, visualize_experiments

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
Experiment: Segmentation strategy
"""

# %%


def experiment_segmentation_strategy(target_args_dict: dict, reducer: Reducer):
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/envelope_variants/LOGS_size_limiting"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            # ERD.INDEXES_COLS: [ISC.INDEX_SIZE_LIMIT, ISC.NUM_LEN_GROUPS, ISC.NUM_SEGMENTS],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
        },
        separate_plots_dict={
            (DSC.DATASET_FILE,): [("synthetic",)],
            (ISC.INDEX_SIZE_LIMIT,): [(1.0,)],
            (QSC.L_MIN, QSC.L_MAX): [(128, 1024)],
        },
        # regex_dict={SSC.METHOD_NAME: r"envelope"},
        num_query_intervals=1,
        merge_csv_datasets=False,
        y_scale="linear",
        bar_plot_color_attr=None,
        # bar_plot_color_attr=SSC.METHOD_NAME,
        # bar_plot_color_attr=ISC.CH_SEGMENTATION_STRATEGY,
        # bar_plot_color_map={
        #     "single": PALETTE["Oranges"][2],
        #     "score_based": PALETTE["Blues"][4],
        #     "multi": PALETTE["Greens"][4],
        # },
        # line_plot_x_attr=ISC.POS_PER_ENV,
        # line_plot_included_cols={DSC.DATASET_FILE},
        x_scale="log",
        heat_map_x_attr=ISC.NUM_SEGMENTS,
        heat_map_y_attr=ISC.NUM_LEN_GROUPS,
        heat_map_included_cols={},
        reducer=reducer,
        **target_args_dict,
    )


for target_args_dict, reducer in [
    (TargetArgs.QUERY_TIME.value, MeanReducer()),
    (TargetArgs.PRUNING_RATIO.value, MeanReducer()),
    # ({"targets_dict": {ERD.INDEXES_COLS: [ISC.NUM_ENVELOPES]}}, MeanReducer()),
    # ({"targets_dict": {ERD.INDEX_STATS_COLS: [StatsColumn(ISTC.SEG_RANGE_STATS, SCP.MEAN)]}}, MeanReducer()),
    ({"targets_dict": {ERD.RUNS_COLS: [QC.MIN_DIST_AVG]}}, MeanReducer()),
    # ({"targets_dict": {ERD.RUNS_COLS: [QC.MIN_DIST_TOTAL]}}, MeanReducer()),
    # ({"targets_dict": {ERD.INDEXES_COLS: [ISC.POS_PER_ENV]}}, MeanReducer()),
    # (TargetArgs.INDEX_SIZE.value, MeanReducer()),
    # ({"targets_dict": {ERD.INDEXES_COLS: [ISC.ESTIMATED_SIZE_ON_DISK_B]}}, MeanReducer()),
]:
    experiment_segmentation_strategy(target_args_dict=target_args_dict, reducer=reducer)
