## (1) Recursive definition of P(C)

$$
P(C) =
\begin{cases}
0 & \text{if } w_i > C \text{ for every } i \\
\displaystyle\max_{0 \le i \le n-1,\ w_i \le C} \bigl( P(C - w_i) + p_i \bigr) & \text{otherwise}
\end{cases}
$$

Think about the last object you put into the knapsack, say one of type $i$. Once it's in, the remaining space $C - w_i$ must also be packed as profitably as possible. If it weren't, you could repack that space and get a higher total. Supply is unlimited, so type $i$ can still be used in that smaller problem. The subproblem therefore has the same form as the original, with only the capacity changed.

You don't know which type comes last in the best packing, so you try every type that fits and take the maximum. The first case handles capacities too small for any object, where the profit is 0. Since every $w_i > 0$, this includes the base case $P(0) = 0$.

## (2) Subproblem graph for P(15), with w = (4, 5, 8) and p = (7, 6, 9)

![Knapsack-graph](knapsack_subproblem_graph_p15.png)

**How to read the graph:**

- **Nodes** are subproblems P(c), with each node's computed value written underneath.
- **Edges** go from $P(c)$ to $P(c - w_i)$, meaning "$P(c)$ depends on $P(c - w_i)$." Each edge stands for putting one object of type $i$ into the knapsack.
- **Gray nodes** (P(3) down to P(0)) are base cases. Nothing fits in them, so their value is 0.

Only 10 distinct subproblems appear, even though a naive recursion tree would contain many repeats. P(7), P(6), P(3), P(1) and P(2) are each reached from more than one parent, with P(2) reached from three. These overlapping subproblems are exactly why dynamic programming beats plain recursion here.

The graph has no cycles, because every edge strictly decreases the capacity. That means we can solve the nodes bottom-up, from small c to large c.

**Final answer:** P(15) = 21, from three type-0 objects (weight 12, profit 21). For comparison:
- 4+4+5 = 13 weight gives profit 20.
- 5+5+5 = 15 weight gives profit 18.
- 8+4 = 12 weight gives profit 16.

## (3) Bottom-up DP algorithm

```
MaxProfit(C, w[0..n-1], p[0..n-1]):
    P[0..C] ← 0
    for c ← 1 to C:
        for i ← 0 to n-1:
            if w[i] ≤ c and P[c − w[i]] + p[i] > P[c]:
                P[c] ← P[c − w[i]] + p[i]
    return P[C]
```

**Explanation:** This algorithm fills the table in the order the graph requires. When it computes P[c], every P[c − wᵢ] is already done, because c − wᵢ < c. Each table entry is the recurrence from (1) evaluated exactly once. Starting P[c] at 0 handles the "nothing fits" case.

- **Time:** O(nC), because the algorithm fills C entries and each one checks n types.
- **Space:** O(C), for the one-dimensional table.

A one-dimensional table is enough because of the unlimited supply: a subproblem is described by its capacity alone. In 0/1 knapsack, where each object can be used only once, a subproblem must also record which objects are still available, $P(i, c)$, which makes the subproblem space two-dimensional.

## (4) Code and results

The Python implementation is in the file below. It also traces back which objects were chosen.

**(a) P(16), with w = (4, 5, 8) and p = (7, 6, 9)**
```
4a: w=[4, 5, 8], p=[7, 6, 9]
  c   :   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16
  P(c):   0   0   0   0   7   7   7   7  14  14  14  14  21  21  21  21  28
  P(16) = 28, items used per type = [4, 0, 0]
```
Type 0 has the best profit per unit weight: 7/4 = 1.75, compared with 1.2 for type 1 and 1.125 for type 2. Since 16 is divisible by 4, four type-0 objects fill the knapsack exactly with no wasted space. The table also matches the values in the graph above, for example P(15) = 21.

**(b) P(12), with w = (5, 6, 7) and p = (7, 6, 9)**
```
4b: w=[5, 6, 7], p=[7, 6, 9]
  c   :   0   1   2   3   4   5   6   7   8   9  10  11  12
  P(c):   0   0   0   0   0   7   7   9   9   9  14  14  16
  P(12) = 16, items used per type = [1, 0, 1]
```
Weight 5 + 7 = 12 fills the knapsack exactly, for profit 7 + 9 = 16.

This case shows why DP is needed. A greedy approach that picks the best ratio first (type 0, at 1.4) would take two type-0 objects for weight 10 and profit 14, wasting 2 units of capacity. The DP considers every combination through its subproblems and finds the better mix.

To reproduce part 4's output, run `python3 knapsack.py`.