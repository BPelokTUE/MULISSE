# %%
import os

if True:
    while not os.getcwd().endswith("MULISSE"):
        os.chdir("..")

from matplotlib import pyplot as plt
from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import IndexSettingsColumn as ISC
from scripts.py.common.columns import ParamEstimatesColumn as PEC
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import QuerySetSettingsColumn as QSC
from scripts.py.common.columns import SearchSettingsColumn as SSC
from scripts.py.visualization.predict_runtime import visualize_clusters
from scripts.py.visualization.reduction import ERD
from scripts.py.visualization.style import CATEGORY_COLORS, PALETTE
from scripts.py.visualization.wrapper import TargetArgs, visualize_experiments

IMPORTANT_METRICS = [
    TargetArgs.QUERY_TIME,
    TargetArgs.PRUNING_RATIO,
    TargetArgs.INDEX_SIZE,
]

FIGURE_DIR = os.path.join("..", "MasterThesis", "figures")
PARAMETRIZATION_FIGS_DIR = os.path.join(FIGURE_DIR, "parametrization")
EXTENSIONS_FIGS_DIR = os.path.join(FIGURE_DIR, "extensions")
EXPERIMENT_FIGS_DIR = os.path.join(FIGURE_DIR, "experiments")

SAVE_FIGURES = True

if SAVE_FIGURES:
    os.makedirs(PARAMETRIZATION_FIGS_DIR, exist_ok=True)
    os.makedirs(EXPERIMENT_FIGS_DIR, exist_ok=True)

# Set default font sizes
plt.rcParams.update(
    {
        "font.size": 18,
        "axes.titlesize": 18,
        "axes.labelsize": 16,
        "xtick.labelsize": 14,
        "ytick.labelsize": 14,
        "legend.fontsize": 14,
    }
)

dim_suffix_to_titles = {"uni": "Univariate", "multi": "Multivariate"}

dataset_line_colors = {
    ("synthetic",): PALETTE["Purples"][3],
    ("stocks",): PALETTE["Oranges"][3],
    ("weather",): PALETTE["Greens"][3],
}

# %%
# 1 - ULISSE stage comparison

for dim_suffix, title in dim_suffix_to_titles.items():
    for target_args in IMPORTANT_METRICS:
        visualize_experiments(
            logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_1_mulisse_stages_{dim_suffix}"],
            groups_dict={
                ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
                ERD.DATASETS_COLS: [
                    DSC.DATASET_FILE,
                    DSC.NUM_CHANNELS,
                    DSC.SERIES_LENGTH,
                ],
                # ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            },
            separate_plots_dict={
                (DSC.DATASET_FILE,): [],
                (SSC.NORMALIZED,): [],
                **({(DSC.NUM_CHANNELS,): []} if dim_suffix == "multi" else {}),
            },
            ignored_attrs={DSC.NUM_CHANNELS},
            title_base=title,
            legend_max_cols=2,
            legend_plots_dict={DSC.DATASET_FILE: {"synthetic"}} if target_args.name == "QUERY_TIME" else None,
            merge_csv_datasets=True,
            bar_plot_label_padding=False,
            bar_plot_label_map={
                "isax_envelope-ed-early": "First phase",
                "isax_env_w_sax_env-ed-early": "Both phases",
                "sax_envelope-ed-early": "Second phase",
            },
            save_dir=os.path.join(
                PARAMETRIZATION_FIGS_DIR,
                f"1_stages_{target_args.name.lower()}_{dim_suffix}",
            ),
            **target_args.value,
        )

# %%
# 2 - SAX vs no SAX envelope
for dim_suffix, title in dim_suffix_to_titles.items():
    for target_args in IMPORTANT_METRICS:
        visualize_experiments(
            logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_2_sax_vs_no_sax_{dim_suffix}"],
            groups_dict={
                ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
                ERD.DATASETS_COLS: [
                    DSC.DATASET_FILE,
                    DSC.NUM_CHANNELS,
                    DSC.SERIES_LENGTH,
                ],
                # ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            },
            separate_plots_dict={
                (DSC.DATASET_FILE,): [],
                (SSC.NORMALIZED,): [],
                **({(DSC.NUM_CHANNELS,): []} if dim_suffix == "multi" else {}),
            },
            title_base=title,
            merge_csv_datasets=True,
            legend_max_cols=2,
            legend_plots_dict={DSC.DATASET_FILE: {"synthetic"}} if target_args.name == "QUERY_TIME" else None,
            bar_plot_color_attrs=SSC.METHOD_NAME,
            bar_plot_label_padding=False,
            bar_plot_label_map={
                "sax_envelope-ed-early": "MT-Env",
                "envelope-ed-early": "MT-Env no SAX",
            },
            save_dir=os.path.join(
                PARAMETRIZATION_FIGS_DIR,
                f"2_sax_vs_no_sax_{target_args.name.lower()}_{dim_suffix}",
            ),
            **target_args.value,
        )

# %%
# 2b - Stocks univariate raw with SAX vs no SAX envelope
for target_args in [TargetArgs.PRUNING_RATIO]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_2_sax_vs_no_sax_uni"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.NUM_CHANNELS, DSC.SERIES_LENGTH],
        },
        separate_plots_dict={(DSC.DATASET_FILE,): [], (SSC.NORMALIZED,): [(False,)]},
        regex_dict={DSC.DATASET_FILE: r"stocks"},
        title_base=dim_suffix_to_titles["uni"],
        merge_csv_datasets=False,
        legend_max_cols=2,
        legend_plots_dict={DSC.DATASET_FILE: {"stocks/second_clean.csv"}},
        bar_plot_color_attrs=SSC.METHOD_NAME,
        bar_plot_label_padding=False,
        bar_plot_label_map={
            "sax_envelope-ed-early": "MT-Env",
            "envelope-ed-early": "MT-Env no SAX",
        },
        save_dir=os.path.join(
            PARAMETRIZATION_FIGS_DIR,
            f"2_sax_vs_no_sax_{target_args.name.lower()}_unmerged",
        ),
        **target_args.value,
    )

