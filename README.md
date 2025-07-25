# Exact Multivariate Variable Length Similarity Search

This repository contains the code for the Master Thesis "Exact Multivariate Variable Length Similarity Search" by Balázs-István Pelok, under the supervision of Odysseas Papapetrou and Jens d'Hondt.

## Setup

[`scripts/sh/setup.sh`](https://github.com/BPelokTUE/ULISSE_MTS/blob/main/scripts/sh/setup.sh) can be used to install required dependencies on Ubuntu. The same script is used for the docker image. Otherwise, install the required dependencies on your system:
1. The following dependencies are required for running the project:
    - [Boost](https://www.boost.org/) for some generic missing features (distributions, easier hash function implementation)
    - [OpenMP](https://www.openmp.org/) for parallel processing
    - [FFTW3](https://www.fftw.org/index.html) for fast Fourier transforms
2. For building:
    - [CMake](https://cmake.org/)
    - [Ninja-build](https://ninja-build.org/) (optional, recommended)
3. For Python glue scripts (runner and visualizations):
    - At least Python 3.11 (Python 3.13 was used for development)
    - The packages in [`scripts/py/requirements.txt`](https://github.com/BPelokTUE/MULISSE/blob/main/scripts/py/requirements.txt)

The project requires C++ 23 and has been tested with Clang 20.1.6 on macOS 14.6.2 and GCC 15.1.0 on [gcc:latest](https://hub.docker.com/_/gcc) (Debian).

## Running the project

Before building the project, first create a `local_setting.json` file. You can use [`local_settings_template.json`](https://github.com/BPelokTUE/ULISSE_MTS/blob/main/local_settings_template.json).

To build the project (with cmake and ninja-build) run [`scripts/sh/build.sh`](https://github.com/BPelokTUE/ULISSE_MTS/blob/main/scripts/sh/build.sh).

Once built, the executable `build/mulisse` is created. Run with the `-h` or `--help` flag to see the full help message, which will show the available subcommands. Running `mulisse <subcommand> -h` will display the options available for the given subcommand.

Runs generate various artifacts (indexes, precomputed ffts) and `.csv` log files, by default in the `./DATA` and `./LOGS` directories respectively. The output location can be overwritten using the `--data` and `--logs` keywords for all subcommands.

**Running the subcommands directly can be cumbersome due to the large number of keyword arguments, therefore using [`scripts/py/run_mulisse.py`](https://github.com/BPelokTUE/MULISSE/blob/main/scripts/py/run_mulisse.py) is recommended in general**.

### Using the Python Runner

The runner executes (sets of) of grid searches, optionally cleans up the artifacts, and logs the execution of each subcommand, as well as the comparison of run results into `LOGS/commands.txt`. The runner executes the requested subcommands in the following order
- Dataset creations
- Dataset analyses
- Query set creations
- Query set analyses
- Sequential scans
- Index creations
- Index analyses
- Index-based searches

The grid searches are defined in `.json` files with numerous examples in `scripts/run_configs`. Most accepted keywords in the configuration files expect lists of values, and all combinations of all lists will be iterated over. For more complex searches, profiles within the configuration file can be defined as keys of dictionaries, and the keywords with dictionary values will iterate over the lists of each profile independently. For a list of all accepted keywords see [`docs/run_config.md`](https://github.com/BPelokTUE/MULISSE/blob/main/docs/run_config.md).

### Using the Docker Container

The script [`scripts/sh/run_mulisse_docker.sh`](https://github.com/BPelokTUE/MULISSE/blob/main/scripts/sh/run_mulisse_docker.sh) runs `run_mulisse.py` with the provided configuration files, in a docker container, and saves the logs into subdirectories corresponding to the run configuration within `EXPERIMENT_LOGS`.

### Recreating the Plots

To recreate the plots in the thesis, the run configurations can be found in `scripts/run_configs/thesis`, while the code to make the plots is in [`scripts/py/visualization/thesis_visualizations.py`](https://github.com/BPelokTUE/MULISSE/blob/main/scripts/py/visualization/thesis_visualizations.py). The visualizer script can be run cell-by-cell using [ipykernel](https://pypi.org/project/ipykernel/), and assumes the experiments were run using `scripts/sh/run_mulisse_docker.sh` (the logs are in `EXPERIMENT_LOGS/thesis`).

## Codebase

The codebase is documented through docstrings, and the documentation can be generated using [Doxygen](https://www.doxygen.nl/) by running `doxygen`, which creates the docs under `docs/html`.
