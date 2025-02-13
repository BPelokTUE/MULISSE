# csv_dir=~/mts-subsequence-search/data/synthetic-big
# num_query_channels=5
series_len=2048
n_queries=100
l_min=1536
l_max=2048
# lengths=(1536 1664 1792 1920 2048)
lengths=(1792)
# num_series=100
k=1

# Stocks
csv_dir=~/mts-subsequence-search/data/stocks
num_query_channels=5
num_series=5000
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $num_series ${lengths[@]}

# Synthetic
csv_dir=~/mts-subsequence-search/data/synthetic-big
num_query_channels=1024
num_series=100
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $num_series ${lengths[@]}

# weather
csv_dir=~/mts-subsequence-search/data/weather
num_query_channels=4
num_series=5000
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $num_series ${lengths[@]}