# %%
# 3- iSAX vs MT-Env
for target_args in IMPORTANT_METRICS:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_3_isax_vs_envelope"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.NUM_CHANNELS],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            ERD.INDEXES_COLS: [ISC.NUM_BITS_LIMIT, ISC.NUM_ENVELOPES],
        },
        separate_plots_dict={(DSC.DATASET_FILE, DSC.NUM_CHANNELS): []},
        bar_plot_color_attrs=[SSC.METHOD_NAME, ISC.NUM_BITS_LIMIT],
        legend_max_cols=2,
        legend_plots_dict={DSC.NUM_CHANNELS: {3}} if target_args.name == "PRUNING_RATIO" else None,
        bar_plot_label_map={
            ("isax-ed-early", 1): "iSAX, Blim=1",
            ("isax-ed-early", 2): "iSAX, Blim=2",
            ("isax-ed-early", 4): "iSAX, Blim=4",
            ("sax_envelope-ed-early", 15): "MT-Env",
        },
        bar_plot_color_map={
            ("isax-ed-early", 1): PALETTE["Reds"][0],
            ("isax-ed-early", 2): PALETTE["Reds"][1],
            ("isax-ed-early", 4): PALETTE["Reds"][2],
            ("sax_envelope-ed-early", 15): PALETTE["Blues"][6],
        },
        ignored_attrs={DSC.DATASET_FILE, ISC.NUM_ENVELOPES},
        bar_plot_label_padding=False,
        regex_dict={ISC.NUM_ENVELOPES: r"^(0|1)(\.0){0,1}$"},
        y_scale="log" if target_args.name != "PRUNING_RATIO" else "linear",
        save_dir=os.path.join(PARAMETRIZATION_FIGS_DIR, f"3_mt-env_vs_isax_{target_args.name.lower()}"),
        **target_args.value,
    )

# %%
# 4a - Low resolution grid search

