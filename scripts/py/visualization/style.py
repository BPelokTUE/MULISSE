import numpy as np
from matplotlib.colors import LinearSegmentedColormap

PALETTE = {
    "Blues": [
        "#004589",
        "#034876",
        "#136fa3",
        "#3a8fc2",
        "#4a9fd3",
        "#5aafe5",
        "#6abff6",
    ],
    "Yellows": [
        "#949459",
        "#afaf56",
        "#caca51",
        "#e0e045",
        "#f3f338",
        "#ffffa5",
    ],
    "Oranges": [
        "#cc4801",
        "#e66101",
        "#f77113",
        "#ff8124",
        "#ff9136",
        "#ffa147",
    ],
    "Reds": [
        "#67001f",
        "#a50026",
        "#d7191c",
        "#e7292e",
        "#f7393f",
        "#ff494f",
        "#ff595f",
    ],
    "Greens": [
        "#003d14",
        "#086929",
        "#158939",
        "#2aa653",
        "#42be6b",
        "#54d07f",
        "#79e9a2",
    ],
    "Purples": [
        "#320565",
        "#45096b",
        "#5f358d",
        "#7f52ae",
        "#956ac6",
        "#a67dd5",
        "#c29eee",
    ],
    "Pinks": [
        "#8b0f5f",
        "#a31a6f",
        "#c0308f",
        "#e046af",
        "#f051bf",
    ],
    "Greys": [
        "#000000",
        "#333333",
        "#666666",
        "#999999",
        "#cccccc",
    ],
}

CATEGORY_COLORS = [
    PALETTE["Blues"][3],
    PALETTE["Oranges"][2],
    PALETTE["Greens"][3],
    PALETTE["Reds"][2],
    PALETTE["Yellows"][3],
    PALETTE["Purples"][2],
    PALETTE["Pinks"][2],
    PALETTE["Blues"][6],
    PALETTE["Oranges"][5],
    PALETTE["Greens"][6],
    PALETTE["Reds"][5],
    PALETTE["Yellows"][5],
    PALETTE["Purples"][6],
    PALETTE["Pinks"][4],
]

COLDEST_COLOR = PALETTE["Blues"][5]
HOTTEST_COLOR = PALETTE["Reds"][1]
COLD_TO_HOT_COLORS = LinearSegmentedColormap.from_list("cold_to_hot", [COLDEST_COLOR, HOTTEST_COLOR])
