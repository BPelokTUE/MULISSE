import textwrap
from copy import copy
from typing import Any, Callable

import numpy as np
import seaborn as sns
from matplotlib import pyplot as plt
from matplotlib.colors import Colormap
from scripts.py.common.columns import Column
from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import IndexSettingsColumn as ISC
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import QuerySetSettingsColumn as QSC
from scripts.py.common.columns import SearchSettingsColumn as SSC
from scripts.py.visualization.helpers import iterate_columns
from scripts.py.visualization.reduction import (
    ERD,
    MaxReducer,
    MeanReducer,
    MinReducer,
    Reducer,
    ReductionResult,
    StdReducer,
)
from scripts.py.visualization.style import CATEGORY_COLORS, PALETTE

METHOD_COLORS = {
    "sequential_scan-ed": PALETTE["Yellows"][2],
    "base_ed-ed": PALETTE["Yellows"][3],
    "sequential_scan-ed-early": PALETTE["Yellows"][5],
    "sequential_scan-mass": PALETTE["Oranges"][0],
    "base_mass-mass": PALETTE["Oranges"][1],
    "sequential_scan-mass-ffts": PALETTE["Oranges"][2],
    "ulisse_single-ed-early": PALETTE["Greys"][1],
    "ulisse_parallel-ed-early": PALETTE["Greys"][3],
    "isax_envelope-ed": PALETTE["Blues"][0],
    "isax_envelope-ed-early": PALETTE["Blues"][2],
    "isax_envelope-mass": PALETTE["Blues"][4],
    "isax_envelope-mass-ffts": PALETTE["Blues"][6],
    "isax-ed": PALETTE["Reds"][1],
    "isax-ed-early": PALETTE["Reds"][1],
    "isax-mass": PALETTE["Reds"][2],
    "isax-mass-ffts": PALETTE["Reds"][4],
    "envelope-ed": PALETTE["Pinks"][1],
    "envelope-ed-early": PALETTE["Pinks"][1],
    "sax_envelope-ed": PALETTE["Pinks"][4],
    "sax_envelope-ed-early": PALETTE["Pinks"][4],
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
ORDERED_DATASETS = [
    "weather",
    "stocks",
    "random_walk",
    "synthetic",
]

# %%[markdown]
"""
### Bar plot function
"""

# %%


def plot_bars(
    reduction_result: ReductionResult,
    color_group_ind: int,
    color_map: dict = METHOD_COLORS,
    label_map: dict = METHOD_LABELS,
    x_labels: dict[tuple, str] = {},
    y_label: str = "",
    y_lim: tuple[float, float] = None,
    y_scale: str = "linear",
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
    :param y_scale: The scale to use for the y-axis.
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
    if len(bar_groups) == 0:
        return

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

    ax.set_yscale(y_scale)
    ax.yaxis.grid(True)
    ax.set_ylabel(y_label)
    ax.set_ylim(y_lim)
    ax.set_xticks(x_ticks)
    ax.set_xticklabels(x_tick_labels)
    ax.set_title(title)

    fig.set_size_inches((num_bars + len(bar_groups)) * bar_width_inches, 6)
    plt.show()


# %%


def plot_lines(
    reduction_result: ReductionResult,
    x_axis_attr_ind: int,
    legend: list[str],
    colors: list[str] = CATEGORY_COLORS,
    x_label: str = "",
    y_label: str = "",
    y_lim: tuple[float, float] | None = None,
    x_scale: str = "linear",
    y_scale: str = "linear",
    title: str = None,
    only_max_points: bool = True,
    mark_minimum: bool = False,
):
    """
    Plot lines for the given reduction result.

    :param reduction_result: The reduction result to plot.
    :param x_axis_attr_ind: The index of the attribute in the keys of the reduction result to use for the x-axis.
    :param legend: The list of labels for the lines.
    :param colors: The list of colors for the lines.
    :param x_label: The label for the x-axis.
    :param y_label: The label for the y-axis.
    :param y_lim: The range to use for the y-axis. If `None`, the range is automatically determined.
    :param y_scale: The scale to use for the y-axis.
    :param title: The title of the plot.
    :param only_max_points: If `True`, only plot lines with the maximum number of points.
    :param mark_minimum: If `True`, mark the minimum points on each line.
    """
    values = {}
    max_points = 0
    for i, (group, target_values) in enumerate(reduction_result.items()):
        x = group[x_axis_attr_ind]
        if x is None or x == "":
            continue

        line = group[:x_axis_attr_ind] + group[x_axis_attr_ind + 1 :]
        if line not in values:
            values[line] = []
        y = sum(target_values)
        values[line].append((x, y))
        max_points = max(max_points, len(values[line]))

    fig, ax = plt.subplots()
    for color, (line, points) in zip(colors, values.items()):
        if only_max_points and len(points) != max_points:
            continue

        sorted_points = sorted(points, key=lambda p: p[0])
        if len(points) > 1 and mark_minimum:
            min_point = min(sorted_points, key=lambda p: p[1])
            if x_scale == "log":
                hline_half_length = (np.log10(sorted_points[-1][0]) - np.log10(sorted_points[0][0])) * 0.1
                line_start = 10 ** (np.log10(min_point[0]) - hline_half_length)
                line_end = 10 ** (np.log10(min_point[0]) + hline_half_length)
            else:
                hline_half_length = (sorted_points[-1][0] - sorted_points[0][0]) * 0.1
                line_start = min_point[0] - hline_half_length
                line_end = min_point[0] + hline_half_length
            ax.hlines(min_point[1], line_start, line_end, linestyle="--", color=color)

        xs = [point[0] for point in sorted_points]
        ys = [point[1] for point in sorted_points]
        ax.plot(xs, ys, color=color, label=legend[line])
        ax.scatter(xs, ys, color=color)

    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.set_xscale(x_scale)
    ax.set_yscale(y_scale)
    ax.set_ylim(y_lim)
    ax.set_title(title)
    ax.grid(True)
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))

    plt.show()


