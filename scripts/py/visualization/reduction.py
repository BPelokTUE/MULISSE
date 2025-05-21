import os
from enum import Enum, auto
from typing import Any

import numpy as np
import pandas as pd
from pydantic import BaseModel, Field
from scripts.py.common.columns import Column, StatsColumn
from scripts.py.common.columns import DatasetSettingsColumn as DSC
from scripts.py.common.columns import DatasetStatsColumn as DSTC
from scripts.py.common.columns import IndexSettingsColumn as ISC
from scripts.py.common.columns import IndexStatsColumn as ISTC
from scripts.py.common.columns import QueryColumn as QC
from scripts.py.common.columns import QuerySetSettingsColumn as QSC
from scripts.py.common.columns import QueryStatsColumn as QSTC
from scripts.py.common.columns import SearchSettingsColumn as SSC
from scripts.py.common.columns import StatsColumnPrefix as SCP
from scripts.py.common.utils import COLS_FOR_METHOD_NAME, define_method_name_col


class ExperimentResultDataframe(Enum):
    DATASETS_COLS = auto()
    DATASET_STATS_COLS = auto()
    QUERY_SETS_COLS = auto()
    INDEXES_COLS = auto()
    METHODS_COLS = auto()
    RUNS_COLS = auto()
    INDEX_STATS_COLS = auto()
    QUERY_STATS_COLS = auto()

    def __str__(self):
        return self.name.lower()


ERD = ExperimentResultDataframe

REQUIRED_COLS = {
    ERD.DATASETS_COLS: [str(DSC.DATASET_FILE)],
    ERD.DATASET_STATS_COLS: [str(DSTC.DATASET_FILE)],
    ERD.QUERY_SETS_COLS: [str(QSC.DATASET_FILE), str(QSC.QUERY_FILE)],
    ERD.INDEXES_COLS: [str(ISC.DATASET_FILE), str(ISC.INDEX_FILE), str(ISC.FFTS_FILE)],
    ERD.METHODS_COLS: [
        str(SSC.DATASET_FILE),
        str(QSC.QUERY_FILE),
        str(SSC.INDEX_FILE),
        str(SSC.FFTS_FILE),
        str(SSC.ID),
    ],
    ERD.RUNS_COLS: [str(QC.SETTINGS_ID)],
    ERD.INDEX_STATS_COLS: [str(ISTC.INDEX_FILE)],
    ERD.QUERY_STATS_COLS: [str(QSTC.DATASET_FILE)],
}

CSV_FILES = {
    ERD.DATASETS_COLS: DSC.get_csv_name(),
    ERD.DATASET_STATS_COLS: DSTC.get_csv_name(),
    ERD.QUERY_SETS_COLS: QSC.get_csv_name(),
    ERD.INDEXES_COLS: ISC.get_csv_name(),
    ERD.METHODS_COLS: SSC.get_csv_name(),
    ERD.RUNS_COLS: QC.get_csv_name(),
    ERD.INDEX_STATS_COLS: ISTC.get_csv_name(),
    ERD.QUERY_STATS_COLS: QSTC.get_csv_name(),
}

MERGED_COL_NAME_SEP = "::"


def get_merged_col_name(df_name: ERD, col_name: str) -> str:
    return f"{str(df_name)}{MERGED_COL_NAME_SEP}{col_name}"


def rename_df_columns(df: pd.DataFrame, df_name: ERD) -> pd.DataFrame:
    return df.rename(columns={col: get_merged_col_name(df_name, col) for col in df.columns})


