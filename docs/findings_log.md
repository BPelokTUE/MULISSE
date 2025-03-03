# Findings

## 26-02-2025

1. MULISSE performs better on *synthetic data* regardless of the number of channels ( #7 )
    - Synthetic data leads to more balanced indexes: Why? **Analyze some indexes to find out**.
2. Optimal envelope size is highly dependent on parameters e.g. the allowed query range ( #8 )
    - **Parametrization is needed**
3. Relative contrast doesn't seem to explain the difference in performance between datasets ( #9 )

## 05-03-2025

1. Pure iSAX performs poorly ( #10 )
    - Index size is much larger (relative to dataset size) than other indexes (iSAX + envelope or pure envelope)
    - Consequently index construction takes much longer
    - Searching the index is 1 to 2 orders of magnitude slower than iSAX + envelope or envelope, even though the pruning ratio ($1 - \frac{\text{subsequences examined}}{\text{subsequences in the index}}$) is very high
    - Improvements could be made to pure iSAX, but this does not seem like a promising direction. However, the poor performance does indicate that summarizing on the level of subsequences is beneficial or even necessary
2. Pure envelope performs slightly better than iSAX + envelope ( #11 )
    - This seems to suggest that focusing on envelope parametrization / improving envelope pruning power is a promising direction, and iSAX is not needed / should not be the focus
3. Index analysis - TODO ( #15 )