for normalized in [False, True]:
    for dim_suffix, title in dim_suffix_to_titles.items():
        for target_args in [TargetArgs.QUERY_TIME]:
            visualize_experiments(
                logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_4a_low_res_{dim_suffix}"],
                groups_dict={
                    ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
                    ERD.DATASETS_COLS: [DSC.SERIES_LENGTH],
                    ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
                    ERD.INDEXES_COLS: [
                        ISC.NUM_SEGMENTS,
                        ISC.NUM_ENVELOPES,
                        ISC.NUM_LEN_GROUPS,
                        ISC.SEGMENTATION_STRATEGY,
                    ],
                },
                merge_csv_datasets=True,
                separate_plots_dict={
                    **({(DSC.SERIES_LENGTH,): []} if normalized else {}),
                    (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [(0.125, 1.0)],
                    (SSC.NORMALIZED,): [(normalized,)],
                    (ISC.SEGMENTATION_STRATEGY,): [],
                },
                title_base=title,
                bar_plot_color_attrs=None,
                cell_height_inches=4,
                heat_map_x_attr=ISC.NUM_ENVELOPES,
                heat_map_y_attr=ISC.NUM_SEGMENTS,
                heat_map_included_cols={ISC.NUM_LEN_GROUPS} if normalized else {DSC.SERIES_LENGTH},
                save_dir=os.path.join(
                    PARAMETRIZATION_FIGS_DIR,
                    f"4a_low_res_{target_args.name.lower()}_{dim_suffix}",
                ),
                **target_args.value,
            )

# %%
# 4a - Low resolution grid search comparison of query ranges

for normalized in [False, True]:
    for dim_suffix, title in dim_suffix_to_titles.items():
        for target_args in [TargetArgs.QUERY_TIME]:
            visualize_experiments(
                logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_4a_low_res_{dim_suffix}"],
                groups_dict={
                    ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
                    ERD.DATASETS_COLS: [DSC.SERIES_LENGTH],
                    ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
                    ERD.INDEXES_COLS: [
                        ISC.NUM_SEGMENTS,
                        ISC.NUM_ENVELOPES,
                        ISC.NUM_LEN_GROUPS,
                        ISC.SEGMENTATION_STRATEGY,
                    ],
                },
                merge_csv_datasets=True,
                separate_plots_dict={
                    (DSC.SERIES_LENGTH,): [],
                    **(
                        {
                            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [
                                (0.5, 1.0),
                                (0.125, 0.75),
                            ]
                        }
                        if normalized
                        else {}
                    ),
                    (SSC.NORMALIZED,): [(normalized,)],
                    (ISC.SEGMENTATION_STRATEGY,): [],
                },
                title_base=title,
                bar_plot_color_attrs=None,
                cell_height_inches=4,
                heat_map_x_attr=ISC.NUM_ENVELOPES,
                heat_map_y_attr=ISC.NUM_SEGMENTS,
                heat_map_included_cols={ISC.NUM_LEN_GROUPS} if normalized else {QSC.L_MIN_RATIO, QSC.L_MAX_RATIO},
                save_dir=os.path.join(
                    PARAMETRIZATION_FIGS_DIR,
                    f"4a_low_res_{target_args.name.lower()}_{dim_suffix}",
                ),
                **target_args.value,
            )

# %%
# 4b - Number of lengths per LG (λ) univariate

dim_suffix = "multi"

for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_4b_length_group_{dim_suffix}"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.NORMALIZED],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [ISC.NUM_LEN_GROUPS, ISC.SEGMENTATION_STRATEGY],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (SSC.NORMALIZED,): [],
            (ISC.SEGMENTATION_STRATEGY,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [(0.125, 1.0)],
        },
        # title_base=dim_suffix_to_titles[dim_suffix],
        legend_plots_dict={DSC.SERIES_LENGTH: {2048}, SSC.NORMALIZED: {True}, ISC.SEGMENTATION_STRATEGY: {"uniform"}},
        bar_plot_color_attrs=None,
        line_plot_x_attr=ISC.NUM_LEN_GROUPS,
        line_plot_included_cols={DSC.DATASET_FILE},
        line_plot_colors_map=dataset_line_colors,
        x_scale="log",
        save_dir=os.path.join(PARAMETRIZATION_FIGS_DIR, f"4b_Nl_{target_args.name.lower()}_{dim_suffix}"),
        **target_args.value,
    )

# %%
# 4c - Positions per envelope (γ) univariate

dim_suffix = "multi"