class ExperimentResults(BaseModel):
    logs_dir: str
    datasets_df: pd.DataFrame
    dataset_stats_df: pd.DataFrame
    query_sets_df: pd.DataFrame
    indexes_df: pd.DataFrame
    methods_df: pd.DataFrame
    runs_df: pd.DataFrame
    index_stats_df: pd.DataFrame
    query_stats_df: pd.DataFrame
    add_runs: bool = True
    add_dataset_stats: bool = False
    add_query_stats: bool = False
    add_index_stats: bool = False

    class Config:
        arbitrary_types_allowed = True

    @classmethod
    def add_extra_cols_for_pruning_ratio(
        cls,
        logs_dir: str,
        extra_cols: dict[ERD, list[str]],
        act_cols: dict[ERD, list[str]],
    ):
        runs_header = pd.read_csv(os.path.join(logs_dir, CSV_FILES[ERD.RUNS_COLS]), nrows=0)
        if str(QC.PRUNING_RATIO) in runs_header.columns:
            return

        extra_cols[ERD.DATASETS_COLS] += [str(DSC.NUM_SERIES), str(DSC.SERIES_LENGTH)]
        extra_cols[ERD.INDEXES_COLS] += [str(ISC.POS_PER_ENV)]
        extra_cols[ERD.METHODS_COLS] += [str(SSC.SEARCH_METHOD)]
        extra_cols[ERD.RUNS_COLS] += [str(QC.ID)]
        act_cols[ERD.RUNS_COLS].remove(str(QC.PRUNING_RATIO))

        if str(QC.NUM_ENTRIES_EXAMINED) in runs_header.columns:
            extra_cols[ERD.RUNS_COLS] += [str(QC.NUM_ENTRIES_EXAMINED), str(QC.QUERY_LENGTH)]
            indexes_header = pd.read_csv(os.path.join(logs_dir, CSV_FILES[ERD.INDEXES_COLS]), nrows=0)
            if str(ISC.NUM_ENTRIES) in indexes_header.columns:
                extra_cols[ERD.INDEXES_COLS] += [str(ISC.NUM_ENTRIES)]
        else:  # Handle case for backward compatibility
            extra_cols[ERD.INDEXES_COLS] += [str(ISC.L_MIN), str(ISC.L_MAX)]
            extra_cols[ERD.RUNS_COLS] += [str(QC.NUM_TS_EXAMINED)]

    def add_pruning_ratio_column(self):
        if str(QC.PRUNING_RATIO) in self.runs_df.columns:
            return

        merged_df = self.get_merged_df()
        dsc_num_series = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.NUM_SERIES))
        dsc_series_length = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.SERIES_LENGTH))
        ssc_search_method = get_merged_col_name(ERD.METHODS_COLS, str(SSC.SEARCH_METHOD))
        isc_pos_per_env = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.POS_PER_ENV))
        isc_num_entries = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.NUM_ENTRIES))

        if str(QC.NUM_ENTRIES_EXAMINED) in self.runs_df.columns:
            if str(ISC.NUM_ENTRIES) not in self.indexes_df.columns:  # Handle case for backward compatibility
                qc_query_length = get_merged_col_name(ERD.RUNS_COLS, str(QC.QUERY_LENGTH))
                merged_df["num_relevant_entries"] = merged_df[dsc_num_series] * np.where(
                    merged_df[ssc_search_method].str.contains("env"),
                    (merged_df[dsc_series_length] - merged_df[qc_query_length] + merged_df[isc_pos_per_env])
                    // merged_df[isc_pos_per_env],
                    np.where(
                        merged_df[ssc_search_method].str.contains("isax"),
                        merged_df[dsc_series_length] - merged_df[qc_query_length] + 1,
                        1,
                    ),
                )
            else:
                merged_df["num_relevant_entries"] = merged_df[isc_num_entries]

            qc_num_entries_examined = get_merged_col_name(ERD.RUNS_COLS, str(QC.NUM_ENTRIES_EXAMINED))
            qc_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.ID))

            merged_df[str(QC.PRUNING_RATIO)] = np.clip(
                1.0 - merged_df[qc_num_entries_examined] / merged_df["num_relevant_entries"], 0.0, 1.0
            )
        else:  # Handle case for backward compatibility
            isc_l_min = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.L_MIN))
            isc_l_max = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.L_MAX))
            qc_num_ts_examined = get_merged_col_name(ERD.RUNS_COLS, str(QC.NUM_TS_EXAMINED))
            qc_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.ID))

            # MULISSE counts one examined series per envelope, iSAX counts one per subsequence
            merged_df["num_start"] = merged_df[dsc_series_length] - merged_df[isc_l_min] + merged_df[isc_pos_per_env]
            merged_df["l_range"] = merged_df[isc_l_max] - merged_df[isc_l_min] + 1
            merged_df["num_subs"] = merged_df["l_range"] * (
                (merged_df[dsc_series_length] - merged_df[isc_l_max] + 1) + (merged_df["l_range"] - 1) / 2
            )

            merged_df["num_series_multiplier"] = np.where(
                merged_df[ssc_search_method].str.contains("isax_envelope"),
                merged_df["num_start"] // merged_df[isc_pos_per_env],
                np.where(merged_df[ssc_search_method].str.contains("isax"), merged_df["num_subs"], 1.0),
            )
            merged_df[str(QC.PRUNING_RATIO)] = 1.0 - merged_df[qc_num_ts_examined] / (
                merged_df[dsc_num_series] * merged_df["num_series_multiplier"]
            )

        merged_df = merged_df[[str(QC.PRUNING_RATIO), qc_id]]
        self.runs_df = self.runs_df.merge(merged_df, left_on=str(QC.ID), right_on=qc_id, how="left")

    @classmethod
    def add_extra_cols_for_amortized_prep_time(
        cls,
        extra_cols: dict[ERD, list[str]],
    ):
        extra_cols[ERD.METHODS_COLS] += [str(SSC.ID), str(SSC.INDEX_FILE), str(SSC.FFTS_FILE)]
        extra_cols[ERD.INDEXES_COLS] += [str(ISC.INDEXING_TIME_S), str(ISC.FFT_CALC_TIME_S)]

    def add_amortized_prep_time_column(self):
        merged_df = self.get_merged_df()

        isc_indexing_time = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEXING_TIME_S))
        isc_fft_calc_time = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.FFT_CALC_TIME_S))
        qc_settings_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.SETTINGS_ID))

        merged_df["num_runs"] = merged_df.groupby(qc_settings_id)[qc_settings_id].transform("count")

        cols_to_drop = [
            col for col in merged_df.columns if col.startswith(str(ERD.RUNS_COLS)) and col != qc_settings_id
        ]
        merged_df = merged_df.drop(columns=cols_to_drop)
        merged_df = merged_df.drop_duplicates()

        merged_df[str(QC.AMORTIZED_PREP_TIME_S)] = (
            merged_df[isc_indexing_time] + merged_df[isc_fft_calc_time]
        ) / merged_df["num_runs"]
        merged_df = merged_df[[str(QC.AMORTIZED_PREP_TIME_S), qc_settings_id]]

        self.runs_df = self.runs_df.merge(merged_df, left_on=str(QC.SETTINGS_ID), right_on=qc_settings_id, how="left")

    def add_leaf_fill_columns(self, leaf_fill_columns: list[str]):
        merged_df = self.get_merged_df()
        isc_leaf_capacity = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.LEAF_CAPACITY))

        for stat in SCP:
            leaf_fill_col = StatsColumn(ISTC.LEAF_FILL_STATS, stat)
            if leaf_fill_col in leaf_fill_columns:
                leaf_size_col = StatsColumn(ISTC.LEAF_SIZE_STATS, stat)
                istc_leaf_size_col = get_merged_col_name(ERD.INDEX_STATS_COLS, leaf_size_col)
                merged_df[leaf_fill_col] = merged_df[istc_leaf_size_col] / merged_df[isc_leaf_capacity]

        isc_index_name = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEX_FILE))
        columns = leaf_fill_columns + [isc_index_name]
        merged_df = merged_df[columns]
        self.index_stats_df = self.index_stats_df.merge(merged_df, left_on=str(ISC.INDEX_FILE), right_on=isc_index_name)

    def add_query_interval_column(self, num_query_intervals: int):
        merged_df = self.get_merged_df()
        qc_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.ID))
        qc_query_length = get_merged_col_name(ERD.RUNS_COLS, str(QC.QUERY_LENGTH))
        qsc_l_min = get_merged_col_name(ERD.QUERY_SETS_COLS, str(QSC.L_MIN))
        qsc_l_max = get_merged_col_name(ERD.QUERY_SETS_COLS, str(QSC.L_MAX))

        merged_df[str(QC.QUERY_INTERVAL)] = (merged_df[qc_query_length] - merged_df[qsc_l_min]) // np.ceil(
            (merged_df[qsc_l_max] - merged_df[qsc_l_min] + 1) / num_query_intervals
        ).astype(int)
        merged_df = merged_df[[str(QC.QUERY_INTERVAL), qc_id]]
        self.runs_df = self.runs_df.merge(merged_df, left_on=str(QC.ID), right_on=qc_id)

    @classmethod
    def add_extra_cols_for_length_ratio(cls, length_col: QSC, extra_cols: dict[ERD, list[str]]):
        extra_cols[ERD.QUERY_SETS_COLS] += [str(length_col), str(QSC.ID)]
        extra_cols[ERD.DATASETS_COLS] += [str(DSC.SERIES_LENGTH)]

    def add_length_ratio_column(self, ratio_col: QSC, length_col: QSC):
        merged_df = self.get_merged_df()
        qsc_id = get_merged_col_name(ERD.QUERY_SETS_COLS, str(QSC.ID))
        merged_df = merged_df.drop_duplicates(subset=[qsc_id])
        qsc_length_col = get_merged_col_name(ERD.QUERY_SETS_COLS, str(length_col))
        dsc_series_length = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.SERIES_LENGTH))

        merged_df[str(ratio_col)] = merged_df[qsc_length_col] / merged_df[dsc_series_length]
        merged_df = merged_df[[str(ratio_col), qsc_id]]
        self.query_sets_df = self.query_sets_df.merge(merged_df, left_on=str(QSC.ID), right_on=qsc_id)

    def add_num_length_groups_column(self):
        self.indexes_df[str(ISC.NUM_LEN_GROUPS)] = np.ceil(
            (self.indexes_df[str(ISC.L_MAX)] - self.indexes_df[str(ISC.L_MIN)] + 1)
            / self.indexes_df[str(ISC.L_PER_GROUP)]
        )

    @classmethod
    def add_extra_cols_for_num_envelopes(cls, extra_cols: dict[ERD, list[str]]):
        extra_cols[ERD.INDEXES_COLS] += [str(ISC.POS_PER_ENV), str(ISC.L_MIN)]
        extra_cols[ERD.DATASETS_COLS] += [str(DSC.SERIES_LENGTH)]

    def add_num_envelopes_column(self):
        merged_df = self.get_merged_df()
        isc_index_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEX_FILE))
        merged_df = merged_df.drop_duplicates(subset=[isc_index_file])
        isc_pos_per_env = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.POS_PER_ENV))
        isc_l_min = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.L_MIN))
        dsc_series_length = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.SERIES_LENGTH))

        merged_df[str(ISC.NUM_ENVELOPES)] = np.ceil(
            (merged_df[dsc_series_length] - merged_df[isc_l_min] + 1) / merged_df[isc_pos_per_env]
        )
        merged_df = merged_df[[str(ISC.NUM_ENVELOPES), isc_index_file]]
        print(len(self.indexes_df), len(merged_df))
        self.indexes_df = self.indexes_df.merge(
            merged_df, left_on=str(ISC.INDEX_FILE), right_on=isc_index_file, how="left"
        )

    @classmethod
    def load_csv_if_exists(cls, path: str, cols: list[str]) -> pd.DataFrame:
        if os.path.exists(path):
            available_cols = pd.read_csv(path, nrows=0).columns
            valid_cols = [col for col in cols if col in available_cols]
            return pd.read_csv(path, usecols=valid_cols)
        return pd.DataFrame()

    @classmethod
    def load(
        cls,
        logs_dir: str,
        cols: dict[ERD, list[Column]],
        add_runs: bool = True,
        add_dataset_stats: bool = False,
        add_query_stats: bool = False,
        add_index_stats: bool = False,
        num_query_intervals: int = 1,
    ):  # -> ExperimentResults:
        original_cols = cols.copy()
        cols = {erd: [str(col) for col in cols[erd]] if erd in cols else [] for erd in ERD}
        act_cols = {erd: list(set(REQUIRED_COLS[erd] + cols[erd])) for erd in ERD}
        extra_cols = {erd: [] for erd in ERD}

        # Handle method name column
        if str(SSC.METHOD_NAME) in cols[ERD.METHODS_COLS]:
            extra_cols[ERD.METHODS_COLS] += [str(col) for col in COLS_FOR_METHOD_NAME]
            act_cols[ERD.METHODS_COLS].remove(str(SSC.METHOD_NAME))

        # Handle pruning ratio column
        if str(QC.PRUNING_RATIO) in cols[ERD.RUNS_COLS]:
            cls.add_extra_cols_for_pruning_ratio(logs_dir, extra_cols, act_cols)

        # Handle amortized prep time column
        if str(QC.AMORTIZED_PREP_TIME_S) in cols[ERD.RUNS_COLS]:
            cls.add_extra_cols_for_amortized_prep_time(extra_cols)
            act_cols[ERD.RUNS_COLS].remove(str(QC.AMORTIZED_PREP_TIME_S))

        # Handle leaf fill stats columns
        leaf_fill_cols = []
        for stat in SCP:
            leaf_fill_col = StatsColumn(ISTC.LEAF_FILL_STATS, stat)
            if leaf_fill_col in cols[ERD.INDEX_STATS_COLS]:
                leaf_fill_cols.append(leaf_fill_col)
                extra_cols[ERD.INDEX_STATS_COLS].append(StatsColumn(ISTC.LEAF_SIZE_STATS, stat))
                act_cols[ERD.INDEX_STATS_COLS].remove(leaf_fill_col)
        if len(leaf_fill_cols) > 0:
            extra_cols[ERD.INDEXES_COLS].append(str(ISC.LEAF_CAPACITY))

        # Handle keep rate column
        if str(QC.KEEP_RATE) in cols[ERD.RUNS_COLS]:
            extra_cols[ERD.RUNS_COLS].append(str(QC.ABANDONING_RATE))
            act_cols[ERD.RUNS_COLS].remove(str(QC.KEEP_RATE))

        # Handle query interval column
        if str(QC.QUERY_INTERVAL) in cols[ERD.RUNS_COLS] and num_query_intervals > 1:
            extra_cols[ERD.RUNS_COLS] += [str(QC.QUERY_LENGTH), str(QC.ID)]
            extra_cols[ERD.QUERY_SETS_COLS] += [str(QSC.L_MIN), str(QSC.L_MAX)]
            act_cols[ERD.RUNS_COLS].remove(str(QC.QUERY_INTERVAL))

        # Handle length ratio columns
        for ratio_col, length_col in [(QSC.L_MIN_RATIO, QSC.L_MIN), (QSC.L_MAX_RATIO, QSC.L_MAX)]:
            if str(ratio_col) in cols[ERD.QUERY_SETS_COLS]:
                cls.add_extra_cols_for_length_ratio(length_col, extra_cols)
                act_cols[ERD.QUERY_SETS_COLS].remove(str(ratio_col))

        # Handle # length groups column
        if str(ISC.NUM_LEN_GROUPS) in cols[ERD.INDEXES_COLS]:
            extra_cols[ERD.INDEXES_COLS] += [str(ISC.L_MIN), str(ISC.L_MAX), str(ISC.L_PER_GROUP)]
            act_cols[ERD.INDEXES_COLS].remove(str(ISC.NUM_LEN_GROUPS))

        # Handle # envelopes column
        if str(ISC.NUM_ENVELOPES) in cols[ERD.INDEXES_COLS]:
            cls.add_extra_cols_for_num_envelopes(extra_cols)
            act_cols[ERD.INDEXES_COLS].remove(str(ISC.NUM_ENVELOPES))

        extra_cols = {erd: list(set(extra_cols[erd]) - set(act_cols[erd])) for erd in ERD}
        cols_to_load = {erd: act_cols[erd] + extra_cols[erd] for erd in ERD}
        csv_paths = {erd: os.path.join(logs_dir, CSV_FILES[erd]) for erd in ERD}
        dfs = {erd: cls.load_csv_if_exists(csv_paths[erd], cols=cols_to_load[erd]) for erd in ERD}

        results = cls(
            logs_dir=logs_dir,
            datasets_df=dfs[ERD.DATASETS_COLS],
            dataset_stats_df=dfs[ERD.DATASET_STATS_COLS],
            query_sets_df=dfs[ERD.QUERY_SETS_COLS],
            indexes_df=dfs[ERD.INDEXES_COLS],
            methods_df=dfs[ERD.METHODS_COLS],
            runs_df=dfs[ERD.RUNS_COLS],
            index_stats_df=dfs[ERD.INDEX_STATS_COLS],
            query_stats_df=dfs[ERD.QUERY_STATS_COLS],
            add_runs=add_runs,
            add_dataset_stats=add_dataset_stats,
            add_query_stats=add_query_stats,
            add_index_stats=add_index_stats,
        )

        # Apply duck-tape fix to the query file column of query sets
        if str(QSC.QUERY_FILE) in results.query_sets_df.columns:
            results.query_sets_df[str(QSC.QUERY_FILE)] = results.query_sets_df[str(QSC.QUERY_FILE)].str.replace(
                "../DATA/", "", regex=False
            )

        # Add method name column
        if str(SSC.METHOD_NAME) in cols[ERD.METHODS_COLS]:
            results.methods_df = define_method_name_col(results.methods_df)

        # Add pruning ratio column
        if str(QC.PRUNING_RATIO) in cols[ERD.RUNS_COLS]:
            results.add_pruning_ratio_column()

        # Add amortized prep time column
        if str(QC.AMORTIZED_PREP_TIME_S) in cols[ERD.RUNS_COLS]:
            results.add_amortized_prep_time_column()

        # Add leaf fill columns
        if len(leaf_fill_cols) > 0:
            results.add_leaf_fill_columns(leaf_fill_cols)

        # Add keep rate column
        if str(QC.KEEP_RATE) in cols[ERD.RUNS_COLS]:
            results.runs_df[str(QC.KEEP_RATE)] = 1 - results.runs_df[str(QC.ABANDONING_RATE)]

        # Add query length group column
        if str(QC.QUERY_INTERVAL) in cols[ERD.RUNS_COLS] and num_query_intervals > 1:
            results.add_query_interval_column(num_query_intervals)

        # Add length ratio columns
        for ratio_col, length_col in [(QSC.L_MIN_RATIO, QSC.L_MIN), (QSC.L_MAX_RATIO, QSC.L_MAX)]:
            if str(ratio_col) in cols[ERD.QUERY_SETS_COLS]:
                results.add_length_ratio_column(ratio_col, length_col)

        # Add # length groups column
        if str(ISC.NUM_LEN_GROUPS) in cols[ERD.INDEXES_COLS]:
            results.add_num_length_groups_column()

        # Add # envelopes column
        if str(ISC.NUM_ENVELOPES) in cols[ERD.INDEXES_COLS]:
            results.add_num_envelopes_column()

        # Drop extra columns
        results.datasets_df = results.datasets_df.drop(columns=extra_cols[ERD.DATASETS_COLS])
        results.dataset_stats_df = results.dataset_stats_df.drop(columns=extra_cols[ERD.DATASET_STATS_COLS])
        results.query_sets_df = results.query_sets_df.drop(columns=extra_cols[ERD.QUERY_SETS_COLS])
        results.indexes_df = results.indexes_df.drop(columns=extra_cols[ERD.INDEXES_COLS])
        results.methods_df = results.methods_df.drop(columns=extra_cols[ERD.METHODS_COLS])
        results.runs_df = results.runs_df.drop(columns=extra_cols[ERD.RUNS_COLS])
        results.index_stats_df = results.index_stats_df.drop(columns=extra_cols[ERD.INDEX_STATS_COLS])
        results.query_stats_df = results.query_stats_df.drop(columns=extra_cols[ERD.QUERY_STATS_COLS])

        cols = original_cols
        return results

    def get_merged_df(self) -> pd.DataFrame:
        dsc_dataset_file = get_merged_col_name(ERD.DATASETS_COLS, str(DSC.DATASET_FILE))

        columns_to_drop = []
        merged_df = rename_df_columns(self.datasets_df, ERD.DATASETS_COLS)

        if self.add_dataset_stats and os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.DATASET_STATS_COLS])):
            dstc_dataset_file = get_merged_col_name(ERD.DATASET_STATS_COLS, str(DSTC.DATASET_FILE))

            merged_df = merged_df.merge(
                rename_df_columns(self.dataset_stats_df, ERD.DATASET_STATS_COLS),
                left_on=dsc_dataset_file,
                right_on=dstc_dataset_file,
                how="left",
            )

        if os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.QUERY_SETS_COLS])):
            qsc_dataset_file = get_merged_col_name(ERD.QUERY_SETS_COLS, str(QSC.DATASET_FILE))

            merged_df = merged_df.merge(
                rename_df_columns(self.query_sets_df, ERD.QUERY_SETS_COLS),
                left_on=dsc_dataset_file,
                right_on=qsc_dataset_file,
                how="left",
            )

        if os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.METHODS_COLS])):
            qsc_query_file = get_merged_col_name(ERD.QUERY_SETS_COLS, str(QSC.QUERY_FILE))
            ssc_dataset_file = get_merged_col_name(ERD.METHODS_COLS, str(SSC.DATASET_FILE))
            ssc_query_file = get_merged_col_name(ERD.METHODS_COLS, str(SSC.QUERY_FILE))

            merged_df = merged_df.merge(
                rename_df_columns(self.methods_df, ERD.METHODS_COLS),
                left_on=[dsc_dataset_file, qsc_query_file],
                right_on=[ssc_dataset_file, ssc_query_file],
                how="left",
            )
            columns_to_drop += [ssc_dataset_file, ssc_query_file]

            if os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.INDEXES_COLS])):
                ssc_index_file = get_merged_col_name(ERD.METHODS_COLS, str(SSC.INDEX_FILE))
                ssc_ffts_file = get_merged_col_name(ERD.METHODS_COLS, str(SSC.FFTS_FILE))
                isc_index_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEX_FILE))
                isc_ffts_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.FFTS_FILE))
                isc_dataset_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.DATASET_FILE))

                has_index = merged_df[ssc_index_file].notna()
                merged_df_w_index = merged_df[has_index]
                merged_df_no_index = merged_df[~has_index]

                indexes_df = rename_df_columns(self.indexes_df, ERD.INDEXES_COLS)
                merged_df_w_index = merged_df_w_index.merge(
                    indexes_df, left_on=ssc_index_file, right_on=isc_index_file, how="left"
                )

                ffts_df = indexes_df[indexes_df[isc_ffts_file].notna()]
                merged_df_no_index = merged_df_no_index.merge(
                    ffts_df, left_on=ssc_ffts_file, right_on=isc_ffts_file, how="left"
                )
                merged_df = pd.concat([merged_df_no_index, merged_df_w_index], ignore_index=True)

                columns_to_drop.extend([ssc_index_file, isc_dataset_file])

                if self.add_index_stats and os.path.exists(
                    os.path.join(self.logs_dir, CSV_FILES[ERD.INDEX_STATS_COLS])
                ):
                    isc_index_file = get_merged_col_name(ERD.INDEXES_COLS, str(ISC.INDEX_FILE))
                    istc_index_file = get_merged_col_name(ERD.INDEX_STATS_COLS, str(ISTC.INDEX_FILE))

                    merged_df = merged_df.merge(
                        rename_df_columns(self.index_stats_df, ERD.INDEX_STATS_COLS),
                        left_on=isc_index_file,
                        right_on=istc_index_file,
                        how="left",
                    )

            if self.add_runs and os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.RUNS_COLS])):
                qc_settings_id = get_merged_col_name(ERD.RUNS_COLS, str(QC.SETTINGS_ID))
                ssc_id = get_merged_col_name(ERD.METHODS_COLS, str(SSC.ID))

                merged_df = merged_df.merge(
                    rename_df_columns(self.runs_df, ERD.RUNS_COLS),
                    left_on=ssc_id,
                    right_on=qc_settings_id,
                    how="left",
                )
                columns_to_drop.append(ssc_id)

        if self.add_query_stats and os.path.exists(os.path.join(self.logs_dir, CSV_FILES[ERD.QUERY_STATS_COLS])):
            qstc_dataset_file = get_merged_col_name(ERD.QUERY_STATS_COLS, str(QSTC.DATASET_FILE))

            merged_df = merged_df.merge(
                rename_df_columns(self.query_stats_df, ERD.QUERY_STATS_COLS),
                left_on=dsc_dataset_file,
                right_on=qstc_dataset_file,
                how="left",
            )
            columns_to_drop.append(qstc_dataset_file)

        print("Size of merged DF:", len(merged_df))
        return merged_df.drop(columns=columns_to_drop)


