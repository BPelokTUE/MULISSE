# %%
import itertools
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
from scripts.py.visualization.reduction import ERD, MeanReducer, RobustMeanReducer
from scripts.py.visualization.style import PALETTE
from scripts.py.visualization.time_series import visualize_time_series
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
        "xtick.labelsize": 16,
        "ytick.labelsize": 16,
        "legend.fontsize": 16,
    }
)

dim_suffix_to_titles = {"uni": "Univariate", "multi": "Multivariate"}

dataset_line_colors = {
    ("synthetic",): PALETTE["Purples"][3],
    ("stocks",): PALETTE["Oranges"][3],
    ("weather",): PALETTE["Greens"][3],
}

method_comparison_labels = {
    ("sax_envelope-ed-early", "uniform"): "MT-Env EqW",
    ("sax_envelope-ed-early", "adaptive"): "MT-Env EqD",
    ("sax_envelope-mass-ffts", "uniform"): "MT-Env EqW MASS",
    ("sax_envelope-mass-ffts", "adaptive"): "MT-Env EqD MASS",
    ("sax_envelope-mass", "uniform"): "MT-Env EqW MASS",
    ("sax_envelope-mass", "adaptive"): "MT-Env EqD MASS",
    ("envelope-ed-early", "uniform"): "MT-Env",
    ("envelope-mass-ffts", "uniform"): "MT-Env MASS",
    ("isax_env_w_sax_env-ed-early", "uniform"): "MULISSE",
    ("sequential_scan-ed-early", 0): "UCR Suite",
    ("sequential_scan-mass", 0): "MASS",
    ("sequential_scan-mass-ffts", 0): "MASS Prec.",
    "Search time": "Search time",
    "Prep. time": "Prep. time",
}
method_comparison_colors = {
    ("sax_envelope-ed-early", "uniform"): PALETTE["Blues"][6],
    ("sax_envelope-ed-early", "adaptive"): PALETTE["Blues"][2],
    ("sax_envelope-mass-ffts", "uniform"): PALETTE["Oranges"][5],
    ("sax_envelope-mass-ffts", "adaptive"): PALETTE["Oranges"][2],
    ("sax_envelope-mass", "uniform"): PALETTE["Oranges"][5],
    ("sax_envelope-mass", "adaptive"): PALETTE["Oranges"][2],
    ("envelope-ed-early", "uniform"): PALETTE["Blues"][6],
    ("envelope-mass-ffts", "uniform"): PALETTE["Oranges"][5],
    ("isax_env_w_sax_env-ed-early", "uniform"): PALETTE["Greens"][2],
    ("sequential_scan-ed-early", 0): PALETTE["Greys"][2],
    ("sequential_scan-mass", 0): PALETTE["Purples"][6],
    ("sequential_scan-mass-ffts", 0): PALETTE["Purples"][2],
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
            legend_plots_dict={SSC.NORMALIZED: {True}}
            if target_args.name == "PRUNING_RATIO" and dim_suffix == "multi"
            else None,
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
                    # DSC.NUM_CHANNELS,
                    DSC.SERIES_LENGTH,
                ],
                # ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            },
            separate_plots_dict={
                (DSC.DATASET_FILE,): [],
                (SSC.NORMALIZED,): [],
                # **({(DSC.NUM_CHANNELS,): []} if dim_suffix == "multi" else {}),
            },
            title_base=title,
            merge_csv_datasets=True,
            fig_height_inches=3,
            legend_max_cols=2,
            legend_plots_dict={SSC.NORMALIZED: {True}}
            if target_args.name == "PRUNING_RATIO" and dim_suffix == "multi"
            else None,
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
        fig_height_inches=3,
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
                    (DSC.SERIES_LENGTH,): [],
                    **({(QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): []} if normalized else {}),
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
# 4a - Low resolution grid for specific raw figure

dim_suffix = "multi"
re_sep = "::"

