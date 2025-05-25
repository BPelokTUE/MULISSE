# %%

import json
import os

if True:
    while not os.getcwd().endswith("MULISSE"):
        os.chdir("..")

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import DatasetStatsColumn as DSTC
from scripts.py.common.columns import IndexStatsColumn as ISTC
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import StatsColumn as SC
from scripts.py.common.columns import StatsColumnPrefix as SCP
from scripts.py.visualization.helpers import dict_to_tuples
from scripts.py.visualization.reduction import (
    ERD,
    CollectionReducer,
    CombinedReducer,
    ExperimentResults,
    MeanReducer,
    Reducer,
    StdReducer,
    execute_reduction,
)
from sklearn.decomposition import PCA
from sklearn.ensemble import AdaBoostRegressor, GradientBoostingRegressor, RandomForestRegressor
from sklearn.linear_model import ElasticNet, Lasso, Ridge
from sklearn.preprocessing import RobustScaler, StandardScaler
from sklearn.svm import SVR

# %%

# LOGS_DIR = "EXPERIMENT_LOGS/dataset_compare/LOGS_dataset_stats_large"
LOGS_DIR_TIME = "EXPERIMENT_LOGS/segmentation/LOGS_max_envelopes"
LOGS_DIR = "EXPERIMENT_LOGS/segmentation/LOGS_max_envelopes"

groups_dict = {ERD.DATASETS_COLS: [DSC.DATASET_FILE]}
groups = dict_to_tuples(groups_dict)
targets_time = [(ERD.RUNS_COLS, QC.PRUNING_RATIO, MeanReducer())]
columns_time = groups_dict.copy()
columns_time[ERD.RUNS_COLS] = groups_dict.get(ERD.RUNS_COLS, []) + [targets_time[0][1]]

targets = []
# summary_stat_cols = [DSTC.MEAN, DSTC.STD, DSTC.SKEWNESS, DSTC.KURTOSIS]
summary_stat_cols = []
for col in summary_stat_cols:
    targets.append((ERD.DATASET_STATS_COLS, col, MeanReducer()))
    targets.append((ERD.DATASET_STATS_COLS, col, StdReducer()))

# shape_stat_cols = [DSTC.TOTAL_VAR_MEANS, DSTC.TOTAL_VAR_STDS, DSTC.AUTOCORR_MEANS, DSTC.AUTOCORR_STDS]
shape_stat_cols = []
for col in shape_stat_cols:
    targets.append((ERD.DATASET_STATS_COLS, col, CollectionReducer(reducer=MeanReducer())))
    targets.append((ERD.DATASET_STATS_COLS, col, CollectionReducer(reducer=StdReducer())))

index_stat_cols = [
    (ISTC.SEG_RANGE_STATS, SCP.MEAN),
    # (ISTC.SEG_RANGE_STATS, SCP.STD),
    # (ISTC.SEG_LOWER_STATS, SCP.MEAN),
    # (ISTC.SEG_LOWER_STATS, SCP.STD),
    # (ISTC.SEG_UPPER_STATS, SCP.MEAN),
    # (ISTC.SEG_UPPER_STATS, SCP.STD),
    # (ISTC.SEG_MID_STATS, SCP.MEAN),
    (ISTC.SEG_MID_STATS, SCP.STD),
]
for col_base, prefix in index_stat_cols:
    targets.append((ERD.INDEX_STATS_COLS, SC(col_base, prefix), MeanReducer()))

index_stat_list_cols = [
    # (ISTC.SEG_LOWER_LIST_STATS, SCP.MEAN),
    # (ISTC.SEG_LOWER_LIST_STATS, SCP.STD),
    # (ISTC.SEG_UPPER_LIST_STATS, SCP.MEAN),
    # (ISTC.SEG_UPPER_LIST_STATS, SCP.STD),
    # (ISTC.SEG_MID_LIST_STATS, SCP.MEAN),
    # (ISTC.SEG_MID_LIST_STATS, SCP.STD),
]
for col_base, prefix in index_stat_list_cols:
    for r in [StdReducer()] if prefix == SCP.MEAN else [MeanReducer(), StdReducer()]:
        reducer = CombinedReducer(reducers=[CollectionReducer(reducer=MeanReducer()), r])
        targets.append((ERD.INDEX_STATS_COLS, SC(col_base, prefix), reducer))

