# Findings

## 26-02-2025

1. MULISSE as better on *synthetic data* regardless of the number of channels ( #7 )
    - Synthetic data leads to more balanced indexes: Why? **Analyze some indexes to find out**.
2. Optimal envelope size is highly dependent on parameters e.g. the allowed query range ( #8 )
    - **Parametrization is needed**
3. Relative contrast doesn't seem to explain the difference in aance between datasets ( #9 )

## 05-03-2025

1. Pure iSAX performs poorly ( #10 )
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
- **MASS outperforms ED**, with or without early abandoning, with or without sorting query data points, **even without precomputing FFT components** 
- Flat indexes (at least with envelopes as they are currently) substantially outperform prefix indexes. Anecdotally, even the creators of ULISSE fundamentally rely on a flat index to do most of the computation, as **the initial approximate search will visit at most 5 leaves** (5 is the default value in the code, and the value mentioned in the paper that is the best for approximate search).
- The pruning ratios observed during this investigation ($>0.3$) are far below what has previously been seen with synthetic data generated in the same way. **This is in line with the observation that increased query length range leads to lower pruning**, most likely due to loose envelopes.

**Note on parallelism**: there is no good reason to parallelize the current method, following in the footsteps of ULISSE:
1. Only parallelizing indexing methods is disingenuous
2. Parallelizing all methods will likely benefit sequential scan more, as in that case all time series have to be examined either way
3. Parallelizing on the level of the whole search (i.e. running multiple queries in parallel) will likely lead to faster overall run-time, while not benefiting any technique (although it may have adverse consequences, e.g. reducing cache locality, memory issues, etc)

## 09-04-2025
- *Note*:
    - Envelope tightness refers to the lower bounds
    - Envelope fullness refers the # subsequences summarized by the envelope
- Does the pruning power of ULISSE for large queries come from the last (few) tight envelope segments, or just the added dimensionality due to more segments:
    - ULISSE performs poorly on small queries
    - Envelopes > Trees
        - Compare to pure iSAX as well
- Optimize one-phase approach
    - The pre-filter should get back subsequences grouped by time series (or just time series). Ideally keep envelopes at a time series level (or even multi-time-series-level) to increase the potency of MASS.
- TODO:
    - [x] **Main task**: Create table of statements (similar to the ones above) supported by plots
    - [~] Figure out how to group
    - [~] Tradeoff between memory footprint and pruning power
    - [~] Grouping into envelopes could be (partially) done after insertion


## 16-04-2025

- Length-based grouping leads to **consistent but modest performance gains** independent of the dataset or query range.
- TODO:
    - [x] Find the bottleneck in the best working solution, figure out how to improve on it 
        - Total time is dominated by TS examination time, therefore pruning ratio should be optimized. It is not clear why an iSAX trie would properly group together envelopes based solely on their discretized lower bounds. An invSAX based approach should work better for this.
    - [x] Figure out why iSAX does not work properly
        - The pruning ratio was incorrectly calculated, however this does not affect performance, pure iSAX is still slow
        - iSAX recalculates distances for overlapping subsequences. This could be optimized, presumably making iSAX faster. However, **index construction time still makes pure iSAX unusable for even moderately large datasets**, therefore I will not implement these optimizations for now, as they will not lead to a worthwhile method.

- TODO analysis:
    - [ ] Analyze the index statistics of length-grouped indexes (the data is already collected)
    - [ ] Figure out when ED is better than MASS (depending on query range, time series length, maybe number of time series but probably not)
- TODO implementation: 
    - [x] Implement invSAX and combine it with envelopes. Upgrade into a UB-tree (Coconut) once this is done and works.
        - UB-tree is not really applicable for our use-case, as it searches for the hypothetical location of the query in the index. Since in our case the entries of the index are envelopes, but we are searching for subsequences, this is not really usable
        - We can still apply invSAX and group together envelopes into nodes, setting the envelope bounds for each parent node to the max of children. Note however, that this cannot help with pruning: a node can be pruned away only if its children can be pruned away. It is possible that min-dist calculation time could be reduced, but that is a minor part of search time, and a cursory look at early results suggests that this not always the case.
    - [ ] Implement way to use precomputed FFTs for more than one envelop per time series (probably calculate FFT for whole time series anyways, then mark the time series once MASS has been run on it to avoid future recomputations)
    - [ ] Use different (dynamically set) index properties (e.g. different # envelope per time series) for the different length-group indexes (e.g. short query range indexes could use more envelopes per time series)

## 24-04-2025
- There was a bug when using smaller position groups. This is now fixed and lower position group sizes seem better than higher ones, especially for shorter queries
- Shorter queries are a bottleneck, length-grouped pure envelopes with small position groups already perform well on queries with $|Q|\ge 0.25 * l_{\max}$
- To optimize short query performance, focusing more on early segments (e.g. making these segments shorter) should be explored (decreasing segment length already show promising results, however this needlessly increases index size)

- TODO:
    - [ ] Run on raw
        - ULISSE fails on `malloc` errors. I could not run a single experiment with raw (unnormalized data). I suggest implementing raw search in MULISSE and running experiments there if needed.
        - [ ] Implement raw
    - [x] Write down optimal PG ($N_p$) and LG ($N_l$) settings
        - $N_p^{\text{optimal}} \approx 20$
        - $N_l^{\text{optimal}} \approx 16$
    - [~] Figure out how to adapt segment lengths also per channel
        - Idea: presence

### Presence
Before segmentation (PAA averaging) is applied in envelopes, we can calculate the "presence" of each point within the envelope. The presence for point at index $l\in[1, l_{\max}]$ is the **number of subsequences in the envelope that contribute to the point**, in other words, the number of subsequences with length and starting position relevant to the envelope that include this point. Let $m$ be the length of time series, $\gamma$ be the number of starting positions in the envelope, $l^g_{\min}$ and $l^g_{\max}$ be the minimum and maximum query lengths in group $g$ respectively, and $p\in\mathbb{N}^{l^g_{\max}}$ be the array of presence scores for points in an envelope of length group $g$. Then, $p$ can be calculated as:
$$
\begin{array}{rcl}
    p_{l^g_{\max}} & := & \min(\gamma, m - l^g_{\max} + 1) \\
    p_{l^g_{\max}-1} & := & \min(\gamma, m - l^g_{\max} + 2) + p_{l^g_{\max}} \\
    & \vdots & \\
    p_{l^g_{\min}} & := & \min(\gamma, m - l^g_{\min} + 1) + p_{l^g_{\min}+1} \\
    p_{l^g_{\min}-1} & := & p_{l^g_{\min}} \\
    & \vdots & \\
    p_1 & := & p_{l^g_{\min}} 
\end{array}
$$
Or equivalently:
$$
p_l=\begin{cases}
    0 & \text{if }l>l^g_{\max} \\
    \min(\gamma, m - l + 1) + p_{l+1} & \text{if }l^g_{\min}\le l\le l^g_{\max}\\
    p_{l^g_{\min}} & \text{if }l<l^g_{\min}
\end{cases}
$$

Using presence:
- [x] A channel can be divided into $N_s$ segments of roughly equal presence
- [~] $N_s*N_l$ segments can be divided across $N_l$ length groups, such that each segment has roughly equal presence
- [ ] $N_s*N_l*N_c$ segments can be divided across $N_l$ length groups and $N_c$ channels, such that each channel gets segments proportional to its importance determined based on **???**

*NOTE*: This **does not solve the parametrization issue** (having three parameters to tune: $N_s$, $N_l$ and $N_p$), however it does make a connection between them. It could lead to a solution down the line, but I currently don't see exactly how.

### Meeting notes
- Why use presence instead of the range of values summarized in each point: presence is agnostic to the dataset, and therefore the segmentation will be the same for all envelopes in a given length group, in contrast if segmentation is determined based on ranges, then every envelope can potentially have a unique segmentation which makes it difficult to calculate mindistances to the query, because all unique segmentation for the query would have to be calculated.

- TODO:
    - [x] Implement raw search:
        - Hypothesis: on raw time series the mean of time series should be good enough for pruning, based on the fact that $\gamma$ is large and sometimes there are only a few segments
    - [ ] Finalize experiments for segmentation
    - [ ] Think about how to prioritize channels (e.g. based on cross-dataset variance)
    - [ ] Think about summarizing after inserting into the index

## 07-05-2025

### ULISSE on raw (MULISSE lib implementation)

Performance on raw is better than MASS, but still not as good as expected:
- Query time is not in line with pruning ratio, logging for Chained indexes may need to be checked
    - $|Q|=l_{\max}$ leads to very fast query time, most likely due to last tight envelope, however this is not reflected in the pruning ratio to a sufficient degree
- $\gamma_{\max}$ may not be optimal for raw either
- *Note*: there was a bug, causing data and logs to be stored in `/home` instead of `$TMPDIR`. This does not affect previous experiments on Linardi's code.

![](./summaries/images/ULISSE_5M_raw_per_length.png)

### Segmentation

In total 6 different ways of choosing segment lengths have been attempted, 4 of them utilizing presence in some form. The results are underwhelming, any method that has lower query time has higher amortized index time and index size, suggesting that "how" segments sizes are chosen is not too important, at least with the current methods.

### Summarization after insertion

To investigate the possibility of summarizing subsequences after insertion into an index, various combinations of $N_p$ and $N_l$ were tested. In the most extreme case, when there is a single length in each length group, and a single starting position in each envelope, all subsequences are inserted separately into the indexes. Bigger length groups and position groups lead to more summarization before insertion, therefore smaller indexes, but potentially suboptimal grouping.