visualize_experiments(
    logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_4a_low_res_{dim_suffix}"],
    groups_dict={
        ERD.METHODS_COLS: [SSC.NORMALIZED],
        ERD.DATASETS_COLS: [DSC.SERIES_LENGTH],
        ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
        ERD.INDEXES_COLS: [ISC.NUM_SEGMENTS, ISC.NUM_ENVELOPES, ISC.NUM_LEN_GROUPS],
    },
    separate_plots_dict={(SSC.NORMALIZED,): [(False,)]},
    regex_dict={
        (
            DSC.SERIES_LENGTH,
            QSC.L_MIN_RATIO,
            QSC.L_MAX_RATIO,
        ): rf"(2048{re_sep}0.125{re_sep}1.0|2048{re_sep}0.5{re_sep}1.0|3072{re_sep}0.125{re_sep}1.0)"
    },
    regex_sep=re_sep,
    title_base=dim_suffix_to_titles[dim_suffix],
    bar_plot_color_attrs=None,
    cell_height_inches=4,
    heat_map_x_attr=ISC.NUM_ENVELOPES,
    heat_map_y_attr=ISC.NUM_SEGMENTS,
    heat_map_included_cols={DSC.SERIES_LENGTH, QSC.L_MIN_RATIO, QSC.L_MAX_RATIO},
    save_dir=os.path.join(
        PARAMETRIZATION_FIGS_DIR,
        f"4a_low_res_{target_args.name.lower()}_{dim_suffix}_pretty",
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
# 4b - Number of lengths per LG (λ)

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
# 4c - Positions per envelope (γ)

for dim_suffix in ["multi", "lmin"]:
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
                (DSC.SERIES_LENGTH,): [(1024,), (2048,)],
                (SSC.NORMALIZED,): [],
                (ISC.SEGMENTATION_STRATEGY,): [],
                (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
            },
            # title_base=dim_suffix_to_titles["multi"],
            legend_plots_dict={
                DSC.SERIES_LENGTH: {2048},
                SSC.NORMALIZED: {True},
                ISC.SEGMENTATION_STRATEGY: {"uniform"},
            },
            bar_plot_color_attrs=None,
            line_plot_x_attr=ISC.POS_PER_ENV,
            line_plot_included_cols={DSC.DATASET_FILE},
            line_plot_colors_map=dataset_line_colors,
            x_scale="log",
            save_dir=os.path.join(PARAMETRIZATION_FIGS_DIR, f"4c_ppe_{target_args.name.lower()}_{dim_suffix}"),
            **target_args.value,
        )

# %%
# 4c - PPE for different distance measures
re_sep = "::"

dist_calc_labels = {
    ("ed", 0, False, False): "EA",
    ("ed", 1, False, False): "EA Whole",
    ("ed", 0, False, True): "EA Sorted",
    ("ed", 1, False, True): "EA Whole Sorted",
    ("mass", 0, False, False): "MASS",
    ("mass", 1, False, False): "MASS Whole",
    ("mass", 1, True, False): "MASS Precomputed",
}
dist_calc_colors = {
    ("ed", 0, False, False): PALETTE["Blues"][6],
    ("ed", 1, False, False): PALETTE["Blues"][2],
    ("ed", 0, False, True): PALETTE["Greens"][6],
    ("ed", 1, False, True): PALETTE["Greens"][2],
    ("mass", 0, False, False): PALETTE["Purples"][6],
    ("mass", 1, False, False): PALETTE["Purples"][3],
    ("mass", 1, True, False): PALETTE["Oranges"][4],
}

regexes = {"mass": r"", "no_mass": r"(ed.*|mass::1.*)", "abr": r"^(?!mass).*"}

for suffix in ["ds", "m", "lmin"]:
    for target_args in [TargetArgs.QUERY_TIME, TargetArgs.ABANDONING_RATE, TargetArgs.PRUNING_RATIO]:
        for regex_key, regex in regexes.items():
            abr_target = target_args.name == "ABANDONING_RATE"
            abr_key = regex_key == "abr"
            if (abr_target and not abr_key) or (not abr_target and abr_key):
                continue

            if target_args.name == "PRUNING_RATIO" and regex_key != "mass":
                continue

            visualize_experiments(
                logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_4c_ppe_dist_calc_methods_{suffix}"],
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
                regex_dict={(SSC.DISTANCE_MEASURE, SSC.EXAMINE_WHOLE, SSC.PRECOMPUTED_FFTS): regex},
                bar_plot_color_attrs=None,
                line_plot_x_attr=ISC.POS_PER_ENV,
                line_plot_included_cols={SSC.DISTANCE_MEASURE, SSC.EXAMINE_WHOLE, SSC.PRECOMPUTED_FFTS, SSC.SORT_QUERY},
                line_plot_legend_map=dist_calc_labels,
                line_plot_colors_map=dist_calc_colors,
                legend_max_cols=2,
                legend_plots_dict={DSC.DATASET_FILE: {"stocks"}} if target_args.name == "PRUNING_RATIO" else None,
                x_scale="log",
                y_scale="linear",
                save_dir=os.path.join(
                    PARAMETRIZATION_FIGS_DIR,
                    f"4c_ppe_dist_calc_{regex_key}_{suffix}_{target_args.name.lower()}",
                ),
                **target_args.value,
            )


# %%
# 4d - Segment size (s) univariate

dim_suffix = "multi"

for normalized in [False, True]:
    suffix = "_m" if normalized else "_raw"
    # suffix = ""
    for target_args in [TargetArgs.QUERY_TIME]:
        visualize_experiments(
            logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_4d_num_segments_{dim_suffix}{suffix}"],
            groups_dict={
                ERD.METHODS_COLS: [SSC.NORMALIZED],
                ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
                ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
                ERD.INDEXES_COLS: [ISC.NUM_SEGMENTS, ISC.SEGMENTATION_STRATEGY],
            },
            merge_csv_datasets=True,
            separate_plots_dict={
                (DSC.SERIES_LENGTH,): [],
                (SSC.NORMALIZED,): [(normalized,)],
                (ISC.SEGMENTATION_STRATEGY,): [],
                (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
            },
            # title_base=dim_suffix_to_titles[dim_suffix],
            x_scale="log",
            legend_plots_dict={
                DSC.SERIES_LENGTH: {2048},
                ISC.SEGMENTATION_STRATEGY: {"uniform"},
                QSC.L_MIN_RATIO: {0.125},
            },
            bar_plot_color_attrs=None,
            line_plot_x_attr=ISC.NUM_SEGMENTS,
            line_plot_included_cols={DSC.DATASET_FILE},
            line_plot_colors_map=dataset_line_colors,
            save_dir=os.path.join(PARAMETRIZATION_FIGS_DIR, f"4d_Ns_{target_args.name.lower()}_{dim_suffix}"),
            **target_args.value,
        )

# %%
# 4e - Showing linear relationship between runtime and dataset size

dim_suffix = "multi"

for reducer_name, reducer in [
    ("mean", MeanReducer()),
    ("robust_mean", RobustMeanReducer(discard_lower_quantile=0.0, discard_upper_quantile=0.1)),
]:
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
            legend_plots_dict={
                DSC.SERIES_LENGTH: {2048},
                SSC.NORMALIZED: {True},
                ISC.SEGMENTATION_STRATEGY: {"uniform"},
            },
            bar_plot_color_attrs=None,
            line_plot_x_attr=DSC.NUM_SERIES,
            line_plot_included_cols={DSC.DATASET_FILE},
            line_plot_colors_map=dataset_line_colors,
            reducer=reducer,
            save_dir=os.path.join(
                PARAMETRIZATION_FIGS_DIR,
                f"4e_num_series_{target_args.name.lower()}_{reducer_name}_{dim_suffix}",
            ),
            **target_args.value,
        )

# %%
# 5a - Presence

sizes = {
    "small": [8, 6, 4, 4],
    "medium": [12, 9, 7, 7],
    "large": [16, 12, 10, 10],
}
strategy_labels = {
    ("uniform", "single"): "EqW",
    ("adaptive", "single"): "EqD",
    ("uniform", "adaptive_multi"): "PLG + EqW",
    ("adaptive", "adaptive_multi"): "PLG + EqD",
}
strategy_colors = {
    ("uniform", "single"): PALETTE["Blues"][6],
    ("adaptive", "single"): PALETTE["Blues"][2],
    ("uniform", "adaptive_multi"): PALETTE["Yellows"][4],
    ("adaptive", "adaptive_multi"): PALETTE["Yellows"][1],
}

re_sep = "::"
size_regexes: dict[str, str] = {}

for size_key, size_values in sizes.items():
    act_regexes = []
    for (ss, lgss), size in zip(strategy_labels, size_values):
        act_regexes.append(re_sep.join([ss, lgss, str(size)]))
    size_regexes[size_key] = rf"^({'|'.join(act_regexes)})"

versions = ["a", "b"]
sizes = list(size_regexes.keys())
target_args_list = [TargetArgs.INDEX_SIZE, TargetArgs.QUERY_TIME]

# Use itertools.product to create all combinations
for target_args, version, size in itertools.product(target_args_list, versions, sizes):
    visualize_experiments(
        logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_5{version}_presence"],
        groups_dict={
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH, DSC.NUM_CHANNELS],
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
            (DSC.NUM_CHANNELS,): [],
            (DSC.DATASET_FILE,): [],
            (DSC.SERIES_LENGTH,): [],
        },
        bar_plot_color_attrs=[
            ISC.SEGMENTATION_STRATEGY,
            ISC.LG_SEGMENTATION_STRATEGY,
        ],
        bar_plot_label_map=strategy_labels,
        bar_plot_color_map=strategy_colors,
        legend_max_cols=2,
        legend_plots_dict={DSC.SERIES_LENGTH: {1024}}
        if target_args.name == "QUERY_TIME" and size == "small"
        else {DSC.SERIES_LENGTH: {2048}}
        if target_args.name == "QUERY_TIME" and size == "medium"
        else None,
        bar_plot_label_padding=False,
        title_base=f"{size.capitalize()} Ns",
        ignored_attrs={DSC.DATASET_FILE, ISC.NUM_SEGMENTS, DSC.SERIES_LENGTH},
        save_dir=os.path.join(EXTENSIONS_FIGS_DIR, f"5{version}_presence_{size}_{target_args.name.lower()}"),
        **target_args.value,
    )

