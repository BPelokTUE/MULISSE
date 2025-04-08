import argparse
import os
import re
from typing import Any

import pandas as pd

from common.columns import QueryColumn as QC


def parse_ulisse_log(txt_log_path: str, runs_df: pd.DataFrame, series_length: int) -> pd.DataFrame:
    query_pattern = re.compile(
        r"Result query for query number (\d+) of file .+\s*\n"
        r"Query time: (\d+\.\d+)\s*\n"
        r"Query disk time: (\d+\.\d+)\s*\n"
        r"Number of disk page access: \d+\.\d+\s*\n"
        r"Number of record read on total: (\d+)/\d+\s*\n"
        r"Pruning power: \d+\.\d+\s*\n"
        r"Abandoning power: (\d+\.\d+)\s*\n"
        r" Size of query: (\d+)\s+Position of match in the raw dataset: (\d+)\s+Distance: (\d+\.\d+)\s*",
    )

    with open(txt_log_path, "r") as file:
        log_content = file.read()

    matches = query_pattern.findall(log_content)

    base_run_id = runs_df[str(QC.ID)].max() + 1 if not runs_df.empty else 0
    base_search_setting_id = runs_df[str(QC.SETTINGS_ID)].max() + 1 if not runs_df.empty else 0

    parsed_runs: dict[QC, list[Any]] = {
        str(QC.ID): [],
        str(QC.SETTINGS_ID): [],
        str(QC.QUERY_ID): [],
        str(QC.QUERY_CHANNELS): [],
        str(QC.QUERY_LENGTH): [],
        str(QC.TOTAL_TIME_S): [],
        str(QC.IO_TIME_S): [],
        str(QC.NUM_ENTRIES_EXAMINED): [],
        str(QC.ABANDONING_RATE): [],
        str(QC.RESULT_SET_TS_INDICES): [],
        str(QC.RESULT_SET_TS_POSITIONS): [],
        str(QC.RESULT_SET_DISTANCES): [],
    }

    for match in matches:
        parsed_runs[str(QC.ID)].append(base_run_id)
        base_run_id += 1
        parsed_runs[str(QC.SETTINGS_ID)].append(base_search_setting_id)
        parsed_runs[str(QC.QUERY_ID)].append(int(match[0]) - 1)
        parsed_runs[str(QC.QUERY_CHANNELS)].append(1)
        parsed_runs[str(QC.QUERY_LENGTH)].append(int(match[5]))
        parsed_runs[str(QC.TOTAL_TIME_S)].append(float(match[1]))
        parsed_runs[str(QC.IO_TIME_S)].append(float(match[2]))
        parsed_runs[str(QC.NUM_ENTRIES_EXAMINED)].append(int(match[3]))
        parsed_runs[str(QC.ABANDONING_RATE)].append(float(match[4]))

        raw_position = int(match[6])
        parsed_runs[str(QC.RESULT_SET_TS_INDICES)].append(raw_position // series_length)
        parsed_runs[str(QC.RESULT_SET_TS_POSITIONS)].append(raw_position % series_length)
        parsed_runs[str(QC.RESULT_SET_DISTANCES)].append(float(match[7]))

    parsed_runs_df = pd.DataFrame(parsed_runs)
    return pd.concat([runs_df, parsed_runs_df], ignore_index=True)


def parse_scan_log(
    txt_log_path: str, runs_df: pd.DataFrame, series_length: int, num_entries: int, abandoning_rate: bool = True
) -> pd.DataFrame:
    query_raw_text = (
        r"Results for query number (\d+) of file .+:\s*\n"
        r"Query time: (\d+\.\d+)\s*\n"
        r"Size of query: (\d+)\s*\n"
        + (r"Abandoning power: (\d+\.\d+)\s*\n" if abandoning_rate else "")
        + r"Nearest neighbors:\n"
        r"Distance: (\d+\.\d+), Location: (\d+)\s*"
    )
    query_pattern = re.compile(query_raw_text)

    with open(txt_log_path, "r") as file:
        log_content = file.read()

    matches = query_pattern.findall(log_content)

    base_run_id = runs_df[str(QC.ID)].max() + 1 if not runs_df.empty else 0
    base_search_setting_id = runs_df[str(QC.SETTINGS_ID)].max() + 1 if not runs_df.empty else 0

    parsed_runs: dict[QC, list[Any]] = {
        str(QC.ID): [],
        str(QC.SETTINGS_ID): [],
        str(QC.QUERY_ID): [],
        str(QC.QUERY_CHANNELS): [],
        str(QC.QUERY_LENGTH): [],
        str(QC.TOTAL_TIME_S): [],
        str(QC.NUM_ENTRIES_EXAMINED): [],
        str(QC.ABANDONING_RATE): [],
        str(QC.RESULT_SET_TS_INDICES): [],
        str(QC.RESULT_SET_TS_POSITIONS): [],
        str(QC.RESULT_SET_DISTANCES): [],
    }

    for match in matches:
        parsed_runs[str(QC.ID)].append(base_run_id)
        base_run_id += 1
        parsed_runs[str(QC.SETTINGS_ID)].append(base_search_setting_id)
        parsed_runs[str(QC.QUERY_ID)].append(int(match[0]))
        parsed_runs[str(QC.QUERY_CHANNELS)].append(1)
        parsed_runs[str(QC.QUERY_LENGTH)].append(int(match[2]))
        parsed_runs[str(QC.TOTAL_TIME_S)].append(float(match[1]))
        parsed_runs[str(QC.NUM_ENTRIES_EXAMINED)].append(num_entries)
        parsed_runs[str(QC.ABANDONING_RATE)].append(float(match[3]) if abandoning_rate else 0.0)

        raw_position = int(match[4 + abandoning_rate])
        parsed_runs[str(QC.RESULT_SET_TS_INDICES)].append(raw_position // series_length)
        parsed_runs[str(QC.RESULT_SET_TS_POSITIONS)].append(raw_position % series_length)
        parsed_runs[str(QC.RESULT_SET_DISTANCES)].append(float(match[3 + abandoning_rate]))

    parsed_runs_df = pd.DataFrame(parsed_runs)
    return pd.concat([runs_df, parsed_runs_df], ignore_index=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-n", "--series_length", type=int, required=True, help="Length of the time series in the dataset"
    )
    parser.add_argument("-m", "--num_series", type=int, required=True, help="Number of time series in the dataset")
    parser.add_argument("-t", "--text_logs_dir", type=str, required=True, help="Directory containing the .txt logs")
    parser.add_argument("-c", "--csv_logs_dir", type=str, required=True, help="Directory containing the .csv logs")
    parser.add_argument(
        "-i", "--inplace", action="store_true", help="If set, the .csv logs will be overwritten with the merged data"
    )
    args = parser.parse_args()

    runs_df = pd.read_csv(os.path.join(args.csv_logs_dir, "runs.csv"))

    for file in ["ResultsULISSE_single.txt", "ResultsULISSE_parallel.txt"]:
        txt_path = os.path.join(args.text_logs_dir, file)
        if os.path.exists(txt_path):
            runs_df = parse_ulisse_log(txt_path, runs_df, args.series_length)
    for file in ["ResultsEDEA.txt", "ResultsMASS.txt"]:
        txt_path = os.path.join(args.text_logs_dir, file)
        if os.path.exists(txt_path):
            runs_df = parse_scan_log(txt_path, runs_df, args.series_length, args.num_series, file == "ResultsEDEA.txt")

    df_name = "runs.csv" if args.inplace else "runs_merged.csv"
    runs_df.to_csv(os.path.join(args.csv_logs_dir, df_name), index=False)