columns = groups_dict.copy()
for erd, col, _ in targets:
    columns[erd] = columns.get(erd, []) + [col]

add_dataset_stats = len(summary_stat_cols) > 0 or len(shape_stat_cols) > 0
add_index_stats = len(index_stat_cols) > 0 or len(index_stat_list_cols) > 0

results_time = ExperimentResults.load(logs_dir=LOGS_DIR_TIME, cols=columns_time)
results = ExperimentResults.load(
    logs_dir=LOGS_DIR,
    cols=columns,
    add_runs=False,
    add_dataset_stats=add_dataset_stats,
    add_index_stats=add_index_stats,
)

reduced_values_time = execute_reduction([results_time], targets_time, groups)
reduced_values = execute_reduction([results], targets, groups, na_replacement=pd.NA)

targets = targets_time + targets
for group in reduced_values:
    reduced_values[group] = reduced_values_time[group] + reduced_values[group]

print("# Reduced values:", len(reduced_values))

# %%

first_row = []
labels = []


def get_reduction_str(reducer: Reducer, only_last_for_combined: bool = True) -> str:
    if isinstance(reducer, CollectionReducer):
        return get_reduction_str(reducer.reducer)
    elif isinstance(reducer, CombinedReducer):
        if only_last_for_combined:
            return get_reduction_str(reducer.reducers[-1])
        return "_".join(get_reduction_str(r) for r in reducer.reducers)
    return type(reducer).__name__.lower().replace("reducer", "")


first_key = list(reduced_values.keys())[0]
for (_, col), val in zip(groups[1:], first_key[1:]):
    labels.append(str(col))
    first_row.append(val)

for (_, target_col, reducer), value in zip(targets, reduced_values[first_key]):
    reduction = get_reduction_str(reducer)
    if isinstance(reducer, CollectionReducer):
        for i, v in enumerate(value):
            labels.append(f"{str(target_col)}_{i}_{reduction}")
            first_row.append(v)
    else:
        labels.append(f"{str(target_col)}_{reduction}")
        first_row.append(value)

print("Labels for first row:")
for label, value in zip(labels, first_row):
    print(f"\t{label:<30}:\t{value}")

# %%

DATASETS = ["weather", "stocks", "synthetic"]
DS_PER_CHANNEL = 10
ds_masks = {ds: np.array([key[0].startswith(ds) for key in reduced_values.keys()]) for ds in DATASETS}
ds_channels = {ds: sorted(list({key[0] for key in reduced_values.keys() if key[0].startswith(ds)})) for ds in DATASETS}
channel_inds = np.zeros(len(reduced_values), dtype=int)
channel_names = {
    ds: [ds_channels[ds][i].split("/")[1] for i in range(0, len(ds_channels[ds]), DS_PER_CHANNEL)]
    for ds in ["weather", "stocks"]
}
channel_names["synthetic"] = [f"SD={sd}" for sd in [0.1, 1.0, 10.0]]
for i, key in enumerate(reduced_values):
    mts_dataset = key[0].split("/", 1)[0]
    channel_inds[i] = ds_channels[mts_dataset].index(key[0]) // DS_PER_CHANNEL

xs = np.zeros(shape=(len(reduced_values), len(first_row) - 1), dtype=float)
ys = np.zeros(shape=(len(reduced_values),), dtype=float)

for i, (dataset, values) in enumerate(reduced_values.items()):
    col_ind = 0
    ys[i] = values[0]
    for value in values[1:]:
        if isinstance(value, np.ndarray):
            for v in value:
                xs[i, col_ind] = v
                col_ind += 1
        else:
            xs[i, col_ind] = value
            col_ind += 1

xs = np.nan_to_num(xs, nan=0.0)
xs = np.clip(xs, a_min=-1e10, a_max=1e10)
scaler = StandardScaler()
xs = scaler.fit_transform(xs)

# %%

discard_quantiles = {"synthetic": 0.0, "weather": 0.0, "stocks": 1e-5}
filtered_mask = np.ones(xs.shape[0], dtype=bool)
for ds, ds_mask in ds_masks.items():
    discard_quantile = discard_quantiles.get(ds, 0.0)
    if discard_quantile > 0.0:
        ds_xs = xs[ds_mask]
        lower_threshold = np.quantile(ds_xs, discard_quantile, axis=0)
        upper_threshold = np.quantile(ds_xs, 1 - discard_quantile, axis=0)
        mask = ~np.any((ds_xs < lower_threshold) | (ds_xs > upper_threshold), axis=1)
        filtered_mask[ds_mask] = mask