# %%
# 6a - Segment size (s) univariate for Envelope Tree

for target_args in [TargetArgs.QUERY_TIME, TargetArgs.PRUNING_RATIO]:
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
            (DSC.DATASET_FILE,): [],
            (DSC.SERIES_LENGTH,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [(0.125, 1.0)],
        },
        legend_max_cols=3,
        legend_plots_dict={} if target_args.name == "QUERY_TIME" else None,
        bar_plot_color_attrs=None,
        line_plot_show_min=target_args.name == "QUERY_TIME",
        line_plot_colors_map={
            ("envelope-ed-early", 0, "uniform"): PALETTE["Blues"][6],
            ("envelope-ed-early", 0, "adaptive"): PALETTE["Blues"][2],
            ("tree_envelope-ed-early", 1, "uniform"): PALETTE["Pinks"][5],
            ("tree_envelope-ed-early", 1, "adaptive"): PALETTE["Pinks"][2],
            ("tree_envelope-ed-early", 0, "uniform"): PALETTE["Yellows"][3],
            ("tree_envelope-ed-early", 0, "adaptive"): PALETTE["Yellows"][0],
        },
        line_plot_legend_map={
            ("tree_envelope-ed-early", 1, "uniform"): "TreeEnv EqW",
            ("tree_envelope-ed-early", 1, "adaptive"): "TreeEnv EqD",
            ("tree_envelope-ed-early", 0, "uniform"): "DS TreeEnv EqW",
            ("tree_envelope-ed-early", 0, "adaptive"): "DS TreeEnv EqD",
            ("envelope-ed-early", 0, "uniform"): "MT-Env EqW",
            ("envelope-ed-early", 0, "adaptive"): "MT-Env EqD",
        },
        line_plot_x_attr=ISC.NUM_SEGMENTS,
        line_plot_included_cols={
            SSC.METHOD_NAME,
            ISC.GROUP_PER_SERIES,
            ISC.SEGMENTATION_STRATEGY,
        },
        x_scale="log",
        y_scale="log" if target_args.name == "QUERY_TIME" else "linear",
        save_dir=os.path.join(EXTENSIONS_FIGS_DIR, f"6a_tree_vs_flat_{target_args.name.lower()}"),
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
        "stocks/third_clean.csv": "Low",
        "stocks/fourth_clean.csv": "High",
        "stocks/fifth_clean_fixed.csv": "Volume",
    },
    "weather": {
        "weather/TMP": "Temperature",
        "weather/DEW": "Humidity",
        "weather/SLP": "Pressure",
        "weather/WND": "Wind Speed",
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
            logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_7a_ch_performance"],
            groups_dict={ERD.DATASETS_COLS: dataset_cols},
            regex_dict={DSC.DATASET_FILE: rf"^{dataset}"},
            legend_max_cols=1,
            bar_plot_color_attrs=dataset_cols[-1],
            bar_plot_label_map=label_maps[dataset],
            bar_plot_label_padding=False,
            bar_plot_no_x_ticks=True,
            title_base=dataset.capitalize(),
            save_dir=os.path.join(
                EXTENSIONS_FIGS_DIR,
                f"7a_ch_performance_{target_args.name.lower()}_{dataset}",
            ),
            verbose=True,
            **target_args.value,
        )

