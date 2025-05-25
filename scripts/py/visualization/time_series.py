# %%

import json
import os
import re

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

if True:
    while not os.getcwd().endswith("MULISSE"):
        os.chdir("..")


from scripts.py.visualization.style import CATEGORY_COLORS

with open("local_settings.json", "r") as f:
    LOCAL_SETTINGS = json.load(f)

CSV_PATH = LOCAL_SETTINGS["CSV_PATH"]

# %%


def plot_time_series(
    data: np.ndarray,
    channel_labels: list[str] | None = None,
    channel_alphas: list[float] | None = None,
    title: str = "Time Series Data",
):
    num_figs = data.shape[0] if data.ndim > 2 else 1
    fig, axs = plt.subplots(nrows=num_figs, figsize=(10, num_figs * 3), squeeze=False)

    if channel_labels is None:
        channel_labels = [f"Channel {i}" for i in range(data.shape[0])]
    if channel_alphas is None:
        channel_alphas = [0.75] * len(channel_labels)

    for i, (ax_row, mts_data) in enumerate(zip(axs, data)):
        ax = ax_row[0]
        for channel, color, label, alpha in zip(mts_data, CATEGORY_COLORS, channel_labels, channel_alphas):
            ax.plot(channel, color=color, label=label, alpha=alpha)
        if i == 0:
            ax.set_title(title)
        ax.legend(loc="center left", bbox_to_anchor=(1.0, 0.5))
    plt.show()


# %%


def normalize_time_series(data: np.ndarray) -> np.ndarray:
    if data.ndim > 1:
        return np.array([normalize_time_series(channel) for channel in data])
    return (data - np.mean(data)) / np.std(data)


# %%


def load_csv_data(
    dir_path: str, series_length: int, series_inds: list[int], regex_query=r".*", normalize: bool = True
) -> tuple[np.ndarray, list[str]]:
    files = sorted([f for f in os.listdir(dir_path) if re.match(regex_query, f)])

    data = np.zeros((len(series_inds), len(files), series_length))
    for i, file in enumerate(files):
        file_path = os.path.join(dir_path, file)
        if not os.path.isfile(file_path):
            raise FileNotFoundError(f"File {file_path} does not exist.")

        for j, series_ind in enumerate(series_inds):
            row = pd.read_csv(file_path, header=None, skiprows=series_ind, nrows=1).to_numpy().squeeze()
            if len(row) < series_length:
                row = np.pad(row, (0, series_length - len(row)), mode="constant", constant_values=0)
            data[j, i, :] = row[:series_length]

    if normalize:
        data = normalize_time_series(data)
    return data, files


# %%

series_inds = [481, 4312]
for highlighted_channel in range(0, 4):
    channel_alphas = [0.3] * 4
    channel_alphas[highlighted_channel] = 1.0
    plot_time_series(
        *load_csv_data(os.path.join(CSV_PATH, "weather"), 1024, series_inds),
        channel_alphas=channel_alphas,
        title="Weather Data",
    )

# %%

regex_query = r".*"  # r"^(?!.*fifth).*$"
plot_time_series(
    *load_csv_data(os.path.join(CSV_PATH, "stocks"), 256, [i for i in [200]], regex_query),
    # channel_alphas=[1.0, 0.25, 0.25, 0.25],
    title="Stock Data",
)