# %%


def plot_heat_map(
    reduction_result: ReductionResult,
    x_axis_attr_ind: int,
    y_axis_attr_ind: int,
    title: str,
    subtitles: dict[tuple, str],
    color_map: Colormap,
    x_label: str = "",
    y_label: str = "",
    only_max_points_x: bool = True,
    only_max_points_y: bool = True,
    max_maps_per_row: int = 3,
    map_inches: float = 5.0,
):
    """
    Plot lines for the given reduction result.

    :param reduction_result: The reduction result to plot.
    :param x_axis_attr_ind: The index of the attribute in the keys of the reduction result to use for the x-axis.
    :param y_axis_attr_ind: The index of the attribute in the keys of the reduction result to use for the y-axis.
    :param title: The prefix for the title of the heat maps.
    :param subtitles: The subtitles for the heat maps.
    :param color_map: The color map to use for the heat maps.
    :param x_label: The label for the x-axis.
    :param y_label: The label for the y-axis.
    :param only_max_points_x: If `True`, only heat maps with maximum number of points on the x-axis are plotted.
    :param only_max_points_y: If `True`, only heat maps with maximum number of points on the y-axis are plotted.
    :param max_maps_per_row: The maximum number of heat maps to plot per row.
    :param map_inches: The width and height of the heat maps in inches.
    """

    values = {}
    max_points_x, max_points_y = 0, 0
    heat_lim = (np.inf, -np.inf)

    for i, (group, target_values) in enumerate(reduction_result.items()):
        x = group[x_axis_attr_ind]
        y = group[y_axis_attr_ind]
        if x is None or x == "" or y is None or y == "":
            continue

        heat_map_key = tuple(group[i] for i in range(len(group)) if i != x_axis_attr_ind and i != y_axis_attr_ind)

        if heat_map_key not in values:
            values[heat_map_key] = {}
        if y not in values[heat_map_key]:
            values[heat_map_key][y] = {}

        val = sum(target_values)
        values[heat_map_key][y][x] = val
        heat_lim = (min(heat_lim[0], val), max(heat_lim[1], val))

        max_points_x = max(max_points_x, len(values[heat_map_key][y]))
        max_points_y = max(max_points_y, len(values[heat_map_key]))

    heat_pad = (heat_lim[1] - heat_lim[0]) * 0.05
    heat_lim = (heat_lim[0] - heat_pad, heat_lim[1] + heat_pad)

    heat_map_matrices = {}
    heat_map_ticks = {}

    for heat_map_key, heat_map_values in values.items():
        y_values = sorted(heat_map_values.keys())
        x_values = list(set([x for y in y_values for x in heat_map_values[y].keys()]))

        if only_max_points_x and len(x_values) != max_points_x:
            continue
        if only_max_points_y and len(y_values) != max_points_y:
            continue

        data = np.zeros((len(y_values), len(x_values)))
        for y_ind, y in enumerate(y_values):
            for x in x_values:
                x_ind = x_values.index(x)
                data[y_ind][x_ind] = heat_map_values[y][x]

        heat_map_matrices[heat_map_key] = data
        heat_map_ticks[heat_map_key] = (x_values, y_values)

    num_heat_maps = len(heat_map_matrices)
    if num_heat_maps == 0:
        return

    num_rows = (num_heat_maps + max_maps_per_row - 1) // max_maps_per_row
    num_cols = min(num_heat_maps, max_maps_per_row)
    fig, axes = plt.subplots(nrows=num_rows, ncols=num_cols, figsize=(map_inches * num_cols, map_inches * num_rows))

    if num_heat_maps == 1:
        axes = np.array([axes])
    elif num_rows > 1 and num_cols > 1:
        axes = axes.flatten()

    for i, heat_map_key in enumerate(heat_map_matrices):
        matrix = heat_map_matrices[heat_map_key]
        x_values, y_values = heat_map_ticks[heat_map_key]

        ax = axes[i]
        sns.heatmap(data=matrix, vmin=heat_lim[0], vmax=heat_lim[1], annot=True, ax=ax, cmap=color_map)

        # Set labels and title
        ax.set_title(subtitles[heat_map_key])
        ax.set_xlabel(x_label)
        ax.set_ylabel(y_label)
        ax.set_xticklabels(x_values)
        ax.set_yticklabels(y_values)

    # Hide any unused subplots
    for i in range(num_heat_maps, len(axes)):
        axes[i].axis("off")

    fig.suptitle(title)
    plt.tight_layout(rect=[0, 0, 1, 0.96])  # Make room for the suptitle
    plt.show()