# %%
# 7b - Channel clustering
visualize_clusters(
    logs_dir="EXPERIMENT_LOGS/thesis/LOGS_7b_ch_clustering",
    save_dir=os.path.join(EXTENSIONS_FIGS_DIR, "7b_ch_clustering"),
    fig_size=(13, 3.5),
)

# %%
# 7c - Channel prioritization
for target_args in [TargetArgs.QUERY_TIME, TargetArgs.PRUNING_RATIO]:
    visualize_experiments(
        # logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_7c_ch_prioritization"],
        logs_dirs=["LOGS"],
        groups_dict={
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.INDEXES_COLS: [
                ISC.NUM_SEGMENTS,
                ISC.CH_SEGMENTATION_STRATEGY,
                ISC.SCORE_BASED_CHSS_SCORE_EXP,
            ],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO],
        },
        separate_plots_dict={(DSC.DATASET_FILE,): []},
        legend_max_cols=2,
        legend_plots_dict={DSC.DATASET_FILE: {"synthetic"}},
        bar_plot_color_attrs=[
            ISC.CH_SEGMENTATION_STRATEGY,
            ISC.SCORE_BASED_CHSS_SCORE_EXP,
        ],
        bar_plot_label_padding=False,
        bar_plot_label_map={
            ("score_based", -1): "Easy prioritization",
            ("score_based", 1): "Hard prioritization",
            ("single", 0): "Simple",
        },
        bar_plot_color_map={
            ("score_based", -1): PALETTE["Greens"][5],
            ("score_based", 1): PALETTE["Reds"][3],
            ("single", 0): PALETTE["Blues"][6],
        },
        # save_dir=os.path.join(EXTENSIONS_FIGS_DIR, f"7c_ch_prioritization_{target_args.name.lower()}"),
        verbose=True,
        **target_args.value,
    )

