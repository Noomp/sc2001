def max_profit(C, w, p):
    """Unbounded knapsack, bottom-up DP. Returns (P table, choice table)."""
    n = len(w)
    P = [0] * (C + 1)          # P[c] = max profit with capacity c
    choice = [-1] * (C + 1)    # last item type added to reach P[c]
    for c in range(1, C + 1):
        for i in range(n):
            if w[i] <= c and P[c - w[i]] + p[i] > P[c]:
                P[c] = P[c - w[i]] + p[i]
                choice[c] = i
    return P, choice

def items_used(C, w, choice):
    counts = [0] * len(w)
    c = C
    while c > 0 and choice[c] != -1:
        counts[choice[c]] += 1
        c -= w[choice[c]]
    return counts

def run(C, w, p, label):
    P, choice = max_profit(C, w, p)
    print(f"{label}: w={w}, p={p}")
    print("  c   :", " ".join(f"{c:>3}" for c in range(C + 1)))
    print("  P(c):", " ".join(f"{v:>3}" for v in P))
    print(f"  P({C}) = {P[C]}, items used per type = {items_used(C, w, choice)}\n")

if __name__ == "__main__":
    run(16, [4, 5, 8], [7, 6, 9], "4a")
    run(12, [5, 6, 7], [7, 6, 9], "4b")
