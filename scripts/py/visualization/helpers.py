from typing import Iterator

import numpy as np
from scripts.py.common.columns import Column
from scripts.py.visualization.reduction import ERD


def sort_dict(d: dict, key_func: callable) -> dict:
    return {k: v for k, v in sorted(d.items(), key=key_func)}


def dict_to_tuples(d: dict[ERD, list[Column]]) -> list[tuple[ERD, Column]]:
    tuples = []
    for key, values in d.items():
        for value in values:
            tuples.append((key, value))
    return tuples


def get_tuple_strings(tuples: list[tuple[ERD, Column]]) -> list[str]:
    return [str(col) for _, col in tuples]


def get_col_index(col: str, tuples: list[tuple[ERD, str]]) -> int:
    for i, (_, col_name) in enumerate(tuples):
        if col_name == col:
            return i
    return -1


def merge_univariate_datasets(reduced_values, ds_index: int = 0):
    merged_reduced_values = {}
    for group, values in reduced_values.items():
        dataset = group[ds_index].split("/", 1)[0]
        merged_group = (*group[:ds_index], dataset, *group[ds_index + 1 :])
        if merged_group not in merged_reduced_values:
            merged_reduced_values[merged_group] = [[] for _ in range(len(values))]
        for i, value in enumerate(values):
            merged_reduced_values[merged_group][i].append(value)

    for group, values_lists in merged_reduced_values.items():
        merged_reduced_values[group] = [np.mean(values) for values in values_lists]

    return merged_reduced_values


def iterate_columns(columns: dict[ERD, list[Column]]) -> Iterator[Column]:
    for erd, erd_columns in columns.items():
        for column in erd_columns:
            yield column