for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_4c_ppe_{dim_suffix}"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.NORMALIZED],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [ISC.POS_PER_ENV, ISC.SEGMENTATION_STRATEGY],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (SSC.NORMALIZED,): [],
            (ISC.SEGMENTATION_STRATEGY,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [(0.125, 1.0)],
        },
        # title_base=dim_suffix_to_titles[dim_suffix],
        legend_plots_dict={DSC.SERIES_LENGTH: {2048}, SSC.NORMALIZED: {True}, ISC.SEGMENTATION_STRATEGY: {"uniform"}},
        bar_plot_color_attrs=None,
        line_plot_x_attr=ISC.POS_PER_ENV,
        line_plot_included_cols={DSC.DATASET_FILE},
        line_plot_colors_map=dataset_line_colors,
        x_scale="log",
        save_dir=os.path.join(PARAMETRIZATION_FIGS_DIR, f"4c_ppe_{target_args.name.lower()}_{dim_suffix}"),
        **target_args.value,
    )

# %%
# 4d - Segment size (s) univariate

dim_suffix = "uni"

for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_4d_num_segments_{dim_suffix}"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [ISC.NUM_SEGMENTS, ISC.SEGMENTATION_STRATEGY],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (SSC.NORMALIZED,): [],
            (ISC.SEGMENTATION_STRATEGY,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
        },
        title_base=dim_suffix_to_titles[dim_suffix],
        # no_legend=True,
        legend_plots_dict={DSC.SERIES_LENGTH: {2048}, ISC.SEGMENTATION_STRATEGY: {"uniform"}, QSC.L_MIN_RATIO: {0.125}},
        bar_plot_color_attrs=None,
        line_plot_x_attr=ISC.NUM_SEGMENTS,
        line_plot_included_cols={DSC.DATASET_FILE},
        save_dir=os.path.join(PARAMETRIZATION_FIGS_DIR, f"4d_Ns_{target_args.name.lower()}_{dim_suffix}"),
        **target_args.value,
    )

# %%
# 4e - Showing linear relationship between runtime and dataset size

dim_suffix = "uni"

for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_4e_num_series_{dim_suffix}"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.NORMALIZED],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH, DSC.NUM_SERIES],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (SSC.NORMALIZED,): [],
            (ISC.SEGMENTATION_STRATEGY,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [(0.125, 1.0)],
        },
        # title_base=dim_suffix_to_titles[dim_suffix],
        legend_plots_dict={DSC.SERIES_LENGTH: {2048}, SSC.NORMALIZED: {True}, ISC.SEGMENTATION_STRATEGY: {"uniform"}},
        bar_plot_color_attrs=None,
        line_plot_x_attr=DSC.NUM_SERIES,
        line_plot_included_cols={DSC.DATASET_FILE},
        line_plot_colors_map=dataset_line_colors,
        save_dir=os.path.join(
            PARAMETRIZATION_FIGS_DIR,
            f"4e_num_series_{target_args.name.lower()}_{dim_suffix}",
        ),
        **target_args.value,
    )

# %%
# 5a - PPE for different distance measures
re_sep = "::"