# %%[markdown]
"""
Misc. helpers
"""

# %%
TIME_TARGETS = [QC.TOTAL_TIME_S, QC.AMORTIZED_PREP_TIME_S]
TIME_LABELS = ["Search time", "Prep. time"]
PREP_TIME_HATCH = "/////"

PQ_TIME_TARGETS = [QC.TS_EXAMINATION_TIME_S, QC.FIRST_LAYER_TIME_S]
PQ_TIME_LABELS = ["TS examination time", "First layer time"]
FIRST_LAYER_TIME_HATCH = "+++"

TOTAL_TIME_Y_LABEL = "Query time (S)"
AMORTIZED_PREP_TIME_Y_LABEL = "Amortized prep. time (S)"
TS_EXAMINATION_TIME_Y_LABEL = "Time for TS examination (S)"
FIRST_LAYER_TIME_S_Y_LABEL = "Time for first layer (S)"
PRUNING_RATIO_Y_LABEL = "Pruning ratio"
ABANDONING_RATE_Y_LABEL = "Abandoning rate"
KEEP_RATE_Y_LABEL = "Keep rate (1 - abandoning rate)"
NUM_PTS_EXAMINED_Y_LABEL = "Number of points examined"
NUM_PTS_IN_EXAMINED_ENTRIES_Y_LABEL = "Number of points in examined entries"