xs = xs[filtered_mask]
ys = ys[filtered_mask]
ds_masks = {ds: ds_mask[filtered_mask] for ds, ds_mask in ds_masks.items()}
channel_inds = channel_inds[filtered_mask]


print(f"Shape of xs: {xs.shape}, ys: {ys.shape}")

# %%[markdown]
"""
Check data quality
"""

# %%
# PCA

pca = PCA(n_components=min(xs.shape[0], xs.shape[1]))
pca.fit(xs)
explained_variance = pca.explained_variance_ratio_
for i, var in enumerate(explained_variance):
    print(f"Principal Component {i + 1}: {var:.4f}")

ys_lims = (ys.min(), ys.max())
ys_range = ys_lims[1] - ys_lims[0]
cbar_lims = (max(0.0, ys_lims[0] - 0.1 * ys_range), min(1.0, ys_lims[1] + 0.1 * ys_range))


# Plot pca, with pca1 on x-axis and pca2 on y-axis, and ys for the color scale
channel_symbols = ["o", "x", "s", "*", "D"]


def plot_points(
    xs: np.ndarray,
    ys: np.ndarray,
    axis_indices: tuple[int, int] = (0, 1),
    title: str = "PCA Plot",
    x_label: str = "PCA Component 1",
    y_label: str = "PCA Component 2",
    cbar_lims: tuple[float, float] = cbar_lims,
    channel_inds: np.ndarray = None,
    channel_names: list[str] = None,
    channel_symbols: list[str] = channel_symbols,
):
    plt.figure(figsize=(10, 8))
    ind1, ind2 = axis_indices

    if channel_inds is None:
        channel_inds = np.zeros(xs.shape[0], dtype=int)
    # Use different markers for each channel
    for channel_idx in np.unique(channel_inds):
        mask = channel_inds == channel_idx
        if np.any(mask):
            marker = channel_symbols[channel_idx % len(channel_symbols)]
            plt.scatter(
                xs[mask, ind1],
                xs[mask, ind2],
                c=ys[mask],
                marker=marker,
                cmap="viridis",
                vmin=cbar_lims[0],
                vmax=cbar_lims[1],
                label=channel_names[channel_idx] if channel_names else f"Channel {channel_idx}",
            )

    plt.colorbar(label=str(targets[0][1]))
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.title(title)
    plt.grid()
    plt.legend(title="Channels", loc="best")
    plt.show()


for ds, ds_mask in ds_masks.items():
    xs_pca = pca.transform(xs)
    # plot_points(xs_pca[ds_mask], ys[ds_mask], title=f"PCA Plot for {ds.capitalize()}")
    axis_indices = (0, 1)
    plot_points(
        xs[ds_mask],
        ys[ds_mask],
        title=f"Points in Original Space for {ds.capitalize()}",
        x_label=labels[len(groups) + axis_indices[0]],
        y_label=labels[len(groups) + axis_indices[1]],
        axis_indices=axis_indices,
        channel_inds=channel_inds[ds_mask],
        channel_names=channel_names.get(ds, None),
    )


# %%[markdown]
"""
Cross validation
"""

# %%


def cross_validation(
    xs: np.ndarray, ys: np.ndarray, value_datasets: list[str], model, filtered_mask: np.ndarray | None = None
) -> dict[str, list[float]]:
    subsets = ["train", "val"]
    metrics = {ds: {s: {metric: 0 for metric in ["rmse", "percent_error"]} for s in subsets} for ds in DATASETS}
    for dataset in DATASETS:
        val_mask = np.array([key[0].startswith(dataset) for key in value_datasets])
        if filtered_mask is not None:
            val_mask = val_mask[filtered_mask]
        train_mask = ~val_mask

        train_xs = xs[train_mask]
        train_ys = ys[train_mask]
        val_xs = xs[val_mask]
        val_ys = ys[val_mask]

        model.fit(train_xs, train_ys)
        train_predictions = model.predict(train_xs)
        val_predictions = model.predict(val_xs)

        for subset, subs_metrics in metrics[dataset].items():
            if subset == "train":
                predictions = train_predictions
                actuals = train_ys
            else:
                predictions = val_predictions
                actuals = val_ys

            rmse = np.sqrt(np.mean((predictions - actuals) ** 2))
            percent_error = np.mean(np.abs((predictions - actuals) / actuals)) * 100

            subs_metrics["rmse"] = rmse
            subs_metrics["percent_error"] = percent_error
        metrics[dataset]["coeffs"] = model.coef_ if hasattr(model, "coef_") else None
        metrics[dataset]["intercept"] = model.intercept_ if hasattr(model, "intercept_") else None

    return metrics