## %%[markdown]
"""
### Reducers
"""


# %%
class Reducer(BaseModel):
    class Config:
        arbitrary_types_allowed = True

    def __call__(self, value: np.ndarray) -> float | np.ndarray:
        raise NotImplementedError


class MeanReducer(Reducer):
    def __call__(self, value: np.ndarray) -> float:
        return np.mean(value)


class RobustMeanReducer(Reducer):
    discard_quantile: float = Field(0.05, ge=0.0, le=1.0)

    def __call__(self, value: np.ndarray) -> float:
        quantiles = np.quantile(value, [self.discard_quantile, 1 - self.discard_quantile])
        return np.mean(value[(value >= quantiles[0]) & (value <= quantiles[1])])


class StdReducer(Reducer):
    def __call__(self, value: np.ndarray) -> float:
        return np.std(value)


class MinReducer(Reducer):
    def __call__(self, value: np.ndarray) -> float:
        return np.min(value)


class MaxReducer(Reducer):
    def __call__(self, value: np.ndarray) -> float:
        return np.max(value)


class CollectionReducer(Reducer):
    reducer: Reducer
    separator: str = Field(";")

    def __call__(self, value: np.ndarray) -> np.ndarray:
        """
        Takes something like ["1;2;3", "4;5;6"] and returns per column reduction. E.g. with mean reducer:
        ["1", "4"] -> 2.5
        ["2", "5"] -> 3.5
        ["3", "6"] -> 4.5
        """
        split_arrays = np.array([np.fromstring(v, sep=self.separator) for v in value])
        return np.apply_along_axis(self.reducer, 0, split_arrays)