# %%
# 8 - Envelope merging

for target_args in [
    TargetArgs.QUERY_TIME,
    # TargetArgs.MINDIST_TIME,
    # TargetArgs.PRUNING_RATIO,
    TargetArgs.INDEX_SIZE,
]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_8_vl_envelope"],
        # logs_dirs=["LOGS"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.INDEXES_COLS: [
                ISC.USE_INV_SAX,
                ISC.MAX_WIDTH_CHANGE,
            ],
        },
        separate_plots_dict={(DSC.DATASET_FILE,): []},
        legend_max_cols=3,
        legend_plots_dict={} if target_args.name == "QUERY_TIME" else None,
        x_ticks_rotation=-15,
        bar_plot_label_padding=False,
        bar_plot_color_attrs=[SSC.METHOD_NAME, ISC.USE_INV_SAX],
        bar_plot_color_map={
            ("envelope-ed-early", False): PALETTE["Blues"][6],
            ("vl_envelope-ed-early", False): PALETTE["Pinks"][0],
            ("vl_envelope-ed-early", True): PALETTE["Pinks"][3],
        },
        bar_plot_label_map={
            ("envelope-ed-early", False): "MT-Env",
            ("vl_envelope-ed-early", False): "VL-Env",
            ("vl_envelope-ed-early", True): "VL-Env invSAX",
        },
        y_scale="log" if target_args.name == "QUERY_TIME" else "linear",
        save_dir=os.path.join(EXTENSIONS_FIGS_DIR, f"8_vl_envelope_{target_args.name.lower()}"),
        **target_args.value,
    )

# %%
# Dataset examples

visualize_time_series(save_dir=os.path.join(EXPERIMENT_FIGS_DIR, "dataset_examples"))

# %%
# 9a - Method comparison

for target_args in [
    TargetArgs.QUERY_TIME,
    TargetArgs.PRUNING_RATIO,
    TargetArgs.ABANDONING_RATE,
    TargetArgs.COMBINED_TIME,
]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_9a_method_comparison"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY],
        },
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [(2048,)],
            (SSC.NORMALIZED,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [(0.125, 1.0)],
        },
        legend_max_cols=4,
        legend_plots_dict={DSC.SERIES_LENGTH: {2048}, QSC.L_MIN_RATIO: {0.125}},
        legend_only_hatch=target_args.name == "COMBINED_TIME",
        bar_plot_label_padding=False,
        bar_plot_color_attrs=[SSC.METHOD_NAME, ISC.SEGMENTATION_STRATEGY],
        bar_plot_label_map=method_comparison_labels,
        bar_plot_color_map=method_comparison_colors,
        bar_width_inches=0.4,
        y_scale="log" if target_args.name == "QUERY_TIME" else "linear",
        merge_csv_datasets=True,
        save_dir=os.path.join(EXPERIMENT_FIGS_DIR, f"9a_method_compare_{target_args.name.lower()}"),
        **target_args.value,
    )