for suffix in ["", "_raw"]:
    for target_args in [TargetArgs.QUERY_TIME, TargetArgs.ABANDONING_RATE]:
        visualize_experiments(
            logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_5a_mt_env_w_mass{suffix}"],
            groups_dict={
                ERD.METHODS_COLS: [
                    SSC.DISTANCE_MEASURE,
                    SSC.EXAMINE_WHOLE,
                    SSC.PRECOMPUTED_FFTS,
                    SSC.SORT_QUERY,
                    SSC.NORMALIZED,
                ],
                ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
                ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
                ERD.INDEXES_COLS: [ISC.POS_PER_ENV],
            },
            merge_csv_datasets=True,
            separate_plots_dict={
                (DSC.SERIES_LENGTH, DSC.DATASET_FILE): [],
                (SSC.NORMALIZED,): [],
                (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
            },
            regex_dict={SSC.DISTANCE_MEASURE: r"^(?!mass)"} if target_args.name == "ABANDONING_RATE" else {},
            bar_plot_color_attrs=None,
            line_plot_x_attr=ISC.POS_PER_ENV,
            line_plot_included_cols={
                SSC.DISTANCE_MEASURE,
                SSC.EXAMINE_WHOLE,
                SSC.PRECOMPUTED_FFTS,
                SSC.SORT_QUERY,
            },
            line_plot_legend_map={
                ("ed", 0, False, False): "EA",
                ("ed", 1, False, False): "EA Whole",
                ("ed", 0, False, True): "EA Sorted",
                ("ed", 1, False, True): "EA Whole Sorted",
                ("mass", 0, False, False): "MASS",
                ("mass", 1, False, False): "MASS Whole",
                ("mass", 0, True, False): "MASS Precomputed",
            },
            line_plot_colors_map={
                ("ed", 0, False, False): PALETTE["Blues"][6],
                ("ed", 1, False, False): PALETTE["Blues"][2],
                ("ed", 0, False, True): PALETTE["Greens"][6],
                ("ed", 1, False, True): PALETTE["Greens"][2],
                ("mass", 0, False, False): PALETTE["Purples"][6],
                ("mass", 1, False, False): PALETTE["Purples"][3],
                ("mass", 0, True, False): PALETTE["Oranges"][4],
            },
            legend_max_cols=3,
            x_scale="log",
            y_scale="linear",
            save_dir=os.path.join(
                PARAMETRIZATION_FIGS_DIR,
                f"5a_mt_env_w_mass{dim_suffix}_{target_args.name.lower()}",
            ),
            **target_args.value,
        )


# %%
# 5b - Presence

sizes = {
    "small": [8, 6, 4, 4],
    "medium": [12, 9, 7, 7],
    "large": [16, 12, 10, 10],
}
strategies = [
    ("uniform", "single"),
    ("adaptive", "single"),
    ("uniform", "adaptive_multi"),
    ("adaptive", "adaptive_multi"),
]
re_sep = "::"
size_regexes: dict[str, str] = {}

for size_key, size_values in sizes.items():
    act_regexes = []
    for (ss, lgss), size in zip(strategies, size_values):
        act_regexes.append(re_sep.join([ss, lgss, str(size)]))
    size_regexes[size_key] = rf"^({'|'.join(act_regexes)})"

for target_args in [TargetArgs.INDEX_SIZE, TargetArgs.QUERY_TIME]:
    for size, regex in size_regexes.items():
        visualize_experiments(
            logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_5b_presence"],
            # logs_dirs=["LOGS"],
            groups_dict={
                ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
                ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
                ERD.INDEXES_COLS: [
                    ISC.NUM_SEGMENTS,
                    ISC.SEGMENTATION_STRATEGY,
                    ISC.LG_SEGMENTATION_STRATEGY,
                ],
            },
            regex_dict={
                (
                    ISC.SEGMENTATION_STRATEGY,
                    ISC.LG_SEGMENTATION_STRATEGY,
                    ISC.NUM_SEGMENTS,
                ): regex
            },
            regex_sep=re_sep,
            separate_plots_dict={
                (DSC.DATASET_FILE,): [],
                (DSC.SERIES_LENGTH,): [],
            },
            bar_plot_color_attrs=[
                ISC.SEGMENTATION_STRATEGY,
                ISC.LG_SEGMENTATION_STRATEGY,
            ],
            bar_plot_label_map={
                ("uniform", "single"): "Simple",
                ("adaptive", "single"): "Adaptive Segmentation",
                ("uniform", "adaptive_multi"): "Adaptive LG",
                ("adaptive", "adaptive_multi"): "Adaptive LG + Segmentation",
            },
            bar_plot_color_map={strat: color for strat, color in zip(strategies, CATEGORY_COLORS)},
            legend_max_cols=2,
            bar_plot_label_padding=False,
            title_base=f"{size.capitalize()} Ns",
            ignored_attrs={DSC.DATASET_FILE, ISC.NUM_SEGMENTS, DSC.SERIES_LENGTH},
            save_dir=os.path.join(EXTENSIONS_FIGS_DIR, f"5_presence_{size}_{target_args.name.lower()}"),
            **target_args.value,
        )

# %%
# 6c - Segment size (s) univariate for Envelope Tree

for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_6a_tree_vs_flat"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [
                ISC.NUM_SEGMENTS,
                ISC.GROUP_PER_SERIES,
                ISC.SEGMENTATION_STRATEGY,
            ],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [(0.125, 1.0)],
        },
        bar_plot_color_attrs=None,
        line_plot_x_attr=ISC.NUM_SEGMENTS,
        line_plot_included_cols={
            DSC.DATASET_FILE,
            SSC.METHOD_NAME,
            ISC.GROUP_PER_SERIES,
            ISC.SEGMENTATION_STRATEGY,
        },
        # save_dir=os.path.join(PARAMETRIZATION_FIGS_DIR, f"4d_Ns_{target_args.name.lower()}"),
        **target_args.value,
    )


