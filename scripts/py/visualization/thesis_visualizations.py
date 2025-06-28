# %%
import os

if True:
    while not os.getcwd().endswith("MULISSE"):
        os.chdir("..")

from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import IndexSettingsColumn as ISC
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import QuerySetSettingsColumn as QSC
from scripts.py.common.columns import SearchSettingsColumn as SSC
from scripts.py.visualization.predict_runtime import visualize_clusters
from scripts.py.visualization.reduction import ERD
from scripts.py.visualization.wrapper import TargetArgs, visualize_experiments

# %%
# 1 - ULISSE stage comparison


for suffix in ["uni"]:
    for target_args_dict in [TargetArgs.QUERY_TIME.value]:
        visualize_experiments(
            logs_dirs=[f"EXPERIMENT_LOGS/thesis/LOGS_1_mulisse_stages_{suffix}"],
            groups_dict={
                ERD.METHODS_COLS: [SSC.METHOD_NAME],
                ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.NUM_CHANNELS],
                ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            },
            separate_plots_dict={(DSC.DATASET_FILE,): []},
            bar_plot_label_padding=False,
            merge_csv_datasets=True,
            bar_plot_label_map={
                "isax_envelope-ed-early": "First phase",
                "isax_env_w_sax_env-ed-early": "Both phases",
                "sax_envelope-ed-early": "Second phase",
            },
            **target_args_dict,
        )

# %%
# 2 - SAX vs no SAX envelope
for target_args_dict in [TargetArgs.QUERY_TIME.value]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_2_sax_vs_no_sax"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
        },
        separate_plots_dict={(DSC.DATASET_FILE,): []},
        bar_plot_color_attrs=SSC.METHOD_NAME,
        bar_plot_label_padding=False,
        bar_plot_legend_max_cols=2,
        bar_plot_label_map={
            "envelope-ed-early": "MT-Env no SAX",
            "sax_envelope-ed-early": "MT-Env",
            "isax_env_w_env-ed-early": "Two-phase MULISSE no SAX",
            "isax_env_w_sax_env-ed-early": "Two-phase MULISSE",
        },
        **target_args_dict,
    )

# %%
# 3- iSAX vs MT-Env
for target_args_dict in [TargetArgs.QUERY_TIME.value, TargetArgs.PRUNING_RATIO.value]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_3_isax_vs_envelope"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            ERD.INDEXES_COLS: [ISC.NUM_BITS_LIMIT, ISC.POS_PER_ENV],
        },
        separate_plots_dict={(DSC.DATASET_FILE,): []},
        bar_plot_color_attrs=SSC.METHOD_NAME,
        bar_plot_legend_max_cols=2,
        bar_plot_label_map={
            "isax-ed-early": "iSAX",
            "sax_envelope-ed-early": "MT-Env",
        },
        **target_args_dict,
    )

# %%
# 4a - Low resolution univariate

for target_args_dict in [TargetArgs.QUERY_TIME.value]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_4a_low_res_uni"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            ERD.INDEXES_COLS: [ISC.NUM_SEGMENTS, ISC.POS_PER_ENV, ISC.NUM_LEN_GROUPS],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.DATASET_FILE, DSC.SERIES_LENGTH): [],
            (QSC.L_MIN, QSC.L_MAX): [],
        },
        bar_plot_color_attrs=None,
        heat_map_x_attr=ISC.POS_PER_ENV,
        heat_map_y_attr=ISC.NUM_LEN_GROUPS,
        heat_map_included_cols={ISC.NUM_SEGMENTS},
        **target_args_dict,
    )

# %%
# 4a - Low resolution multivariate

for target_args_dict in [TargetArgs.QUERY_TIME.value]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_4a_low_res_multi"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            ERD.INDEXES_COLS: [ISC.NUM_SEGMENTS, ISC.POS_PER_ENV, ISC.NUM_LEN_GROUPS],
        },
        merge_csv_datasets=False,
        separate_plots_dict={
            (DSC.DATASET_FILE, DSC.SERIES_LENGTH): [],
            (QSC.L_MIN, QSC.L_MAX): [],
        },
        bar_plot_color_attrs=None,
        heat_map_x_attr=ISC.POS_PER_ENV,
        heat_map_y_attr=ISC.NUM_LEN_GROUPS,
        heat_map_included_cols={ISC.NUM_SEGMENTS},
        **target_args_dict,
    )

# %%
# 4b - Number of lengths per LG (λ) univariate

for target_args_dict in [TargetArgs.QUERY_TIME.value]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_4b_length_group_uni"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            ERD.INDEXES_COLS: [ISC.NUM_LEN_GROUPS],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (QSC.L_MIN, QSC.L_MAX): [],
        },
        bar_plot_color_attrs=None,
        line_plot_x_attr=ISC.NUM_LEN_GROUPS,
        line_plot_included_cols={DSC.DATASET_FILE},
        x_scale="log",
        **target_args_dict,
    )

# %%
# 4c - Positions per envelope (γ) univariate

for target_args_dict in [TargetArgs.QUERY_TIME.value]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_4c_ppe_uni"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            ERD.INDEXES_COLS: [ISC.POS_PER_ENV],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (QSC.L_MIN, QSC.L_MAX): [],
        },
        bar_plot_color_attrs=None,
        line_plot_x_attr=ISC.POS_PER_ENV,
        line_plot_included_cols={DSC.DATASET_FILE},
        x_scale="log",
        **target_args_dict,
    )

# %%
# 4d - Segment size (σ) univariate