# %%
# 9a - Method comparison across series lengths

key_to_col = {
    "m": DSC.SERIES_LENGTH,
    "lmin": QSC.L_MIN_RATIO,
    "ql": QC.QUERY_INTERVAL,
}

for key, col in key_to_col.items():
    suffix = "_m" if key == "m" else ""
    for target_args in [TargetArgs.QUERY_TIME]:
        visualize_experiments(
            logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_9a_method_comparison{suffix}"],
            groups_dict={
                ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
                ERD.DATASETS_COLS: [DSC.SERIES_LENGTH],
                ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO],
                ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY],
            },
            separate_plots_dict={
                **({(QSC.L_MIN_RATIO,): []} if key != "lmin" else {}),
                **({(DSC.SERIES_LENGTH,): []} if key != "m" else {}),
                (SSC.NORMALIZED,): [],
            },
            num_query_intervals=10 if key == "ql" else 1,
            legend_max_cols=2,
            legend_plots_dict={
                DSC.DATASET_FILE: {"synthetic"},
                **({QSC.L_MIN_RATIO: {0.125}} if key != "lmin" else {}),
                **({DSC.SERIES_LENGTH: {2048}} if key != "m" else {}),
                SSC.NORMALIZED: {True},
            },
            bar_plot_color_attrs=None,
            line_plot_x_attr=col,
            line_plot_show_min=False,
            line_plot_included_cols={SSC.METHOD_NAME, ISC.SEGMENTATION_STRATEGY},
            line_plot_legend_map=method_comparison_labels,
            line_plot_colors_map=method_comparison_colors,
            x_scale="log" if key == "lmin" else "linear",
            y_scale="log" if target_args.name == "QUERY_TIME" else "linear",
            save_dir=os.path.join(EXPERIMENT_FIGS_DIR, f"9a_method_compare_{key}_{target_args.name.lower()}"),
            **target_args.value,
        )

# %%
# 9b - Method comparison by number of channels

for constant in [""]:
    suffix = f"_{constant}" if len(constant) > 0 else ""
    for target_args in [TargetArgs.QUERY_TIME, TargetArgs.PRUNING_RATIO]:
        visualize_experiments(
            logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_9b_num_channels{suffix}"],
            groups_dict={
                ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
                ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.NUM_CHANNELS],
                ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
                ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY],
            },
            separate_plots_dict={
                (DSC.DATASET_FILE,): [],
                (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
                (SSC.NORMALIZED,): [],
            },
            # title_base=f"Constant {constant}" if len(constant) > 0 else None,
            legend_max_cols=2,
            legend_plots_dict={SSC.NORMALIZED: {True}} if target_args.name == "QUERY_TIME" else None,
            bar_plot_color_attrs=None,
            line_plot_x_attr=DSC.NUM_CHANNELS,
            line_plot_show_min=False,
            line_plot_included_cols={SSC.METHOD_NAME, ISC.SEGMENTATION_STRATEGY},
            line_plot_legend_map=method_comparison_labels,
            line_plot_colors_map=method_comparison_colors,
            x_scale="log",
            y_scale="log" if target_args.name == "QUERY_TIME" else "linear",
            save_dir=os.path.join(EXPERIMENT_FIGS_DIR, f"9b_num_channels{suffix}_{target_args.name.lower()}"),
            **target_args.value,
        )

# %%
# 9c - Method comparison under ad-hoc channel selection

for target_args in [TargetArgs.QUERY_TIME, TargetArgs.PRUNING_RATIO]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_9c_ad_hoc_channels"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.USED_CHANNELS],
            ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY],
        },
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (SSC.NORMALIZED,): [],
            (DSC.DATASET_FILE,): [],
        },
        legend_max_cols=2,
        legend_plots_dict={DSC.DATASET_FILE: {"synthetic"}, SSC.NORMALIZED: {True}}
        if target_args.name == "QUERY_TIME"
        else None,
        bar_plot_color_attrs=None,
        line_plot_x_attr=QSC.USED_CHANNELS,
        line_plot_show_min=False,
        line_plot_included_cols={SSC.METHOD_NAME, ISC.SEGMENTATION_STRATEGY},
        line_plot_legend_map=method_comparison_labels,
        line_plot_colors_map=method_comparison_colors,
        y_scale="log" if target_args.name == "QUERY_TIME" else "linear",
        save_dir=os.path.join(EXPERIMENT_FIGS_DIR, f"9c_ad_hoc_channels_{target_args.name.lower()}"),
        **target_args.value,
    )