# %%[markdown]
"""
Models
"""

# %%

value_datasets = list(reduced_values.keys())

models = {
    # "ols": LinearRegression(),
    "ridge_a=0.1": Ridge(alpha=0.1),
    "ridge_a=1.0": Ridge(alpha=1.0),
    "ridge_a=10.0": Ridge(alpha=10.0),
    "lasso_a=0.1": Lasso(alpha=0.1),
    "lasso_a=1.0": Lasso(alpha=0.1),
    "lasso_a=10.0": Lasso(alpha=0.1),
    "elastic_net_a=0.1_l1=0.5": ElasticNet(alpha=0.1, l1_ratio=0.5),
    "elastic_net_a=1.0_l1=0.5": ElasticNet(alpha=1.0, l1_ratio=0.5),
    "elastic_net_a=10.0_l1=0.5": ElasticNet(alpha=10.0, l1_ratio=0.5),
    "svr_linear": SVR(kernel="linear"),
    "svr_rbf": SVR(kernel="rbf"),
    "random_forest_n=100": RandomForestRegressor(n_estimators=100, random_state=42),
    "gradient_boosting_n=100": GradientBoostingRegressor(n_estimators=100, random_state=42),
    "adaboost_n=100": AdaBoostRegressor(n_estimators=100, random_state=42),
}
metrics_dict = {}

for name, model in models.items():
    metrics_dict[name] = cross_validation(xs, ys, value_datasets, model, filtered_mask=filtered_mask)

# %%

for metric_name, metrics in metrics_dict.items():
    print("---------------------------")
    print(f"Model: {metric_name}")
    for dataset, ds_metrics in metrics.items():
        print(f"Validation dataset: {dataset}")
        for subset in ["train", "val"]:
            subset_metrics = ds_metrics[subset]
            print(f"\t{subset}:")
            for metric in ["rmse", "percent_error"]:
                value = subset_metrics[metric]
                print(f"\t\t{metric}: {value:.6f}")

# Visualize percentage errors per model with bar plot
plt.figure(figsize=(12, 8))

# Extract validation percentage errors for each model and dataset
model_names = list(metrics_dict.keys())
x = np.arange(len(model_names))
width = 0.25  # Width of bars

for i, dataset in enumerate(DATASETS):
    percent_errors = [metrics_dict[model][dataset]["val"]["percent_error"] for model in model_names]
    plt.bar(x + i * width, percent_errors, width, label=f"{dataset}")

plt.xlabel("Models")
plt.ylabel("Validation Percentage Error (%)")
plt.title("Percentage Error by Model and Dataset")
plt.xticks(x + width, model_names, rotation=45, ha="right")
plt.legend()
plt.tight_layout()
plt.yscale("linear")
plt.grid(axis="y", linestyle="--", alpha=0.7)
plt.show()

# %%

# cross_validation(xs, ys, value_datasets, models["ridge_a=10.0"], verbose=True)

for name, model in models.items():
    if any(name.startswith(prefix) for prefix in ["ridge", "lasso", "elastic_net"]):
        print(f"Model: {name}")
        coef_labels = labels[1 + len(groups) - 1 :]
        for dataset, ds_metrics in metrics_dict[name].items():
            print(f"CV weights for: {dataset}")
            weights = dict(zip(coef_labels, ds_metrics["coeffs"]))
            weights["intercept"] = ds_metrics["intercept"]
            print(json.dumps(weights, indent=4))
    elif name.startswith("svr"):
        print(f"Model: {name}")
        print(f"Support vectors: {model.support_vectors_.shape[0]}")
        print(f"Number of support vectors per class: {model.n_support_}")
        print(f"Dual coefficients: {model.dual_coef_}")
    elif name.startswith("adaboost"):
        print(f"Model: {name}")
        print(f"Number of estimators: {model.n_estimators}")
        print(f"Feature importances: {model.feature_importances_}")

# %%
