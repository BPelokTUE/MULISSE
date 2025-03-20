# Findings

## 26-02-2025

1. MULISSE as better on *synthetic data* regardless of the number of channels ( #7 )
    - Synthetic data leads to more balanced indexes: Why? **Analyze some indexes to find out**.
2. Optimal envelope size is highly dependent on parameters e.g. the allowed query range ( #8 )
    - **Parametrization is needed**
3. Relative contrast doesn't seem to explain the difference in aance between datasets ( #9 )

## 05-03-2025

1. Pure iSAX as poorly ( #10 )
    - Index size is much larger (relative to dataset size) than other indexes (iSAX + envelope or pure envelope)
    - Consequently index construction takes much longer
    - Searching the index is 1 to 2 orders of magnitude slower than iSAX + envelope or envelope, even though the pruning ratio ($1 - \frac{\text{subsequences examined}}{\text{subsequences in the index}}$) is very high
    - Improvements could be made to pure iSAX, but this does not seem like a promising direction. However, the poor performance does indicate that summarizing on the level of subsequences is beneficial or even necessary
2. Pure envelope performs slightly better than iSAX + envelope ( #11 )
    - This seems to suggest that focusing on envelope parametrization / improving envelope pruning power is a promising direction, and iSAX is not needed / should not be the focus
3. Index analysis - TODO ( #15 )

## 12-03-2025

### Parametrization

- The fastest methods seem to be completely flat or very imbalanced
- Lower leaf capacity (as a percentage of entries in the index) leads to better results:
    - This makes iSAX envelope indexes more imbalanced, but faster to search with nonetheless
    - On the other hand, pure iSAX indexes do not get more imbalanced, or at least not to the same degree
    - To attempt to fix this, the breakpoints were adjusted to the actual distribution of the lower PAA values, however this made the index even more imbalanced. On closer inspection the standard deviations of the lower envelope values is huge (500-1000).
- Higher starting # bits leads to flatter indexes, as the # possible first layer iSAX words grows exponentially with the # starting bits. As a consequence, large starting bit count (e.g. 4) leads to an index that performs worse than simply using envelopes without discretization or a tree in all cases.
- Pure iSAX actually performs on par (or at least not much worse) than the other two indexes, however, since parallelized insertion / bulk loading is not implemented (yet) testing is slow, because index creation can take multiple hours.
- Optimal envelope size is still hard to determine. Flatter indexes seem to prefer lower envelope size, deeper indexes on the other hand perform better with larger envelopes.

### Suggestions / Future work

- Parallelize iSAX insertion (following the parallelized iSAX 2.0 bulk loading algorithm) and the runner (`run_mulisse.py`) for faster testing
- Implement everything required for raw time series search, and check if the performance differences are still there
    - If not, one hypothesis is (based on high standard deviation of lower envelope values) is summarizing many subsequence segments in one envelope segment, can lead to highly varied envelope values due to normalization. This might be mitigated by grouping subsequence by length in addition to starting positions, which should reduce the variety of values that the data points within each segment take. TODO: rephrase this.
- If implementing length-based grouping does not make the indexes balanced, then switching to UB-trees (akin to Coconut) instead of prefix trees might help, although it is possible that it will only hide the issue (envelopes being too varied), but the issue will still continue to hurt performance.

### TODOs:
- Run original ULISSE vs MASS on HPC, try reproducing the original results
- Run comparison on s