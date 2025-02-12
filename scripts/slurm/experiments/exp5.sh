csv_dir=~/mts-subsequence-search/data/stocks
num_query_channels=5
series_len=2048
n_queries=20
# l_min=1536
l_max=2048
# lengths=(1536 1664 1792 1920 2048)
n_series=5000
k=1

l_min=512
lengths=($l_min 1024 1536 1742 2048)
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $n_series ${lengths[@]}

l_min=1024
lengths=($l_min 1536 1742 2048)
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $n_series ${lengths[@]}

l_min=1536
lengths=($l_min 1742 2048)
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $n_series ${lengths[@]}

l_min=1742
lengths=($l_min 2048)
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $n_series ${lengths[@]}

l_min=2048
lengths=($l_min)
./scripts/slurm/experiments/run.sh $csv_dir $num_query_channels $series_len $n_queries $l_min $l_max $n_series ${lengths[@]}