# %%
# 7a - Channel performance

# Use the dataset as the legend for the CSV datasets
# Use the SD as the legend for the synthetic datasets

label_maps = {
    "synthetic": {step: f"Step={step:.1f}" for step in [0.1, 1.0, 10.0]},
    "stocks": {
        "stocks/first_clean.csv": "Open",
        "stocks/second_clean.csv": "Close",
        "stocks/third_clean.csv": "Median",
        "stocks/fourth_clean.csv": "Mean",
        "stocks/fifth_clean_fixed.csv": "Volume",
    },
    "weather": {
        "weather/DEW": "Humidity",
        "weather/SLP": "Pressure",
        "weather/WND": "Wind",
        "weather/TMP": "Temperature",
    },
}

dataset_cols_map = {
    "synthetic": [DSC.DATASET_FILE, DSC.SD],
    "stocks": [DSC.DATASET_FILE],
    "weather": [DSC.DATASET_FILE],
}

for target_args in [TargetArgs.QUERY_TIME]:
    for dataset, dataset_cols in dataset_cols_map.items():
        visualize_experiments(
            logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_6_ch_performance"],
            groups_dict={ERD.DATASETS_COLS: dataset_cols},
            regex_dict={DSC.DATASET_FILE: rf"^{dataset}"},
            bar_plot_color_attrs=dataset_cols[-1],
            legend_max_cols=2,
            bar_plot_label_map=label_maps[dataset],
            save_dir=os.path.join(
                EXTENSIONS_FIGS_DIR,
                f"6_ch_performance_{target_args.name.lower()}_{dataset}",
            ),
            **target_args.value,
        )

# %%
# 7b - Channel clustering
visualize_clusters(
    logs_dir="EXPERIMENT_LOGS/thesis/LOGS_6_ch_clustering",
    save_dir=os.path.join(EXTENSIONS_FIGS_DIR, "6_ch_clustering"),
)

# %%
# 7c - Channel prioritization
for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_6_ch_prioritization"],
        groups_dict={
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.INDEXES_COLS: [
                ISC.NUM_SEGMENTS,
                ISC.CH_SEGMENTATION_STRATEGY,
                ISC.SCORE_BASED_CHSS_SCORE_EXP,
            ],
        },
        separate_plots_dict={(DSC.DATASET_FILE, ISC.NUM_SEGMENTS): []},
        bar_plot_color_attrs=[
            ISC.CH_SEGMENTATION_STRATEGY,
            ISC.SCORE_BASED_CHSS_SCORE_EXP,
        ],
        legend_max_cols=2,
        bar_plot_label_map={
            ("single", 0): "Simple",
            ("score_based", -1): "Prioritize Easy",
            ("score_based", 1): "Prioritize Hard",
        },
        save_dir=os.path.join(EXTENSIONS_FIGS_DIR, f"6_ch_prioritization_{target_args.name.lower()}"),
        **target_args.value,
    )

# %%
# 8 - Envelope merging

for target_args_dict in [
    TargetArgs.QUERY_TIME.value,
    # TargetArgs.MINDIST_TIME.value,
    # TargetArgs.PRUNING_RATIO.value,
    # TargetArgs.INDEX_SIZE.value,
    {"targets_dict": {ERD.RUNS_COLS: [QC.TS_EXAMINATION_TIME_S]}},
]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_7_vl_envelope"],
        # logs_dirs=["LOGS"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.INDEXES_COLS: [
                ISC.USE_INV_SAX,
                ISC.POS_PER_ENV,
                ISC.MAX_WIDTH_CHANGE,
            ],
        },
        separate_plots_dict={(DSC.DATASET_FILE,): []},
        bar_plot_color_attrs=SSC.METHOD_NAME,
        legend_max_cols=2,
        bar_plot_label_map={
            "bucketing_envelope-ed-early": "Bucket merger",
            "envelope-ed-early": "No merger",
            "vl_envelope-ed-early": "Variance-limiting merger",
        },
        **target_args_dict,
    )

