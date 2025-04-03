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
    - **Done**, although the parallelized iSAX insertion displays strange behavior from time to time
- ~~Implement everything required for raw time series search, and check if the performance differences are still there~~
- If implementing length-based grouping does not make the indexes balanced, then switching to UB-trees (akin to Coconut) instead of prefix trees might help, although it is possible that it will only hide the issue (envelopes being too varied), but the issue will still continue to hurt performance.

## 26-03-2025
- Reproduced ULISSE vs MASS results for $n=5*10^6$ synthetic dataset
    - While the ULISSE is around one order of magnitude faster than MASS, this does not include index creation time
    - Additionally, the pruning ratio of ULISSE is highly variable, while the early abandoning power is consistently above $95\%$
        - **Suggestion**: measure early abandoning power as it seems quite important
- Ran original ULISSE, C implementations of ED with EA, MASS and MULISSE implementation for all of them, on a synthetic univariate dataset with $n=10^5$:
    - MULISSE implementations are considerably slower:
        - Experiments with potential fixes are on the way:
            - Add cache clearing between queries to own MASS in the same as done in C implementation
            - Use `float` instead of `double` for ED
            - Attempt to use top-down inserter for MULISSE, in case the parallel inserter is causing structural issues
        - Double check early abandoning implementation in ULISSE, maybe it contains additional tricks
    - The pruning ratio is low for both ULISSE and MULISSE, even though experiments were run on a synthetic dataset
        - MULISSE pruning ratio is lower, further investigation is required
        - Even ULISSE pruning ratio is only $17\%$ on average, much lower than what we have seen before on synthetic data.
            - The one major difference between this and previous experiments on synthetic data, is that **the range of query length is much greater:** $|Q|\in[256,4096]$ with $m=4096$
                - One hypothesis for the low pruning ratio then is that the large number (and varied size) of subsequences leads to loose envelopes $\Rightarrow$ **Suggestion**: let me implement length-based grouping (in addition to starting position based grouping). At this point I think we have seen plenty of evidence that shorter query ranges lead to better pruning, and with this implementation, this hypothesis could be denied or confirmed and potentially solved.

## ULISSE backtracking

ULISSE and pure C sequential scan implementations were achieving faster query times than their MULISSE-lib counterparts. This issue has been addressed by:
- The discrepancy between sequential scan implementations was due to unnecessary floating point type conversions and run-time (as opposed to compile-time) polymorphism. These have been fixed, and sequential scan methods in MULISSE operate at similar speeds to stand-alone implementations.
- The main reason for the good performance of ULISSE was that **it was using parallelism during search** (all other methods, including MASS are single-threaded). Once parallelism was disabled (by settings the number of cores to 1) **ULISSE became the slowest method out of all the ones tested**. Do note however, that using a single separate thread for running all scans still brings some overhead, so it is not an entirely fair comparison.
- Other aspects of ULISSE were also discovered during the backtracking process:
    - Most importantly: ULISSE uses two indexes for search: an iSAX prefix-tree for a quick approximate search, and a *flat SAX+Envelope index* for the exhaustive search right after. This aspect is mentioned in the paper, but was overlooked during the first implementation, which only used a prefix-tree, for a single exhaustive search.
    - Interestingly: ULISSE sorts the data points in each query by their absolute value, in order to increase early abandoning power. While this goal is achieved by sorting, **final query-time is higher for sorted queries**, most likely due to increased number of indirections (more indexing) and reduced cache locality. Perhaps ED with sorted queries could be improved, but considering that MASS outperforms ED in all cases, this is probably a poor use of time.
    - Less importantly: The backtracking revealed some questionable implementation choices in ULISSE, in particular a bugged splitting strategy, and the use of fixed breakpoint indexes, many of them being equal. Implementing these did not lead to significant changes.

The following may be inferred from this investigation:
- MASS outperforms ED, with or without early abandoning, with or without sorting query data points, **even without precomputing FFT components** 
- Flat indexes (at least with envelopes as they are currently) substantially outperform prefix indexes. Anecdotally, even the creators of ULISSE fundamentally rely on a flat index to do most of the computation, as **the initial approximate search will visit at most 5 leaves** (5 is the default value in the code, and the value mentioned in the paper that is the best for approximate search).
- The pruning ratios observed during this investigation ($>0.3$) are far below what has previously been seen with synthetic data generated in the same way. **This is in line with the observation that increased query length range leads to lower pruning**, most likely due to loose envelopes.

**Note on parallelism**: there is no good reason to parallelize the current method, following in the footsteps of ULISSE:
1. Only parallelizing indexing methods is disingenuous
2. Parallelizing all methods will likely benefit sequential scan more, as in that case all time series have to be examined either way
3. Parallelizing on the level of the whole search (i.e. running multiple queries in parallel) will likely lead to faster overall run-time, while not benefiting any technique (although it may have adverse consequences, e.g. reducing cache locality, memory issues, etc)