Y_LABELS = {
    QC.TOTAL_TIME_S: TOTAL_TIME_Y_LABEL,
    QC.AMORTIZED_PREP_TIME_S: AMORTIZED_PREP_TIME_Y_LABEL,
    QC.TS_EXAMINATION_TIME_S: TS_EXAMINATION_TIME_Y_LABEL,
    QC.FIRST_LAYER_TIME_S: FIRST_LAYER_TIME_S_Y_LABEL,
    QC.ABANDONING_RATE: ABANDONING_RATE_Y_LABEL,
    QC.PRUNING_RATIO: PRUNING_RATIO_Y_LABEL,
    QC.KEEP_RATE: KEEP_RATE_Y_LABEL,
    QC.NUM_PTS_EXAMINED: NUM_PTS_EXAMINED_Y_LABEL,
    QC.NUM_PTS_IN_EXAMINED_ENTRIES: NUM_PTS_IN_EXAMINED_ENTRIES_Y_LABEL,
}


def abbreviate(name: str, max_len: int = 5) -> str:
    return (name if len(name) <= 5 else f"{name[:5]}.").capitalize()


def get_x_label(
    key: tuple,
    columns: dict[ERD, list[str]],
    ignore_cols: set[Column] = set(),
    include_cols: set[Column] | None = None,
    padding: str = "",
    num_query_intervals: int = 0,
    sep: str = "\n",
    max_line_length: int = 20,
) -> str:
    label_parts = []
    length_values = {}

    for col, val in zip(iterate_columns(columns), key):
        if col in ignore_cols or (include_cols is not None and col not in include_cols):
            continue

        match col:
            case DSC.L_MIN | ISC.L_MIN | QSC.L_MIN:
                length_values["l_min"] = val
            case DSC.L_MAX | ISC.L_MAX | QSC.L_MAX:
                length_values["l_max"] = val
            case QC.QUERY_INTERVAL:
                length_values["l_q_interval"] = val
            case ISC.L_PER_GROUP:
                l_per_group = val
                if l_per_group is not None and l_per_group > 0:
                    length_values["l_per_group"] = l_per_group
            case QC.QUERY_LENGTH:
                label_parts.append(f"|Q|={int(val)}")
            case ISC.POS_PER_ENV:
                label_parts.append(f"PPE={int(val)}")
            case DSC.DATASET_FILE:
                label_parts.append(val)
            case DSC.NUM_SERIES:
                label_parts.append(f"n={int(val)}")
            case DSC.NUM_CHANNELS:
                label_parts.append(f"|C|={int(val)}")
            case DSC.SD:
                label_parts.append(f"SD={int(val)}")
            case ISC.FIRST_LAYER_NUM_BITS:
                if val is not None and val > 0:
                    label_parts.append(f"FLB={int(val)}")
            case ISC.NUM_BITS_LIMIT:
                if val is not None and val > 0:
                    label_parts.append(f"BLim={int(val)}")
            case ISC.LEAF_CAPACITY:
                if val is not None and val > 0:
                    label_parts.append(
                        f"LC={int(val) if val < 10000 else f'{int(val / 1000)}K' if val < 1e6 else f'{round(val / 1e6, 2)}M'}"
                    )
            case ISC.ADAPT_TO_DATASET:
                if val == 1:
                    label_parts.append("Adapt")
            case SSC.SORT_QUERY:
                if val == 1:
                    label_parts.append("Sort")
            case SSC.USE_PRIORITY_QUEUE:
                if val == 1:
                    label_parts.append("PQ")
            case ISC.LG_SEGMENTATION_STRATEGY:
                if isinstance(val, str) and len(val) > 0:
                    label_parts.append(f"LGS={abbreviate(val)}")
            case ISC.SEGMENTATION_STRATEGY:
                if isinstance(val, str) and len(val) > 0:
                    label_parts.append(f"SEG={abbreviate(val)}")
            case ISC.BREAKPOINT_STRATEGY:
                if isinstance(val, str) and len(val) > 0:
                    label_parts.append(f"BRK={abbreviate(val)}")
            case ISC.SPLIT_STRATEGY:
                if isinstance(val, str) and len(val) > 0:
                    label_parts.append("".join(s[0].upper() for s in val.split("_")))
            case ISC.NUM_SEGMENTS:
                if val is not None and val > 0:
                    label_parts.append(f"|S|={int(val)}")

    if "l_min" in length_values:
        l_min = length_values["l_min"]
        if "l_max" in length_values:
            l_max = length_values["l_max"]
            label_parts.append(f"{l_min}≤l≤{l_max}")
            if num_query_intervals > 0 and "l_q_interval" in length_values:
                query_interval = length_values["l_q_interval"]
                query_interval_size = int(np.ceil((l_max - l_min + 1) / num_query_intervals))
                low_len = l_min + query_interval * query_interval_size
                high_len = min(l_min + (query_interval + 1) * query_interval_size, l_max + 1)
                label_parts.append(f"{sep}{int(low_len)}≤|Q|<{int(high_len)}")
            if "l_per_group" in length_values:
                num_l_groups = int(np.ceil((l_max - l_min + 1) / l_per_group))
                label_parts.append(f"#LG={num_l_groups}")
        else:
            label_parts.append(f"{int(l_min)}≤l")
    elif "l_max" in length_values:
        label_parts.append(f"l<{int(length_values['l_max'])}")

    label = sep.join(label_parts)
    if "\n" not in label:
        label = textwrap.fill(
            sep.join(label_parts), width=max_line_length, break_long_words=False, break_on_hyphens=False
        )
    return padding + label


