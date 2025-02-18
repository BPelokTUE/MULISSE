k=1
n_queries=100

# Stocks
csv_dir=~/mts-subsequence-search/data/stocks
series_len=5500
l_min=730
l_max=730
num_query_channels=5
num_series=2000
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $num_series

# Synthetic
csv_dir=~/mts-subsequence-search/data/synthetic-big
series_len=4096
l_min=1024
l_max=1024
num_query_channels=1024
num_series=100
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $num_series

# weather
csv_dir=~/mts-subsequence-search/data/weather
series_len=8600
l_min=1488
l_max=1488
num_query_channels=4
num_series=1300
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $num_series