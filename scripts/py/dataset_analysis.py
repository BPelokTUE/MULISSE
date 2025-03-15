# %%
import os
from typing import Any

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

from common.ulisse_envelopes import ulisse_envelope_normalized

LOGS_DIR = "../../LOGS"
DATA_DIR = "../../DATA"
DATASET_SETTINGS_CSV = "dataset_settings.csv"

ID_COL = "id"
DATASET_FILE_COL = "dataset_file"
SERIES_LENGTH_COL = "series_length"
NUM_CHANNELS_COL = "num_channels"
NUM_SERIES_COL = "num_series"
SD_COL = "sd"
SOURCE_CSVS_COL = "source_csvs"
LOW_SD_LEN_COL = "low_sd_len"

dataset_settings_df = pd.read_csv(f"{LOGS_DIR}/{DATASET_SETTINGS_CSV}")
dataset_settings_df = dataset_settings_df[
    dataset_settings_df[DATASET_FILE_COL].apply(lambda x: os.path.exists(f"{DATA_DIR}/{x}"))
]

datasets = dataset_settings_df[DATASET_FILE_COL].unique()
print(datasets)


# %%
def load_series(dataset_file: str, series_ind: int, num_channels: int, series_len: int) -> np.ndarray:
    file_path = f"{DATA_DIR}/{dataset_file}"
    start_pos = series_ind * num_channels * series_len * 4
    series = np.fromfile(file_path, dtype=np.float32, count=num_channels * series_len, offset=start_pos)
    return series.reshape((num_channels, series_len))


def z_normalize_series(series: np.ndarray) -> np.ndarray:
    mean = np.mean(series, axis=1, keepdims=True)
    std = np.std(series, axis=1, keepdims=True)
    return (series - mean) / std


# %%
"""
#### Pick random time series
"""

QUERY_LEN = 384

selected_series: dict[str, Any] = {dataset: None for dataset in datasets}
selected_queries: dict[str, Any] = {dataset: None for dataset in datasets}

for dataset in datasets:
    settings = dataset_settings_df[dataset_settings_df[DATASET_FILE_COL] == dataset].iloc[0]

    series_len = settings[SERIES_LENGTH_COL]
    num_channels = settings[NUM_CHANNELS_COL]

    series_ind = np.random.randint(0, settings[NUM_SERIES_COL])
    series = load_series(dataset, series_ind, num_channels, series_len)
    series = z_normalize_series(series)
    selected_series[dataset] = series
    series_std = np.std(series, axis=1, keepdims=True)

    query_series_ind = np.random.randint(0, settings[NUM_SERIES_COL])
    query_series = load_series(dataset, query_series_ind, num_channels, series_len)
    query = query_series[:, :QUERY_LEN]
    query = query + np.random.normal(0, 0.1 * series_std, query.shape)
    query = z_normalize_series(query)
    selected_queries[dataset] = query

    for c in range(series.shape[0]):
        plt.plot(series[c])
    plt.title(f"Selected series from {dataset}")
    plt.show()

    for c in range(query.shape[0]):
        plt.plot(query[c])
    plt.title(f"Selected query from {dataset}")
    plt.show()


# %%
"""
#### Generate envelopes for time series
"""

selected_envelopes: dict[str, dict] = {dataset: {} for dataset in datasets}

SEGMENT_LENGTHS = [32, 128]
L_MIN = 128

for dataset, series in selected_series.items():
    pos_per_env = series_len - L_MIN + 1
    l_max = series_len
    for segment_len in SEGMENT_LENGTHS:
        selected_envelopes[dataset][segment_len] = [
            ulisse_envelope_normalized(channel, (pos_per_env, segment_len, L_MIN, l_max)) for channel in series
        ]


# %%
"""
#### Visualized envelopes on time series
"""

SHOW_QUERY = False
SHOW_SERIES = True
ENVELOPE_MASK = [1, 1]

alpha = 1.0 / len(SEGMENT_LENGTHS)

for dataset, envelope_dict in selected_envelopes.items():
    print(f"Showing envelopes for {dataset}")

    num_channels = len(envelope_dict[SEGMENT_LENGTHS[0]])
    for c in range(num_channels):
        for sl_ind, segment_len in enumerate(SEGMENT_LENGTHS):
            if sl_ind < len(ENVELOPE_MASK) and ENVELOPE_MASK[sl_ind] == 0:
                continue

            envelopes = envelope_dict[segment_len]
            lower = envelopes[c][0]["lower"]
            upper = envelopes[c][0]["upper"]
            query_channel = selected_queries[dataset][c]

            for i, (l_val, u_val) in enumerate(zip(lower, upper)):
                start = i * segment_len
                end = start + segment_len

                if len(query_channel) >= end:
                    query_paa = np.mean(query_channel[start:end])
                    plt.hlines(query_paa, start, end, colors="purple", linewidth=3)

                min_l_val = np.min(l_val)
                plt.hlines(min_l_val, start, end, colors="r")
                max_u_val = np.max(u_val)
                plt.hlines(max_u_val, start, end, colors="r")
                plt.fill_between((start, end), min_l_val, max_u_val, color="r", alpha=alpha)
        if SHOW_QUERY:
            plt.plot(selected_queries[dataset][c], color="g")
        if SHOW_SERIES:
            plt.plot(selected_series[dataset][c])
        plt.show()

# %%
