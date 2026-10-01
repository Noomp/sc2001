"""
Exact average-case model of key comparisons for the hybrid merge/insertion sort.

This file is the "theory" curve in every figure of the report. It evaluates the
recurrence exactly (no asymptotics, no fitting) by memoised recursion, so it can
be compared directly against the measured counts in the CSV files.

The three formulas it uses are derived in the report:

  H(m)      = 1 + 1/2 + ... + 1/m          the m-th harmonic number (~ ln m + 0.577)

  C_ins(m)  = m(m+3)/4 - H(m)              average key comparisons of insertion sort
                                           on m random distinct keys.  The -H(m) term
                                           comes from the case where the key being
                                           inserted is a new minimum: the scan runs off
                                           the left end and the final *failing* key
                                           comparison never happens.

  M(p,q)    = p + q - p/(q+1) - q/(p+1)    average key comparisons to merge two random
                                           sorted runs of lengths p and q.  The merge
                                           loop stops as soon as one run is exhausted;
                                           p/(q+1) is the expected number of elements of
                                           the first run that exceed everything in the
                                           second and are therefore copied for free.
"""
from functools import lru_cache


@lru_cache(maxsize=None)
def H(m):
    """m-th harmonic number: 1 + 1/2 + 1/3 + ... + 1/m."""
    return sum(1.0 / i for i in range(1, m + 1))


@lru_cache(maxsize=None)
def C_ins(m):
    """Average key comparisons of insertion sort on m distinct random keys."""
    return m * (m + 3) / 4.0 - H(m)


@lru_cache(maxsize=None)
def C_hyb(n, S):
    """Average key comparisons of the hybrid sort on n keys with threshold S."""
    if n <= 1:
        return 0.0                       # nothing to do
    if n <= S:
        return C_ins(n)                  # base case: hand it to insertion sort

    # The C code splits with  mid = left + (right - left) / 2,  which puts
    # ceil(n/2) elements in the left half and floor(n/2) in the right half.
    p = (n + 1) // 2                     # left half  = ceil(n/2)
    q = n - p                            # right half = floor(n/2)

    merge_cost = p + q - p / (q + 1.0) - q / (p + 1.0)
    return C_hyb(p, S) + C_hyb(q, S) + merge_cost


@lru_cache(maxsize=None)
def C_ms(n):
    """Average key comparisons of the ORIGINAL merge sort.

    With S = 1 the hybrid's base case only ever fires on single elements, where
    insertion sort does no work at all, so the hybrid degenerates exactly into
    the original merge sort.
    """
    return C_hyb(n, 1)


if __name__ == "__main__":
    import sys
    sys.setrecursionlimit(200000)
    print(f"{'n':>12} {'pure merge sort':>18} {'hybrid S=16':>18} {'hybrid S=20':>18}")
    for n in (1000, 100000, 1000000, 10000000):
        print(f"{n:>12,} {round(C_ms(n)):>18,} {round(C_hyb(n, 16)):>18,} {round(C_hyb(n, 20)):>18,}")
