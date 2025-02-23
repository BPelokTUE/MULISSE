from enum import Enum, auto


class DatasetSettingsColumn(Enum):
    ID = auto()
    DATASET_FILE = auto()
    SERIES_LENGTH = auto()
    NUM_CHANNELS = auto()
    NUM_SERIES = auto()
    SD = auto()
    SOURCE_CSVS = auto()
    LOW_SD_LEN = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "dataset_settings.csv"


class IndexSettingsColumn(Enum):
    ID = auto()
    DATASET_FILE = auto()
    INDEX_FILE = auto()
    FFTS_FILE = auto()
    L_MIN = auto()
    L_MAX = auto()
    NORMALIZED = auto()
    INDEX_TYPE = auto()
    SEGMENT_LENGTH = auto()
    POS_PER_ENV = auto()
    FIRST_LAYER_NUM_BITS = auto()
    LEAF_CAPACITY = auto()
    BREAKPOINT_STRATEGY = auto()
    SPLIT_STRATEGY = auto()
    MIN_NUM_BITS_ON_TIE = auto()
    NUM_BITS_LIMIT = auto()
    NUM_LEAVES = auto()
    NUM_NODES = auto()
    INDEXING_TIME_S = auto()
    FFT_CALC_TIME_S = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "index_settings.csv"


class QuerySettingsColumn(Enum):
    ID = auto()
    INDEX_FILE = auto()
    DATASET_FILE = auto()
    FFTS_FILE = auto()
    QUERY_FILE = auto()
    NUM_QUERIES = auto()
    QUERY_TYPE = auto()
    R_RANGE_R = auto()
    KNN_K = auto()
    EXACT = auto()
    NORMALIZED = auto()
    SEARCH_METHOD = auto()
    DISTANCE_MEASURE = auto()
    EARLY_ABANDONING = auto()

    METHOD_NAME = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "search_settings.csv"


class QueryColumn(Enum):
    ID = auto()
    SETTINGS_ID = auto()
    QUERY_ID = auto()
    QUERY_LENGTH = auto()
    QUERY_CHANNELS = auto()
    RESULT_SET_TS_INDICES = auto()
    RESULT_SET_TS_POSITIONS = auto()
    RESULT_SET_DISTANCES = auto()
    NUM_LEAVES_VISITED = auto()
    NUM_NODES_VISITED = auto()
    NUM_TS_EXAMINED = auto()
    TOTAL_TIME_S = auto()
    FIRST_LAYER_TIME_S = auto()
    TREE_TRAVERSAL_TIME_S = auto()
    IO_TIME_S = auto()
    TS_EXAMINATION_TIME_S = auto()

    PRUNING_RATIO = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "runs.csv"


class QueryStatsColumn(Enum):
    ID = auto()
    DATASET_FILE = auto()
    QUERY_FILE = auto()
    QUERY_LENGTH = auto()
    QUERY_CHANNELS = auto()
    NORMALIZED = auto()
    QUERY_NOISE = auto()
    MIN_DIST = auto()
    MAX_DIST = auto()
    MEAN_DIST = auto()
    DIST_STD_DEV = auto()
    RC_USING_MAX = auto()
    RC_USING_MEAN = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "query_stats.csv"
