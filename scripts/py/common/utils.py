import pandas as pd

from .columns import QuerySettingsColumn as QSC

COLS_FOR_METHOD_NAME = [
    QSC.ID,
    QSC.SEARCH_METHOD,
    QSC.DISTANCE_MEASURE,
    QSC.FFTS_FILE,
    QSC.EARLY_ABANDONING,
    QSC.INDEX_FILE,
]


def get_method_name(df: pd.DataFrame, settings_id: int) -> str:
    setting = df[df[str(QSC.ID)] == settings_id].iloc[0]
    parts = [setting[str(QSC.SEARCH_METHOD)], setting[str(QSC.DISTANCE_MEASURE)]]
    if pd.notna(setting[str(QSC.FFTS_FILE)]) and setting[str(QSC.FFTS_FILE)] != "":
        parts.append("ffts")
    if pd.notna(setting[str(QSC.EARLY_ABANDONING)]) and setting[str(QSC.EARLY_ABANDONING)] != "":
        uses_early_abandon = bool(setting[str(QSC.EARLY_ABANDONING)])
        if uses_early_abandon:
            parts.append("early")
    if pd.notna(setting[str(QSC.INDEX_FILE)]) and setting[str(QSC.INDEX_FILE)] != "":
        index_name = setting[str(QSC.INDEX_FILE)].split("/")[-1].split(".")[0]
        parts.append(index_name)
    return "-".join(parts)


def define_method_name_col(df: pd.DataFrame, required_cols: list[str]) -> pd.DataFrame:
    df_with_name_col = df.copy()
    df_with_name_col[str(QSC.METHOD_NAME)] = df.apply(lambda row: get_method_name(df, row[str(QSC.ID)]), axis=1)

    col_strs_for_method_name = [str(col) for col in COLS_FOR_METHOD_NAME]
    df_with_name_col.drop(
        columns=[col for col in col_strs_for_method_name if col not in required_cols and col != str(QSC.METHOD_NAME)],
        inplace=True,
    )
    return df_with_name_col
