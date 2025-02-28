import pandas as pd

from .columns import SearchSettingsColumn as SSC

COLS_FOR_METHOD_NAME = [
    SSC.ID,
    SSC.SEARCH_METHOD,
    SSC.DISTANCE_MEASURE,
    SSC.FFTS_FILE,
    SSC.EARLY_ABANDONING,
    SSC.INDEX_FILE,
]


def get_method_name(df: pd.DataFrame, settings_id: int) -> str:
    setting = df[df[str(SSC.ID)] == settings_id].iloc[0]
    parts = [setting[str(SSC.SEARCH_METHOD)], setting[str(SSC.DISTANCE_MEASURE)]]
    if pd.notna(setting[str(SSC.FFTS_FILE)]) and setting[str(SSC.FFTS_FILE)] != "":
        parts.append("ffts")
    if pd.notna(setting[str(SSC.EARLY_ABANDONING)]) and setting[str(SSC.EARLY_ABANDONING)] != "":
        uses_early_abandon = bool(setting[str(SSC.EARLY_ABANDONING)])
        if uses_early_abandon:
            parts.append("early")
    if pd.notna(setting[str(SSC.INDEX_FILE)]) and setting[str(SSC.INDEX_FILE)] != "":
        index_name = setting[str(SSC.INDEX_FILE)].split("/")[-1].split(".")[0]
        parts.append(index_name)
    return "-".join(parts)


def define_method_name_col(df: pd.DataFrame) -> pd.DataFrame:
    df_with_name_col = df.copy()
    df_with_name_col[str(SSC.METHOD_NAME)] = df.apply(lambda row: get_method_name(df, row[str(SSC.ID)]), axis=1)
    return df_with_name_col
