# %%
import os

if True:
    while not os.getcwd().endswith("MULISSE"):
        os.chdir("..")

from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import IndexSettingsColumn as ISC
from scripts.py.common.columns import QuerySetSettingsColumn as QSC
from scripts.py.common.columns import SearchSettingsColumn as SSC
from scripts.py.visualization.reduction import ERD
from scripts.py.visualization.wrapper import TargetArgs, visualize_experiments

# %%
# 1 - ULISSE stage comparison


for suffix in ["uni", "multi"]:
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
        bar_plot_color_attr=SSC.METHOD_NAME,
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
        bar_plot_color_attr=SSC.METHOD_NAME,
        bar_plot_legend_max_cols=2,
        bar_plot_label_map={
            "isax-ed-early": "iSAX",
            "sax_envelope-ed-early": "MT-Env",
        },
        **target_args_dict,
    )
