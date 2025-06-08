## Envelope width

Given that envelope width has a substantial effect on pruning ratio (shown empirically, mathematical proof should also be found), we want to assess how it scales with the various flat envelope index parameters:
1. $\beta$ - Length per length group, $N_l:=\left\lceil (l_{\max}-l_{\min}+1)/\beta \right\rceil$
2. $\gamma$ - Positions per envelope, $N_e:=\left\lceil (m-l_{\min}+1)/\gamma \right\rceil$
3. $s$ - The segment length, $N_s:=\left\lfloor l_{\max}/s \right\rfloor$

We can define the envelope width for a single segment as:
$$
\begin{array}{rcl}
    r_{i,j,k} & := & u_{i,j,k} - l_{i,j,k} \\[4pt]
    & \text{where} & \\[4pt]
    u_{i,j,k} & := & \max(E_{i,j,k}) \\[4pt]
    l_{i,j,k} & := & \min(E_{i,j,k}) \\
\end{array}
$$
Where $E_{i,j,k}$ is the set of PAA segment values in the $k$'th segment of the $j$'th envelope of the $i$'th length group, defined as:
$$
\begin{array}{rcl}
    E_{i,j,k} & := & \left\{ \dfrac{S_{p,l,k}-\sigma^t_{p:l}}{s\mu^t_{p:l}}  \mid t\in\mathcal{D}, l\in L_i,p\in P_j \right\} \\[4pt]
    & \text{where} & \\[4pt]
    \mu^t_{p:l} & := & \sum_{i'=p}^{p+l-1} t_{i'} / l \\[4pt]
    \sigma^t_{p:l} & := & \sum_{i'=p}^{p+l-1}\left(t_{i'}-\mu^t_{p:l}\right)^2/l \\[4pt]
    S_{p,l,k}^t & := & \sum_{i'=p+(k-1)s}^{p+ks-1}t_{i'}\\[4pt]
    L_i & := & \left[l_{\min}+(i-1)\beta,l_{\min}+i\beta\right) \\[4pt]
    P_j & := & \left[j\gamma, (j+1)\gamma\right) \\
\end{array}
$$

We may wish to find (the complexity class of) the following:
1. $\mathbb{E}_\beta[r_{i,j,k}]$ - How the range scales with the size of length groups
2. $\mathbb{E}_\gamma[r_{i,j,k}]$ - How the range scales with the size of envelopes (position groups)
3. $\mathbb{E}_s[r_{i,j,k}]$ - How the range scales with the segment size

**However**, empirical evidence only supports the claim that pruning power is inversely proportional to envelope width **using the same index parameters**. A different approach should be identified.

