# MULISSE

An extension of the [ULISSE](https://helios2.mi.parisdescartes.fr/~mlinardi/ULISSE.html) similarity search algorithm for multivariate time series with ad-hoc channel selection.

## Setup

[`scripts/sh/setup.sh`](https://github.com/BPelokTUE/ULISSE_MTS/blob/main/scripts/sh/setup.sh) can be used to install required dependencies on Ubuntu (or other Unix systems with `apt` package manager?). The same script is used for the docker image. Otherwise, install the required dependencies on your system:
1. The following dependencies are required for running the project:
    - [Boost](https://www.boost.org/) for some generic missing features (distributions, easier hash function implementation)
    - [OpenMP](https://www.openmp.org/) for parallel processing
    - [FFTW3](https://www.fftw.org/index.html) for fast Fourier transforms
2. And for building:
    - [CMake](https://cmake.org/)
    - [Ninja-build](https://ninja-build.org/) (optional, recommended)

The project requires C++ 23 and has been tested with Clang 19.1.7 on macOS 14.6.1 and GCC 14.2.0 on [gcc:latest](https://hub.docker.com/_/gcc) (Debian).

## Running the project

To build the project (with cmake and ninja-build) run [`scripts/sh/build.sh`](https://github.com/BPelokTUE/ULISSE_MTS/blob/main/scripts/sh/build.sh).

Once built, the executable `build/mulisse` is created. Run with the `-h` or `--help` flag to see the full help message, which will show the following available subcommands:
```
Subcommands:
  create_ds                   Create random walk dataset
  create_qs                   Create queries from dataset
  index                       Construct MULISSE index
  search                      Search using MULISSE
```
Passing the subcommand as the first parameter to the executable executes it (e.g. `mulisse search`). The subcommand followed by the help flag displays the options specific to that command. For an example use of the commands, see [`scripts/sh/run.sh`](https://github.com/BPelokTUE/ULISSE_MTS/blob/main/scripts/sh/run.sh). Results from the subcommands will be logged into `.csv` files inside of the `LOGS` directory. See [`scripts/py/check_results.py`](https://github.com/BPelokTUE/ULISSE_MTS/blob/main/scripts/py/check_results.py) for an example of parsing these log files.

### Create Random Walk Dataset (`mulisse create_ds`)

Creates a random walk dataset in the `DATA` directory. A new entry will be created for the dataset in `LOGS/dataset_settings.csv`.

```
Create random walk dataset
Usage: build/mulisse create_ds [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -d,--dataset TEXT REQUIRED  Output dataset path
  --noise FLOAT [1]           Random walk standard deviation
  -z,--zero_start             Start the random walk from zero
  -n,--num_series UINT:POSITIVE_INTEGER REQUIRED
                              Number of series
  -m,--series_len UINT:POSITIVE_INTEGER REQUIRED
                              Length of series
  -c,--num_channels UINT:POSITIVE_INTEGER REQUIRED
                              Number of channels
  -S,--seed INT [1]           Random seed
```

### Create a Dataset from CSV files (`mulisse parse_csv`)

Creates a dataset from a list of `.csv` files, where each file corresponds to one channel. If time series are too long they are truncated to the passed length. Time series are discarded if any of the following applies:
- Any of the channels is too short
- Any of the channels contains cells which cannot be converted to `Real`
- Any of the channels contains at least one subsequence of length `low_sd_len` where the standard deviation is lower than `MIN_SUBS_SIGMA=1e-3`

```
Create dataset from CSV
Usage: ./mulisse parse_csv [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -i,--input TEXT ... REQUIRED
                              Input CSV file paths, in the order of channels
  -d,--dataset TEXT REQUIRED  Output dataset path
  -l,--low_sd_len UINT REQUIRED
                              Discard time series with any low standard deviation subsequence of this length. Pass 0 to disable.
  -m,--series_len UINT:POSITIVE_INTEGER REQUIRED
                              Length of series
```

### Create Queries (`mulisse create_qs`)

Creates a set of queries by extracting subsequences from a dataset file and adding random Gaussian noise. If the neither the number of channels to be used, nor the channel mask is passed, the selection of channels in each query will be random, with at least one channel present in each query.

```
Create queries from dataset
Usage: ./mulisse create_qs [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -d,--dataset TEXT REQUIRED  Dataset to use
  -q,--query TEXT REQUIRED    Output query path
  --noise FLOAT [1]           Query noise
  -m,--series_len UINT:POSITIVE_INTEGER REQUIRED
                              Length of series
  -c,--num_channels UINT:POSITIVE_INTEGER REQUIRED
                              Number of channels
  -Q,--num_queries UINT:POSITIVE_INTEGER REQUIRED
                              Number of queries
  -l,--lengths UINT:POSITIVE_INTEGER ... REQUIRED
                              Query lengths
  -S,--seed INT [1]           Random seed
  -u,--used_channels UINT [0] 
                              Number of channels to use for queries. 0 by default, meaning that the number of used channels is selected randomly for each query.
  -M,--channel_mask BOOLEAN [{}]  ...
                              Mask for which channels to use in the queries. Overrides used_channels if provided.
```

### Index a Dataset (`mulisse index`)

Create a similarity search index from a dataset and save it into the `DATA` directory. Optionally can also save the FFT representations of the time series in the dataset (*NOTE*: this only works if the index creates one enveloper per time series, i.e. when `pos_per_env > series_len - l_min`). A new entry will be created for the index in `LOGS/index_settings.csv`.

```
Construct MULISSE index
Usage: build/mulisse index [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -i,--index TEXT REQUIRED    Output index path
  -d,--dataset TEXT REQUIRED  Dataset to use
  -F,--ffts TEXT              Path to save FFTs; if not provided, FFTs will not be calculated
  -m,--series_len UINT:POSITIVE_INTEGER REQUIRED
                              Length of series
  -c,--num_channels UINT:POSITIVE_INTEGER REQUIRED
                              Number of channels
  -f,--format TEXT:{binary,json} [binary] 
                              Index format
  -t,--index_type TEXT:{isax_envelope,sequential_scan,sequential,isax,scan} [isax_envelope] 
                              Index type
  -S,--split_strategy TEXT:{double_round_robin,entropy_maximizing,drr,em} [double_round_robin] 
                              Split strategy
  -B,--breakpoint_strategy TEXT:{equiprobable} [equiprobable] 
                              Breakpoint strategy
  -l,--l_min UINT:POSITIVE_INTEGER REQUIRED
                              Minimum length of subsequences
  -L,--l_max UINT:POSITIVE_INTEGER REQUIRED
                              Maximum length of subsequences
  -s,--segment_len UINT:POSITIVE_INTEGER REQUIRED
                              Segment length
  -p,--pos_per_env UINT:POSITIVE_INTEGER REQUIRED
                              Positions per envelope
  -C,--leaf_capacity UINT:POSITIVE_INTEGER REQUIRED
                              Leaf capacity
  --raw                       Do not normalize
```

### Run Similarity Search (`mulisse search`)

Run similarity search. A new entry will be created in `LOGS/search_settings.csv` describing the search method, and for each executed query a new entry is written into `LOGS/runs.csv`.

```
Search using MULISSE
Usage: ./mulisse search [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -i,--index TEXT             Index file path
  -d,--dataset TEXT REQUIRED  Dataset file path
  -q,--query TEXT REQUIRED    Query file path
  -F,--ffts TEXT              Path to load FFTs from; if not provided, FFTs will not be loaded
  -o,--out TEXT REQUIRED      Output file path
  -c,--num_channels UINT:POSITIVE_INTEGER REQUIRED
                              Number of channels
  -m,--series_len UINT:POSITIVE_INTEGER REQUIRED
                              Length of series
  -t,--method_type TEXT:{isax_envelope,sequential_scan,sequential,isax,scan} [isax_envelope] 
                              Search method type
  -f,--format TEXT:{binary,json} [binary] 
                              Index format
  -D,--distance TEXT:{ed,mass,euclidean} [ed] 
                              Distance measure
  --early_abandon             Use early abandoning
  --approx                    Approximate search
  --raw                       Do not normalize
  -T,--search_type TEXT:{knn,r_range} REQUIRED
                              Search type
  -k,--k UINT:POSITIVE_INTEGER [1] 
                              Number of nearest neighbors for kNN
  -r,--range FLOAT:POSITIVE_FLOAT [1] 
                              Range for range search

```

## Codebase

The codebase is documented through docstrings, and the documentation can be generated using [Doxygen](https://www.doxygen.nl/) by running `doxygen`, which creates the docs under `docs/html`.
