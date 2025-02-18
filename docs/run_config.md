# Configuration File Documentation

This document describes the values used in configuration files for [`run_mulisse.py`](../scripts/py/run_mulisse.py). For an example configuration see [`default_config.json`](../scripts/run_configs/default_config.json). 

## Configuration Values

### `run_synthetic`
- **Type**: `boolean`
- **Description**: Whether to run on synthetic data or not.

### `csv_data_dirs`
- **Type**: `list[string]`
- **Description**: List of directories within `local_settings.CSV_DIR`.

### `dataset_sizes`
- **Type**: `list[int]`
- **Description**: List of dataset sizes.

### `series_lengths`
- **Type**: `list[int]`
- **Description**: List of series lengths.

### `syn_num_channels`
- **Type**: `list[int]`
- **Description**: List of numbers of channels for synthetic data.

### `query_set_sizes`
- **Type**: `list[int]`
- **Description**: List of query set sizes.

### `l_range_ratios`
- **Type**: `list[list[float]]`
- **Description**: Ratios of `l_min` and `l_max` compared to `series_length`.

### `used_channel_ratios`
- **Type**: `list[float]`
- **Description**: Ratio of number of used channels compared to number of channels. `0.0` means completely random selection of channels, `1.0` means all channels.

### `index_types`
- **Type**: `list[string]`
- **Description**: List of index types.

### `isax_split_strategies`
- **Type**: `list[string]`
- **Description**: List of iSAX split strategies.

### `isax_breakpoint_strategies`
- **Type**: `list[string]`
- **Description**: List of iSAX breakpoint strategies.

### `isax_leaf_capacities`
- **Type**: `list[int]`
- **Description**: List of iSAX leaf capacities.

### `isax_start_bit_numbers`
- **Type**: `list[int]`
- **Description**: List of iSAX first layer bit numbers.

### `num_segments`
- **Type**: `list[int]`
- **Description**: Ratios of segment length compared to `series_length`.

### `envelope_size_ratios`
- **Type**: `list[float]`
- **Description**: Ratios of envelope size compared to `series_length - l_min + 1`.

### `scan_methods`
- **Type**: `list[string]`
- **Description**: List of scan methods.

### `distance_measures`
- **Type**: `list[string]`
- **Description**: List of distance measures.

### `early_abandon`
- **Type**: `list[boolean]`
- **Description**: List of early abandon flags.

### `precalculate_ffts`
- **Type**: `list[boolean]`
- **Description**: List describing whether to use precalculated FFTs or not. The FFT file path is determined automatically.

### `search_types`
- **Type**: `list[string]`
- **Description**: List of search types.

### `search_ks`
- **Type**: `list[int]`
- **Description**: List of `k` values for kNN search.

### `search_rs`
- **Type**: `list[float]`
- **Description**: List of `r` values for r-range search.

### `search_approx`
- **Type**: `list[boolean]`
- **Description**: List of approximate search flags.

### `search_raw`
- **Type**: `list[boolean]`
- **Description**: List of raw (unnormalized) search flags.
