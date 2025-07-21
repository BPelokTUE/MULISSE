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


from scripts.py.visualization.plots import get_dataset_label
from scripts.py.visualization.style import CATEGORY_COLORS

# %%


def plot_time_series(
    data: np.ndarray,
    channel_labels: list[str] | None = None,
    channel_alphas: list[float] | None = None,
    title: str = "Time Series Data",
    save_path: str | None = None,
):
    num_figs = data.shape[0] if data.ndim > 2 else 1
    num_channels = data.shape[1] if data.ndim > 2 else data.shape[0]
    fig, axs = plt.subplots(nrows=num_figs, figsize=(10, num_figs * 3), squeeze=False)

    if channel_labels is None:
        channel_labels = [f"Channel {i}" for i in range(num_channels)]
    if channel_alphas is None:
        channel_alphas = [0.75] * num_channels

    for i, (ax_row, mts_data) in enumerate(zip(axs, data)):
        ax = ax_row[0]
        for channel, color, label, alpha in zip(mts_data, CATEGORY_COLORS, channel_labels, channel_alphas):
            ax.plot(channel, color=color, label=label, alpha=alpha)
        if i == 0:
            ax.set_title(title)
        ax.legend(loc="center left", bbox_to_anchor=(1.0, 0.5))

    if save_path is not None:
        os.makedirs(os.path.dirname(save_path), exist_ok=True)
        plt.savefig(save_path, bbox_inches="tight")

    plt.show()


# %%


def normalize_time_series(data: np.ndarray) -> np.ndarray:
    if data.ndim > 1:
        return np.array([normalize_time_series(channel) for channel in data])
    return (data - np.mean(data)) / np.std(data)


# %%


def get_random_walk(
    series_length: int,
    num_channels: int,
    num_series: int = 1,
    step_stdevs: list[float] | float = 1.0,
    normalize: bool = True,
) -> np.ndarray:
    random_walk = np.zeros((num_series, num_channels, series_length))
    if isinstance(step_stdevs, list):
        for c, stdev in enumerate(step_stdevs):
            noise = np.random.normal(0, stdev, (num_series, series_length - 1))
            random_walk[:, c, 1:] += np.cumsum(noise, axis=-1)
    else:
        noise = np.random.normal(0, step_stdevs, (num_series, num_channels, series_length - 1))
        random_walk[:, :, 1:] = np.cumsum(noise, axis=-1)
    if normalize:
        random_walk = normalize_time_series(random_walk)
    return random_walk


# %%


def load_csv_data(
    dir_path: str,
    series_length: int,
    series_inds: list[int],
    files: list[str] = [],
    regex_query=r".*",
    normalize: bool = True,
) -> tuple[np.ndarray, list[str]]:
    if len(files) == 0:
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

    base_dir = os.path.basename(dir_path)
    return data, [get_dataset_label(os.path.join(base_dir, file), only_last=True) for file in files]


# %%


def visualize_time_series(stocks_series_inds=[200], weather_series_inds=[481], save_dir=None):
    with open("local_settings.json", "r") as f:
        LOCAL_SETTINGS = json.load(f)

    CSV_PATH = LOCAL_SETTINGS["CSV_PATH"]

    plot_time_series(
        *load_csv_data(
            os.path.join(CSV_PATH, "stocks"),
            2048,
            series_inds=stocks_series_inds,
            files=[
                "first_clean.csv",
                "second_clean.csv",
                "third_clean.csv",
                "fourth_clean.csv",
                "fifth_clean_fixed.csv",
            ],
        ),
        title="Stock Data",
        save_path=os.path.join(save_dir, "stocks.pdf") if save_dir else None,
    )

    num_channels_syn = 4
    plot_time_series(
        get_random_walk(2048, num_channels_syn, step_stdevs=1.0),
        [f"Channel {i}" for i in range(num_channels_syn)],
        title="Synthetic",
        save_path=os.path.join(save_dir, "synthetic.pdf") if save_dir else None,
    )

    plot_time_series(
        *load_csv_data(
            os.path.join(CSV_PATH, "weather"), 2048, weather_series_inds, files=["TMP", "DEW", "SLP", "WND"]
        ),
        title="Weather Data",
        save_path=os.path.join(save_dir, "weather.pdf") if save_dir else None,
    )


# %%