# %%
# 9a - Method comparison

method_comparison_labels = {
    ("sax_envelope-ed-early", "uniform"): "MT-Env EqW",
    ("sax_envelope-ed-early", "adaptive"): "MT-Env EqD",
    ("envelope-ed-early", "uniform"): "MT-Env",
    ("sequential_scan-ed-early", 0): "UCR Suite",
    ("sequential_scan-mass", 0): "MASS",
}
method_comparison_colors = {
    ("sax_envelope-ed-early", "uniform"): PALETTE["Blues"][6],
    ("sax_envelope-ed-early", "adaptive"): PALETTE["Blues"][2],
    ("envelope-ed-early", "uniform"): PALETTE["Blues"][6],
    ("sequential_scan-ed-early", 0): PALETTE["Greens"][6],
    ("sequential_scan-mass", 0): PALETTE["Purples"][6],
}

for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_9a_method_comparison"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY],
        },
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (SSC.NORMALIZED,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
        },
        legend_max_cols=2,
        bar_plot_label_padding=False,
        bar_plot_color_attrs=[SSC.METHOD_NAME, ISC.SEGMENTATION_STRATEGY],
        bar_plot_label_map=method_comparison_labels,
        bar_plot_color_map=method_comparison_colors,
        y_scale="log",
        merge_csv_datasets=True,
        **target_args.value,
    )

for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_9a_method_comparison"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
            ERD.DATASETS_COLS: [DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO],
            ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY],
        },
        separate_plots_dict={(DSC.SERIES_LENGTH,): [], (SSC.NORMALIZED,): []},
        legend_max_cols=2,
        bar_plot_color_attrs=None,
        line_plot_x_attr=QSC.L_MIN_RATIO,
        line_plot_show_min=False,
        line_plot_included_cols={SSC.METHOD_NAME, ISC.SEGMENTATION_STRATEGY},
        line_plot_legend_map=method_comparison_labels,
        line_plot_colors_map=method_comparison_colors,
        y_scale="log",
        merge_csv_datasets=True,
        **target_args.value,
    )


# %%
# 10b - Size limiting

for target_args in [TargetArgs.ESTIMATE_SCORE]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_9a_size_limiting_baselines"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [
                ISC.PARAM_ESTIMATOR_TYPE,
                ISC.INDEX_SIZE_LIMIT,
                ISC.SEGMENTATION_STRATEGY,
            ],
            ERD.PARAM_ESTIMATES_COLS: [PEC.NUM_LEN_GROUPS, PEC.NUM_SEGMENTS],
        },
        separate_plots_dict={
            (SSC.METHOD_NAME,): [],
            (ISC.INDEX_SIZE_LIMIT,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
            (ISC.PARAM_ESTIMATOR_TYPE,): [("min_dist",), ("query_time",)],
        },
        bar_plot_color_attrs=None,
        merge_csv_datasets=True,
        heat_map_x_attr=PEC.NUM_LEN_GROUPS,
        heat_map_y_attr=PEC.NUM_SEGMENTS,
        heat_map_included_cols={DSC.DATASET_FILE, ISC.SEGMENTATION_STRATEGY},
        cell_width_inches=7.5,
        **target_args.value,
    )

for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_9a_size_limiting_baselines"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [
                ISC.INDEX_SIZE_LIMIT,
                ISC.NUM_LEN_GROUPS,
                ISC.NUM_SEGMENTS,
                ISC.PARAM_ESTIMATOR_TYPE,
            ],
        },
        separate_plots_dict={
            (SSC.METHOD_NAME,): [],
            (ISC.INDEX_SIZE_LIMIT,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
            (ISC.PARAM_ESTIMATOR_TYPE,): [("no_est",)],
        },
        bar_plot_color_attrs=None,
        merge_csv_datasets=True,
        heat_map_x_attr=ISC.NUM_LEN_GROUPS,
        heat_map_y_attr=ISC.NUM_SEGMENTS,
        heat_map_included_cols={DSC.DATASET_FILE, ISC.SEGMENTATION_STRATEGY},
        cell_width_inches=7.5,
        **target_args.value,
    )