# %%
# 10a - Size limiting

size_limit = 0.5

for target_args in [TargetArgs.ESTIMATE_SCORE]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_10a_size_limiting_baselines"],
        groups_dict={
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
            (DSC.DATASET_FILE,): [],
            (ISC.INDEX_SIZE_LIMIT,): [(size_limit,)],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
            (ISC.PARAM_ESTIMATOR_TYPE,): [("min_dist",), ("query_time",)],
        },
        bar_plot_color_attrs=None,
        merge_csv_datasets=True,
        heat_map_x_attr=PEC.NUM_LEN_GROUPS,
        heat_map_y_attr=PEC.NUM_SEGMENTS,
        heat_map_included_cols={ISC.SEGMENTATION_STRATEGY},
        cell_width_inches=7.5,
        save_dir=os.path.join(EXPERIMENT_FIGS_DIR, f"10a_size_lim_baselines_est_{target_args.name.lower()}"),
        **target_args.value,
    )

for target_args in [TargetArgs.QUERY_TIME]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_10a_size_limiting_baselines"],
        groups_dict={
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [
                ISC.INDEX_SIZE_LIMIT,
                ISC.NUM_LEN_GROUPS,
                ISC.NUM_SEGMENTS,
                ISC.SEGMENTATION_STRATEGY,
                ISC.PARAM_ESTIMATOR_TYPE,
            ],
        },
        separate_plots_dict={
            (DSC.DATASET_FILE,): [],
            (ISC.INDEX_SIZE_LIMIT,): [(size_limit,)],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
            (ISC.PARAM_ESTIMATOR_TYPE,): [("no_est",)],
        },
        bar_plot_color_attrs=None,
        merge_csv_datasets=True,
        heat_map_x_attr=ISC.NUM_LEN_GROUPS,
        heat_map_y_attr=ISC.NUM_SEGMENTS,
        heat_map_included_cols={DSC.DATASET_FILE, ISC.SEGMENTATION_STRATEGY},
        cell_width_inches=7.5,
        save_dir=os.path.join(EXPERIMENT_FIGS_DIR, f"10a_size_lim_baselines_gt_{target_args.name.lower()}"),
        **target_args.value,
    )

# %%
# 10b - Size limiting performance

values = {}

for target_args in [TargetArgs.QUERY_TIME, TargetArgs.INDEX_SIZE]:
    values[target_args.name] = visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_10b_size_limiting_performance"],
        # logs_dirs=["LOGS"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY, ISC.INDEX_SIZE_LIMIT, ISC.PARAM_ESTIMATOR_TYPE],
        },
        separate_plots_dict={
            (DSC.DATASET_FILE,): [],
            (DSC.SERIES_LENGTH,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
        },
        regex_dict={(ISC.INDEX_SIZE_LIMIT, SSC.METHOD_NAME): r"^(0\.0::.*|.*::.*ed-early)"},
        legend_max_cols=5,
        legend_plots_dict={DSC.DATASET_FILE: {"stocks"}} if target_args.name == "QUERY_TIME" else None,
        bar_plot_label_padding=False,
        bar_plot_color_attrs=[SSC.METHOD_NAME, ISC.SEGMENTATION_STRATEGY],
        bar_plot_label_map=method_comparison_labels,
        bar_plot_color_map=method_comparison_colors,
        bar_width_inches=0.4,
        y_scale="log",
        return_values=target_args.name == "QUERY_TIME",
        save_dir=os.path.join(EXPERIMENT_FIGS_DIR, f"10b_size_limiting_performance_{target_args.name.lower()}"),
        **target_args.value,
    )

# %%

# Calculate percentage differences (adaptive compared to uniform)
for title_key, results in values[TargetArgs.QUERY_TIME.name].items():
    dataset = title_key[0]
    print(f"\nDataset: {dataset}")

    method_name = "sax_envelope-ed-early"
    size_limits = list(set([key[2] for key in results.keys() if key[0] == method_name]))
    est_types = list(set([key[3] for key in results.keys() if key[0] == method_name]))

    diffs = []
    for size_limit in size_limits:
        for est_type in est_types:
            adaptive_val = results[(method_name, "adaptive", size_limit, est_type)][0]
            uniform_val = results[(method_name, "uniform", size_limit, est_type)][0]

            pct_diff = (uniform_val - adaptive_val) / uniform_val * 100
            diffs.append(pct_diff)
            print(f"Size limit {size_limit}, {est_type}: {pct_diff:.2f}%")

    # Calculate statistics
    avg_diff = sum(diffs) / len(diffs)
    min_diff = min(diffs)
    max_diff = max(diffs)

    print("\nSummary statistics:")
    print(f"Average improvement: {avg_diff:.2f}%")
    print(f"Best improvement: {max_diff:.2f}%")
    print(f"Worst case: {min_diff:.2f}%")


