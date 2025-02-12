csv_dir=~/mts-subsequence-search/data/stocks
num_query_channels=5
# series_len=2048
n_queries=20
l_min=1536
l_max=2048
lengths=(1536 1664 1792 1920 2048)
num_series=5000
k=1

for l in {2048..4096+1..512}; do
    series_len=$l
    ./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $num_series ${lengths[@]}
done
