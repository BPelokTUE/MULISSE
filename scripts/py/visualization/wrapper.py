import copy
import os
import re
from enum import Enum

if True:
    while not os.getcwd().endswith("MULISSE"):
        os.chdir("..")

from scripts.py.common.columns import Column
from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import IndexSettingsColumn as ISC
from scripts.py.common.columns import ParamEstimatesColumn as PEC
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import SearchSettingsColumn as SSC
from scripts.py.visualization.helpers import (
    dict_to_tuples,
    get_col_index,
    get_col_indexes,
    iterate_columns,
    merge_univariate_datasets,
    sort_dict,
)
from scripts.py.visualization.plots import (
    FIRST_LAYER_TIME_HATCH,
    METHOD_COLORS,
    METHOD_LABELS,
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
from scripts.py.visualization.style import CATEGORY_COLORS, COLD_TO_HOT_COLORS

SAVE_EXTENSION = "pdf"

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
    ESTIMATE_SCORE = {"targets_dict": {ERD.PARAM_ESTIMATES_COLS: [PEC.SCORE]}}
    NUM_LEN_GROUPS = {"targets_dict": {ERD.INDEXES_COLS: [ISC.NUM_LEN_GROUPS]}}
    ENV_PARAM_ESTIMATION_TIME = {"targets_dict": {ERD.INDEXES_COLS: [ISC.ENV_PARAM_ESTIMATION_TIME_S]}}


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
    separate_plots_dict: dict[tuple, list] = {},
    # Filtering
    regex_dict: dict[Column, str | tuple] = {},
    regex_sep: str = "::",
    ignored_attrs: set = set(),
    # Plotting
    x_scale: str = "linear",
    y_scale: str = "linear",
    x_ticks_rotation: int = 0,
    title_base: str = "",
    legend_max_cols: int = 4,
    legend_offset: float = 0.18,
    legend_plots_dict: dict[tuple, str] | None = {},
    legend_only_hatch: bool = False,
    fig_height_inches: float = 3.5,
    # Bar plots
    hatches=None,
    hatch_labels=None,
    bar_plot_label_padding: bool = True,
    bar_plot_no_x_ticks: bool = False,
    bar_width_inches: float = 0.4,
    bar_gap_inches: float = 0.4,
    bar_plot_color_attrs: Column | list[Column] | None = SSC.METHOD_NAME,
    bar_plot_color_map: dict[str | tuple, str] = METHOD_COLORS,
    bar_plot_label_map: dict[str | tuple, str] = METHOD_LABELS,
    # Line plots
    line_plot_x_attr: Column | None = None,
    line_plot_included_cols: set[Column] | None = None,
    line_plot_legend_map: dict[tuple, str] = {},
    line_plot_colors_map: dict[tuple, str] = {},
    line_plot_thickness: float = 2.25,
    line_plot_show_min: bool = True,
    # Heat maps
    heat_map_x_attr: Column | None = None,
    heat_map_y_attr: Column | None = None,
    heat_map_included_cols: set[Column] | None = None,
    cell_height_inches: float = 5.0,
    cell_width_inches: float = 5.0,
    # Output
    return_values: bool = False,
    save_dir: str | None = None,
    verbose: bool = False,
):
    if num_query_intervals > 1:
        groups_dict[ERD.RUNS_COLS] = groups_dict.get(ERD.RUNS_COLS, []) + [QC.QUERY_INTERVAL]
    columns = groups_dict.copy()
    for erd, target_cols in targets_dict.items():
        columns[erd] = target_cols + groups_dict.get(erd, [])

    def should_add(erd: ERD) -> bool:
        return any([len(d.get(erd, [])) > 0 for d in [groups_dict, targets_dict]])

    add_runs = should_add(ERD.RUNS_COLS)
    add_methods = add_runs or should_add(ERD.METHODS_COLS)

    results_list = [
        ExperimentResults.load(
            logs_dir=logs_dir,
            cols=columns,
            num_query_intervals=num_query_intervals,
            add_runs=add_runs,
            add_methods=add_methods,
            add_param_estimates=should_add(ERD.PARAM_ESTIMATES_COLS),
            add_dataset_stats=should_add(ERD.DATASET_STATS_COLS),
            add_query_stats=should_add(ERD.QUERY_STATS_COLS),
            add_index_stats=should_add(ERD.INDEX_STATS_COLS),
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
    for re_key, regex in regex_dict.items():
        cols = re_key if isinstance(re_key, tuple) else (re_key,)
        inds = [groups_list.index(col) for col in cols if col in groups_list]
        reduced_values = {
            key: values
            for key, values in reduced_values.items()
            if re.search(regex, regex_sep.join([str(key[ind]) for ind in inds]))
        }

    act_ignored_attrs = ignored_attrs.copy()
    act_ignored_attrs.update({attr for key in separate_plots_dict for attr in key})
    ignored_attr_inds = {get_col_index(attr, groups) for attr in act_ignored_attrs}
    values_to_return = {}

    def create_plots(
        title_key: list,
        title_columns: list[Column],
        remaining_separate_plots_dict: dict[tuple, list],
        reduced_values_subset: dict[tuple, list],
    ):
        if len(remaining_separate_plots_dict) > 0:
            sp_cols, sp_accepted_vals = next(iter(remaining_separate_plots_dict.items()))
            col_inds = [get_col_index(col, groups) for col in sp_cols]
            all_vals = set(sorted([tuple([key[col_ind] for col_ind in col_inds]) for key in reduced_values_subset]))
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

            reduced_values_subset = {
                tuple([k for i, k in enumerate(key) if i not in ignored_attr_inds]): val
                for key, val in reduced_values_subset.items()
            }

            groups_dict_filtered = {
                erd: [col for col in cols if col not in act_ignored_attrs] for erd, cols in groups_dict.items()
            }
            groups_filtered = dict_to_tuples(groups_dict_filtered)

            labels_dict = get_config_labels(reduced_values_subset, groups_dict_filtered)
            padding_rows = (
                max([label.count("\n") + 1 for label in labels_dict.values()]) if bar_plot_label_padding else 0
            )

            def get_plot_save_path(type: str, suffix: str = "") -> str | None:
                if save_dir is not None:
                    return os.path.join(
                        save_dir, f"{'-'.join([str(k) for k in title_key])}_{type}{suffix}.{SAVE_EXTENSION}"
                    )
                return None

            add_legend = legend_plots_dict is not None
            if legend_plots_dict is not None:
                for col, val in zip(title_columns, title_key):
                    accepted_vals = legend_plots_dict.get(col, None)
                    if accepted_vals is not None and val not in accepted_vals:
                        add_legend = False
                        break

            if verbose:
                max_key_length = max([len(str(key)) for key in reduced_values_subset.keys()])
                for key, values in reduced_values_subset.items():
                    print(f"{str(key):<{max_key_length}}:\t{values}")
            if return_values:
                values_to_return[tuple(title_key)] = reduced_values_subset

            # TODO: this is horrible
            suffixes = [""]
            if save_dir is not None:
                os.makedirs(os.path.dirname(get_plot_save_path("")), exist_ok=True)
                # If legend was specifically requested only for certain plots, create separate version with legend
                if add_legend and len(legend_plots_dict) > 0:
                    suffixes.append("_legend")
                    if legend_only_hatch:
                        suffixes.append("_hatch_legend")

            if bar_plot_color_attrs is not None:
                use_tuple_keys = isinstance(bar_plot_color_attrs, list)
                bar_plot_color_attrs_inds = (
                    get_col_indexes(bar_plot_color_attrs, groups_filtered)
                    if use_tuple_keys
                    else get_col_index(bar_plot_color_attrs, groups_filtered)
                )
                if bar_plot_label_map is not None:
                    label_keys_list = list(bar_plot_label_map.keys())

                    if use_tuple_keys:

                        def key_func(x):
                            return (
                                label_keys_list.index(tuple([x[0][ind] for ind in bar_plot_color_attrs_inds])),
                                *[val for ind, val in enumerate(list(x[0])) if ind not in bar_plot_color_attrs_inds],
                            )
                    else:

                        def key_func(x):
                            return (
                                label_keys_list.index(x[0][bar_plot_color_attrs_inds]),
                                *x[0][:bar_plot_color_attrs_inds],
                                *x[0][1 + bar_plot_color_attrs_inds :],
                            )

                    reduced_values_subset = sort_dict(reduced_values_subset, key_func=key_func)

                discard_cols = (
                    set(bar_plot_color_attrs) if isinstance(bar_plot_color_attrs, list) else {bar_plot_color_attrs}
                )

                for suffix in suffixes:
                    plot_bars(
                        reduced_values_subset,
                        bar_plot_color_attrs_inds,
                        x_labels=get_config_labels(
                            reduced_values_subset,
                            groups_dict_filtered,
                            discard_cols=discard_cols,
                            padding_rows=padding_rows,
                        )
                        if not bar_plot_no_x_ticks
                        else {},
                        x_ticks_rotation=x_ticks_rotation,
                        y_label=y_label,
                        y_lim=y_lim,
                        title=title,
                        hatches=hatches,
                        hatch_labels=hatch_labels,
                        y_scale=y_scale,
                        bar_width_inches=bar_width_inches,
                        bar_gap_inches=bar_gap_inches,
                        fig_height_inches=fig_height_inches,
                        color_map=bar_plot_color_map,
                        label_map=bar_plot_label_map,
                        legend_max_cols=legend_max_cols,
                        legend_offset=legend_offset,
                        add_legend=add_legend if len(suffixes) == 1 else len(suffix) > 0,
                        only_hatch_legend=suffix == "_hatch_legend",
                        save_path=get_plot_save_path("bar", suffix),
                    )

            if line_plot_x_attr is not None:
                config_label_map = get_config_labels(
                    reduced_values_subset,
                    groups_dict_filtered,
                    discard_cols={line_plot_x_attr},
                    include_cols=line_plot_included_cols,
                )
                legend = {key: line_plot_legend_map.get(key, val) for key, val in config_label_map.items()}
                label_order = {key: i for i, key in enumerate(line_plot_legend_map.keys())}
                legend = dict(sorted(legend.items(), key=lambda item: label_order.get(item[0], float("inf"))))
                colors = {
                    key: line_plot_colors_map.get(key, CATEGORY_COLORS[i % len(CATEGORY_COLORS)])
                    for i, key in enumerate(config_label_map)
                }

                for suffix in suffixes:
                    plot_lines(
                        reduced_values_subset,
                        get_col_index(line_plot_x_attr, groups_filtered),
                        legend=legend,
                        colors=colors,
                        x_label=str(line_plot_x_attr).replace("_", " ").capitalize(),
                        y_label=y_label,
                        x_scale=x_scale,
                        y_scale=y_scale,
                        y_lim=y_lim,
                        line_thickness=line_plot_thickness,
                        title=title,
                        fig_height_inches=fig_height_inches,
                        legend_max_cols=legend_max_cols,
                        legend_offset=legend_offset,
                        add_legend=add_legend if len(suffixes) == 1 else suffix == "_legend",
                        mark_minimum=line_plot_show_min,
                        save_path=get_plot_save_path("line", suffix),
                    )

            if heat_map_x_attr is not None and heat_map_y_attr is not None:
                plot_heat_map(
                    reduced_values_subset,
                    get_col_index(heat_map_x_attr, groups_filtered),
                    get_col_index(heat_map_y_attr, groups_filtered),
                    title=f"{y_label} for {title}",
                    subtitles=get_config_labels(
                        reduced_values_subset,
                        groups_dict_filtered,
                        discard_cols={heat_map_x_attr, heat_map_y_attr},
                        include_cols=heat_map_included_cols,
                    ),
                    x_label=str(heat_map_x_attr).replace("_", " ").capitalize(),
                    y_label=str(heat_map_y_attr).replace("_", " ").capitalize(),
                    color_map=COLD_TO_HOT_COLORS,
                    cell_height_inches=cell_height_inches,
                    cell_width_inches=cell_width_inches,
                    save_path=get_plot_save_path("hm"),
                )

    create_plots([], [], separate_plots_dict, reduced_values)

    if return_values:
        return values_to_return


# %%