def get_x_labels(
    reduced_values: ReductionResult,
    groups_dict: dict[ERD, list[Column]],
    ignore_cols: set[Column] = set(),
    discard_cols: set[Column] = {SSC.METHOD_NAME},
    include_cols: set[Column] | None = None,
    num_query_intervals: int = 0,
    padding_rows: int = 0,
    sep: str = "\n",
    max_line_length: int = 30,
) -> dict[tuple, str]:
    x_labels = {}

    keys = list(reduced_values.keys())
    key_to_show_index = [True] * len(keys[0])
    columns_to_show = {}

    ind = 0
    for erd, erd_columns in groups_dict.items():
        for col in erd_columns:
            if col not in discard_cols:
                if erd not in columns_to_show:
                    columns_to_show[erd] = []
                columns_to_show[erd].append(col)
            else:
                key_to_show_index[ind] = False
            ind += 1

    for ind, key in enumerate(keys):
        key_to_show = tuple([key[i] for i in range(len(key)) if key_to_show_index[i]])
        x_labels[key_to_show] = get_x_label(
            key_to_show,
            columns_to_show,
            num_query_intervals=num_query_intervals,
            padding=sep * padding_rows if ind % 2 == 0 else "",
            ignore_cols=ignore_cols,
            include_cols=include_cols,
            sep=sep,
            max_line_length=max_line_length,
        )
    return x_labels


def get_y_label(targets: list[tuple[ERD, Column, Reducer]]) -> str:
    col = targets[0][1]
    reducer_str = ""
    match targets[0][2]:
        case MeanReducer():
            reducer_str = "mean"
        case StdReducer():
            reducer_str = "std"
        case MinReducer():
            reducer_str = "min"
        case MaxReducer():
            reducer_str = "max"

    y_label = Y_LABELS.get(col, str(col))
    y_label = y_label[0].lower() + y_label[1:]
    return f"{reducer_str.capitalize()} {y_label}"