# %%
# 10b - Size limiting bias of QT-Est for high Nl

method_name = "sax_envelope-ed-early"

for target_args in [TargetArgs.NUM_LEN_GROUPS, TargetArgs.ENV_PARAM_ESTIMATION_TIME, TargetArgs.INDEX_TIME]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_10b_size_limiting_performance"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN_RATIO, QSC.L_MAX_RATIO],
            ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY, ISC.INDEX_SIZE_LIMIT, ISC.PARAM_ESTIMATOR_TYPE],
        },
        separate_plots_dict={
            (DSC.DATASET_FILE,): [],
            (DSC.SERIES_LENGTH,): [],
            (QSC.L_MIN_RATIO, QSC.L_MAX_RATIO): [],
        },
        regex_dict={(ISC.INDEX_SIZE_LIMIT, SSC.METHOD_NAME): rf"^(.*::{method_name})"},
        legend_max_cols=2,
        legend_plots_dict={DSC.DATASET_FILE: {"synthetic"}} if target_args.name == "NUM_LEN_GROUPS" else None,
        bar_plot_color_attrs=None,
        line_plot_legend_map={
            (method_name, "uniform", "query_time"): "EqW QT-Est",
            (method_name, "adaptive", "query_time"): "EqD QT-Est",
            (method_name, "uniform", "min_dist"): "EqW MD-Est",
            (method_name, "adaptive", "min_dist"): "EqD MD-Est",
        },
        line_plot_colors_map={
            (method_name, "uniform", "query_time"): PALETTE["Pinks"][5],
            (method_name, "adaptive", "query_time"): PALETTE["Pinks"][2],
            (method_name, "uniform", "min_dist"): PALETTE["Blues"][6],
            (method_name, "adaptive", "min_dist"): PALETTE["Blues"][2],
        },
        line_plot_x_attr=ISC.INDEX_SIZE_LIMIT,
        line_plot_included_cols={ISC.SEGMENTATION_STRATEGY, ISC.PARAM_ESTIMATOR_TYPE},
        line_plot_show_min=False,
        x_scale="log",
        y_scale="linear" if target_args.name == "NUM_LEN_GROUPS" else "log",
        save_dir=os.path.join(EXPERIMENT_FIGS_DIR, f"10b_size_limiting_{target_args.name.lower()}"),
        **target_args.value,
    )

# %%
# 11a,b,c - Query difficulty, KNN K, R-range

key_to_col = {
    "a_query_difficulty": QSC.NOISE,
    "b_knn_k": SSC.KNN_K,
    "c_r_range_r": SSC.R_RANGE_R,
}

for key, col in key_to_col.items():
    for target_args in [TargetArgs.QUERY_TIME]:
        visualize_experiments(
            logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_11{key}"],
            groups_dict={
                ERD.METHODS_COLS: [SSC.METHOD_NAME, SSC.NORMALIZED, *([col] if isinstance(col, SSC) else [])],
                ERD.DATASETS_COLS: [DSC.DATASET_FILE],
                ERD.INDEXES_COLS: [ISC.SEGMENTATION_STRATEGY],
                **({ERD.QUERY_SETS_COLS: [col]} if isinstance(col, QSC) else {}),
            },
            separate_plots_dict={
                (DSC.DATASET_FILE,): [],
                (SSC.NORMALIZED,): [],
            },
            legend_max_cols=2,
            legend_plots_dict={SSC.NORMALIZED: {True}} if col == SSC.KNN_K else None,
            bar_plot_color_attrs=None,
            line_plot_x_attr=col,
            line_plot_included_cols={SSC.METHOD_NAME, ISC.SEGMENTATION_STRATEGY},
            line_plot_colors_map=method_comparison_colors,
            line_plot_legend_map=method_comparison_labels,
            x_scale="log" if col in [QSC.NOISE, SSC.KNN_K] else "linear",
            y_scale="log" if target_args.name == "QUERY_TIME" else "linear",
            save_dir=os.path.join(EXPERIMENT_FIGS_DIR, f"11{key}_{target_args.name.lower()}"),
            verbose=col == QSC.NOISE,
            **target_args.value,
        )