for target_args_dict in [TargetArgs.QUERY_TIME.value]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_4d_num_segments_uni"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
            ERD.INDEXES_COLS: [ISC.NUM_SEGMENTS],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (QSC.L_MIN, QSC.L_MAX): [],
        },
        bar_plot_color_attrs=None,
        line_plot_x_attr=ISC.NUM_SEGMENTS,
        line_plot_included_cols={DSC.DATASET_FILE},
        **target_args_dict,
    )

# %%
# 4e - Showing linear relationship between runtime and dataset size

for target_args_dict in [TargetArgs.QUERY_TIME.value]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_4e_num_series"],
        groups_dict={
            ERD.METHODS_COLS: [SSC.METHOD_NAME],
            ERD.DATASETS_COLS: [DSC.DATASET_FILE, DSC.SERIES_LENGTH, DSC.NUM_SERIES],
            ERD.QUERY_SETS_COLS: [QSC.L_MIN, QSC.L_MAX],
        },
        merge_csv_datasets=True,
        separate_plots_dict={
            (DSC.SERIES_LENGTH,): [],
            (QSC.L_MIN, QSC.L_MAX): [],
        },
        bar_plot_color_attrs=None,
        line_plot_x_attr=DSC.NUM_SERIES,
        line_plot_included_cols={DSC.DATASET_FILE},
        **target_args_dict,
    )

# %%
# 5 - Presence

# The visualization function does not easily support separate plots on combinations of attributes (in this case
# LG segmentation strategy + Segmentation strategy + Number of segments would be needed), so this ID-based solution
# is needed
size_to_ids = {
    "small": [9, 0, 6, 3],
    "medium": [10, 1, 7, 4],
    "large": [11, 2, 8, 5],
}
size_regexes = {size: rf"^({'|'.join(map(str, ids))})(\.0){{0,1}}$" for size, ids in size_to_ids.items()}
#

for target_args_dict in [TargetArgs.INDEX_SIZE.value, TargetArgs.QUERY_TIME.value]:
    for size, regex in size_regexes.items():
        visualize_experiments(
            logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_5_presence"],
            groups_dict={
                ERD.DATASETS_COLS: [DSC.DATASET_FILE],
                ERD.INDEXES_COLS: [
                    ISC.ID,
                    ISC.NUM_SEGMENTS,
                    ISC.SEGMENTATION_STRATEGY,
                    ISC.LG_SEGMENTATION_STRATEGY,
                ],
            },
            regex_dict={ISC.ID: regex},
            separate_plots_dict={(DSC.DATASET_FILE,): []},
            bar_plot_color_attrs=[ISC.SEGMENTATION_STRATEGY, ISC.LG_SEGMENTATION_STRATEGY],
            bar_plot_label_map={
                ("uniform", "single"): "Simple",
                ("adaptive", "single"): "Adaptive Segmentation",
                ("uniform", "adaptive_multi"): "Adaptive LG",
                ("adaptive", "adaptive_multi"): "Adaptive LG + Segmentation",
            },
            bar_plot_legend_max_cols=2,
            **target_args_dict,
        )

# %%
# 6 - Channel performance

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

for target_args_dict in [TargetArgs.QUERY_TIME.value]:
    for dataset, dataset_cols in dataset_cols_map.items():
        visualize_experiments(
            logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_6_ch_performance"],
            groups_dict={ERD.DATASETS_COLS: dataset_cols},
            regex_dict={DSC.DATASET_FILE: rf"^{dataset}"},
            bar_plot_color_attrs=dataset_cols[-1],
            bar_plot_legend_max_cols=2,
            bar_plot_label_map=label_maps[dataset],
            **target_args_dict,
        )

# %%
# 6 - Channel clustering
visualize_clusters(logs_dir="EXPERIMENT_LOGS/thesis/LOGS_6_ch_clustering")

# %%
# 6 - Channel prioritization
for target_args_dict in [TargetArgs.QUERY_TIME.value]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_6_ch_prioritization"],
        groups_dict={
            ERD.DATASETS_COLS: [DSC.DATASET_FILE],
            ERD.INDEXES_COLS: [ISC.NUM_SEGMENTS, ISC.CH_SEGMENTATION_STRATEGY, ISC.SCORE_BASED_CHSS_SCORE_EXP],
        },
        separate_plots_dict={(DSC.DATASET_FILE, ISC.NUM_SEGMENTS): []},
        bar_plot_color_attrs=[ISC.CH_SEGMENTATION_STRATEGY, ISC.SCORE_BASED_CHSS_SCORE_EXP],
        bar_plot_legend_max_cols=2,
        bar_plot_label_map={
            ("single", 0): "Simple",
            ("score_based", -1): "Prioritize Easy",
            ("score_based", 1): "Prioritize Hard",
        },
        **target_args_dict,
    )

# %%
# 7 - Envelope merging

for target_args_dict in [
    TargetArgs.QUERY_TIME.value,
    TargetArgs.MINDIST_TIME.value,
    TargetArgs.PRUNING_RATIO.value,
    # TargetArgs.INDEX_SIZE.value,
    # {"targets_dict": {ERD.RUNS_COLS: [QC.TS_EXAMINATION_TIME_S]}},
]:
    visualize_experiments(
        logs_dirs=["EXPERIMENT_LOGS/thesis/LOGS_8_vl_envelope"],
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
        bar_plot_color_attr=SSC.METHOD_NAME,
        bar_plot_legend_max_cols=2,
        bar_plot_label_map={
            "bucketing_envelope-ed-early": "Bucket merger",
            "envelope-ed-early": "No merger",
            "vl_envelope-ed-early": "Variance-limiting merger",
        },
        **target_args_dict,
    )

# %%