# %%[markdown]
"""
### Reduction executor function
"""

# %%

Targets = list[tuple[ERD, Column, Reducer]]
Groups = list[tuple[ERD, Column]]
ReductionResult = dict[tuple, list[float]]


def execute_reduction(
    experiments: list[ExperimentResults], targets: Targets, groups: Groups, na_replacement: Any = 0
) -> ReductionResult:
    """
    Executes a reduction on the given experiment results. The reduction result is a dictionary mapping the group
    tuples to a list of reduced values for each target.

    :param experiments: The list of experiment results to reduce.
    :param targets: The list of targets to reduce.
    :param groups: The list of groups to reduce.
    :return: The reduction result.
    """

    merged_targets: dict[str, list[Reducer]] = {}
    for target_df, target_col, reducer in targets:
        merged_col_name = get_merged_col_name(target_df, str(target_col))
        if merged_col_name not in merged_targets:
            merged_targets[merged_col_name] = []
        merged_targets[merged_col_name].append(reducer)

    merged_groups = [get_merged_col_name(group, str(group_col)) for group, group_col in groups]
    reduction_result = {}

    merged_df = pd.concat([experiment.get_merged_df() for experiment in experiments], ignore_index=True)
    merged_df = merged_df.fillna(na_replacement)

    grouped = merged_df.groupby(merged_groups)

    for group_keys, group_df in grouped:
        if not isinstance(group_keys, tuple):
            group_keys = (group_keys,)
        for target, reducers in merged_targets.items():
            for reducer in reducers:
                reduced_value = reducer(group_df[target])
                if group_keys not in reduction_result:
                    reduction_result[group_keys] = []
                reduction_result[group_keys].append(reduced_value)

    return reduction_result
