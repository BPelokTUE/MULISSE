# Summary of Findings (10-04-2024)

## 1. ULISSE 2018 performance

### 1.1 Parallelization

**ULISSE 2018 performs below expectations**. The unmodified version does not reach the promised speed, and it was found that this version is parallelized. **After making it sequential, ULISSE performs only marginally better than MASS** on a synthetic dataset with $n=5*10^6, m=4096$.

![ULISSE vs MASS](images/ULISSE_5M.png)

### 1.2 Pruning power

Inspecting queries by length, we can see that the **pruning power increases substantially with the length of the query**. The current hypothesis is that this is caused by the relative tightness of later segments, which are included in longer envelopes, but not in shorter ones (to be investigated).

![ULISSE vs MASS pruning rate](images/ULISSE_5M_pruning.png)

![ULISSE vs MASS per query length](images/ULISSE_5M_per_length.png)


## 2. MASS vs Euclidean distance with early abandoning

Across a wide range of methods run on synthetic data with $n=10^5, m=4096$, **MASS was found to be faster than ED w EA**, even without precomputed FFTs. Although there could be some cases where ED w EA is preferred, future methods should take advantage of MASS when possible.

![ED w EA vs MASS](images/EDEA_vs_MASS.png) 

## 3. Envelopes vs iSAX trie

**A simple flat envelope index is consistently faster than an iSAX trie with or without envelopes**. Tests were run on multiple datasets, query length ranges, leaf capacities and envelope sizes, and *pure envelopes always perform better than prefix-tree based methods*. This result also holds for the large $n=10^5, m=4096$ dataset.

![Envelope vs iSAX trie](images/Envelope_vs_iSAX_256_stocks.png)

Additionally, even with parallelization, 16 cores, and 128GB memory capacity, **index construction without envelopes is 1.5-2 orders of magnitude slower than with envelopes**, making amortized query time significantly slower for pure iSAX:

![Envelope vs iSAX trie](images/Envelope_vs_iSAX_256_stocks_indexing.png)

## 4. Pruning ratio, envelope size, query range

**Pruning ratio is highly dependent on envelope size and query range**, both of which influence the number of subsequences summarized by a single envelope:

![Pruning ratio by envelope size and query range](images/PR_ES_QR_stocks.png)

### 4.1 Potential solutions:
- **Grouping subsequences by length**: grouping subsequences by length (possibly in addition to grouping by starting position as done in envelopes) could have the following benefits:
    - Separate indexes could be created per length group, and at query time only one of these would have to be checked, determined based on the length of the query
    - Grouping by length would decrease the number of subsequences summarized in each envelope, making envelopes tighter, and thereby increasing pruning power
    - Grouping by length could decrease the variation in the scale of subsequences summarized by each envelope, making envelopes tighter
- **Grouping subsequences by similarity** (possibly after inserting all data into the index): leveraging the grouping of an index, subsequences (or small envelopes) could be combined after insertion into the index
    - *Note 1*: some summarization before insertion is required, due to the volume of subsequences even in a small dataset (based on the speed of creating a pure iSAX index)

## 5. Length based grouping

TODO
