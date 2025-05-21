from enum import Enum, auto


class DatasetSettingsColumn(Enum):
    ID = auto()
    DATASET_FILE = auto()
    SERIES_LENGTH = auto()
    NUM_CHANNELS = auto()
    NUM_SERIES = auto()
    SD = auto()
    SOURCE_CSVS = auto()
    L_MIN = auto()
    L_MAX = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "dataset_settings.csv"


class DatasetStatsColumn(Enum):
    DATASET_FILE = auto()
    TS_IND = auto()
    CHANNEL = auto()
    MEAN = auto()
    STD = auto()
    SKEWNESS = auto()
    KURTOSIS = auto()
    TOTAL_VAR_MEANS = auto()
    TOTAL_VAR_STDS = auto()
    AUTOCORR_MEANS = auto()
    AUTOCORR_STDS = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "dataset_stats.csv"


class IndexSettingsColumn(Enum):
    ID = auto()
    DATASET_FILE = auto()
    INDEX_FILE = auto()
    FFTS_FILE = auto()
    L_MIN = auto()
    L_MAX = auto()
    L_PER_GROUP = auto()
    POS_PER_ENV = auto()
    NUM_LEN_GROUPS = auto()
    NUM_ENVELOPES = auto()
    ENTRY_MERGER_TYPE = auto()
    MERGER_NUM_BITS = auto()
    NORMALIZED = auto()
    INDEX_TYPE = auto()
    LG_SEGMENTATION_STRATEGY = auto()
    SEGMENTATION_STRATEGY = auto()
    NUM_SEGMENTS = auto()
    BREAKPOINT_STRATEGY = auto()
    SPLIT_STRATEGY = auto()
    MERGE_IN_LEAVES = auto()
    MIN_NUM_BITS_ON_TIE = auto()
    FIRST_LAYER_NUM_BITS = auto()
    LEAF_CAPACITY = auto()
    NUM_BITS_LIMIT = auto()
    ADAPT_TO_DATASET = auto()
    NUM_LEAVES = auto()
    NUM_NODES = auto()
    NUM_ENTRIES = auto()
    INDEXING_TIME_S = auto()
    FFT_CALC_TIME_S = auto()
    SIZE_ON_DISK_B = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "index_settings.csv"


class SearchSettingsColumn(Enum):
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
    SORT_QUERY = auto()
    USE_PRIORITY_QUEUE = auto()

    METHOD_NAME = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "search_settings.csv"


class QuerySetSettingsColumn(Enum):
    ID = auto()
    DATASET_FILE = auto()
    QUERY_FILE = auto()
    NUM_QUERIES = auto()
    L_MIN = auto()
    L_MAX = auto()
    L_MIN_RATIO = auto()
    L_MAX_RATIO = auto()
    EXACT_LENGTHS = auto()
    USED_CHANNELS = auto()
    CHANNEL_MASK = auto()
    NOISE = auto()
    SEED = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "query_set_settings.csv"


class QueryColumn(Enum):
    ID = auto()
    SETTINGS_ID = auto()
    QUERY_ID = auto()
    QUERY_LENGTH = auto()
    QUERY_INTERVAL = auto()
    LENGTH_GROUP = auto()
    QUERY_CHANNELS = auto()
    RESULT_SET_TS_INDICES = auto()
    RESULT_SET_TS_POSITIONS = auto()
    RESULT_SET_DISTANCES = auto()
    MIN_DIST_TOTAL = auto()
    NUM_LEAVES_VISITED = auto()
    NUM_NODES_VISITED = auto()
    NUM_TS_EXAMINED = auto()  # For backward compatibility
    NUM_ENTRIES_EXAMINED = auto()
    NUM_MIN_DIST_CALCULATED = auto()
    NUM_PTS_IN_EXAMINED_ENTRIES = auto()
    NUM_PTS_EXAMINED = auto()
    ABANDONING_RATE = auto()
    KEEP_RATE = auto()
    TOTAL_TIME_S = auto()
    FIRST_LAYER_TIME_S = auto()
    TREE_TRAVERSAL_TIME_S = auto()
    IO_TIME_S = auto()
    TS_EXAMINATION_TIME_S = auto()
    AMORTIZED_PREP_TIME_S = auto()

    PRUNING_RATIO = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "runs.csv"


class StatsColumnPrefix(Enum):
    MIN = auto()
    MAX = auto()
    MEAN = auto()
    STD = auto()

    def __str__(self):
        return self.name.lower()


class IndexStatsColumn(Enum):
    INDEX_FILE = auto()
    LEAF_SIZE_STATS = auto()
    LEAF_FILL_STATS = auto()
    LEAF_HEIGHT_STATS = auto()
    SEG_RANGE_STATS = auto()
    SEG_LOWER_STATS = auto()
    SEG_UPPER_STATS = auto()
    SEG_MID_STATS = auto()
    NUM_INF_LOWER = auto()
    NUM_INF_UPPER = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "index_stats.csv"


class QueryStatsColumn(Enum):
    ID = auto()
    DATASET_FILE = auto()
    QUERY_FILE = auto()
    QUERY_LENGTH = auto()
    QUERY_CHANNELS = auto()
    NORMALIZED = auto()
    QUERY_NOISE = auto()
    DIST_STATS = auto()
    RC_USING_MAX = auto()
    RC_USING_MEAN = auto()

    def __str__(self):
        return self.name.lower()

    @classmethod
    def get_csv_name(cls) -> str:
        return "query_stats.csv"


class StatsColumn:
    def __init__(self, col: IndexStatsColumn | QueryStatsColumn, prefix: StatsColumnPrefix):
        self.col = col
        self.prefix = prefix

    def __str__(self):
        return f"{str(self.prefix)}_{str(self.col).replace('_stats', '')}"


Column = (
    IndexSettingsColumn
    | DatasetSettingsColumn
    | DatasetStatsColumn
    | QuerySetSettingsColumn
    | QueryStatsColumn
    | IndexStatsColumn
    | SearchSettingsColumn
    | QueryColumn
    | StatsColumn
)
