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

### Work Items
0. Double check
    - Why MASS and ED w EA are the same time
        - Bug in parsing corrected
    - Double-check ULISSE EA 
        - The two algorithms are the same
    - Measure ULISSE vs ED w EA on query with 0 pruning power
        - Compared the average runtime between methods on the queries where ULISSE got pruning ratio 0. ULISSE is still the best performing method, and in fact the difference increases.
1. Add support for measuring abandoning power and run experiments with it
    - For non-MULISSE-lib implementations as well
2. Fix (or at least minimize) discrepancy between ULISSE, MASS, ED and their MULISSE library counterparts
    - ULISSE uses some "interesting" split strategies. See if these make a difference.
3. Add support for length-based grouping 

Checks to do:
- Rerun ED (C), MASS (C) and ULISSE (original and on a single thread)
- Important comparisons:
  - ED (C) vs ED scan vs ED scan with sorting vs ULISSE vs ULISSE single threaded
  - iSAX env vs iSAX env w SAX env w no priority queue vs ULISSE vs ULISSE single threaded
