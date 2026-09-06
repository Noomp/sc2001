"""Regenerate every figure in the report from the raw CSV measurements.

Run it from anywhere:  python3 plots.py
It reads the CSVs from, and writes the PNGs to, the directory this file lives in.

Each graph is its own figure (fig4 keeps its two bar panels, which are already
uncluttered). Data lines carry no per-point markers -- the sweeps have up to 256
points and markers turn them into a smear -- so line colour and style alone
distinguish the series.
"""
import csv, math, sys, os, collections
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
sys.setrecursionlimit(200000)

# Look for model.py (the exact average-case cost model) next to this script.
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from model import H, C_ins, C_hyb, C_ms

plt.rcParams.update({
    "figure.dpi": 140, "font.size": 9, "axes.grid": True,
    "grid.alpha": .3, "axes.spines.top": False, "axes.spines.right": False,
})
OUT = os.path.join(HERE, "")
SIZE = (6.4, 4.2)          # one graph per figure, room to breathe
LW   = 1.3                 # data lines
LWR  = 0.9                 # reference / model lines


def load(f):
    return list(csv.DictReader(open(OUT + f)))


def save(fig, name):
    fig.tight_layout()
    fig.savefig(OUT + name)
    plt.close(fig)
    print("  wrote", name)


def shade(i, n_curves):
    """Consistent colour ramp for 'one curve per input size' plots."""
    return plt.cm.viridis(i / max(n_curves - 1, 1) * .85)


# ================================================================= (c)(i)
A   = load("exp_a.csv")
n_  = [int(r["n"]) for r in A]
hc  = [int(r["hybrid_comparisons"]) for r in A]
mc  = [int(r["merge_comparisons"]) for r in A]
mod = [C_hyb(n, 16) for n in n_]

# --- fig 1a: absolute counts, log-log
fig, ax = plt.subplots(figsize=SIZE)
ax.loglog(n_, hc, "-",  lw=LW,  color="#1f77b4", label="Hybrid, S = 16 (measured)")
ax.loglog(n_, mod, "--", lw=LWR, color="#111",    label="Hybrid, S = 16 (theoretical model)")
ax.loglog(n_, mc, "-",  lw=LW,  color="#d62728", label="Original merge sort (measured)")
ax.loglog(n_, [x * math.log2(x) for x in n_], ":", lw=LWR, color="gray",
          label=r"$n\log_2 n$ reference")
ax.set_xlabel("input size n")
ax.set_ylabel("total key comparisons (absolute count)")
ax.set_title("(c)(i) Key comparisons vs n   (S fixed at 16)")
ax.legend(fontsize=7.5)
save(fig, "fig1a_comparisons_vs_n.png")

# --- fig 1b: same data divided by n
fig, ax = plt.subplots(figsize=SIZE)
ax.semilogx(n_, [c / x for c, x in zip(hc, n_)], "-", lw=LW, color="#1f77b4",
            label="Hybrid, S = 16")
ax.semilogx(n_, [c / x for c, x in zip(mc, n_)], "-", lw=LW, color="#d62728",
            label="Original merge sort")
ax.semilogx(n_, [math.log2(x) for x in n_], ":", lw=LWR, color="gray", label=r"$\log_2 n$")
ax.set_xlabel("input size n")
ax.set_ylabel("key comparisons per element,  C(n) / n")
ax.set_title("(c)(i) Same data $\\div$ n:  straight line $\\Rightarrow\\ \\Theta(n\\log n)$")
ax.legend(fontsize=7.5)
save(fig, "fig1b_comparisons_per_element.png")

# ================================================================= (c)(ii)
B = load("exp_b.csv")
by_n = collections.defaultdict(list)
for r in B:
    by_n[int(r["n"])].append((int(r["S"]), int(r["comparisons"]), float(r["time"])))
ns_b = sorted(by_n)

# --- fig 2a: key comparisons vs S, each curve / its own S = 1
fig, ax = plt.subplots(figsize=SIZE)
for i, n in enumerate(ns_b):
    rows = sorted(by_n[n]); col = shade(i, len(ns_b))
    S = [x[0] for x in rows]; c = [x[1] for x in rows]
    base = c[0]                                    # count at S = 1 (= pure merge sort)
    ax.plot(S, [x / base for x in c], "-", lw=LW, color=col,
            label=f"n = {n:,}   (S=1: {base:,} cmp)")
    ax.plot(S, [C_hyb(n, s) / base for s in S], "--", lw=.7, color="k",
            label="exact model" if i == 0 else None)
ax.axhline(1.0, color="gray", lw=.6)
ax.set_xlabel("threshold S")
ax.set_ylabel("key comparisons $\\div$ count at S = 1")
ax.set_title("(c)(ii) Key comparisons vs S   (each curve $\\div$ its own S=1)")
ax.legend(fontsize=7)
save(fig, "fig2a_comparisons_vs_S.png")

# --- fig 2b: CPU time vs S, same runs, same normalisation
fig, ax = plt.subplots(figsize=SIZE)
for i, n in enumerate(ns_b):
    rows = sorted(by_n[n]); col = shade(i, len(ns_b))
    S = [x[0] for x in rows]; t = [x[2] for x in rows]
    ax.plot(S, [x / t[0] for x in t], "-", lw=1.0, color=col,
            label=f"n = {n:,}   (S=1: {t[0]*1000:,.3g} ms)")
ax.axhline(1.0, color="gray", lw=.6)
ax.set_xlabel("threshold S")
ax.set_ylabel("CPU time $\\div$ time at S = 1")
ax.set_title("(c)(ii) CPU time vs S   (same runs, same normalisation)")
ax.legend(fontsize=7)
save(fig, "fig2b_time_vs_S.png")

# ================================================================= (c)(iii)
C = load("exp_c.csv")
by_n2 = collections.defaultdict(list)
for r in C:
    by_n2[int(r["n"])].append((int(r["S"]), int(r["comparisons"]), float(r["time"])))
ns_c = sorted(by_n2)

# --- fig 3a: comparison cost vs S, each curve / its own minimum
fig, ax = plt.subplots(figsize=SIZE)
for i, n in enumerate(ns_c):
    rows = sorted(by_n2[n]); col = shade(i, len(ns_c))
    S = [x[0] for x in rows]; c = [x[1] for x in rows]
    ax.semilogx(S, [x / min(c) for x in c], "-", lw=LW, color=col,
                label=f"n = {n:,}   (best: {min(c):,} cmp)")
ax.set_xlabel("threshold S")
ax.set_ylabel("key comparisons $\\div$ best for that n")
ax.set_title("(c)(iii) Comparison cost vs S   (1.0 = that n's own minimum)")
ax.legend(fontsize=7)
save(fig, "fig3a_comparison_cost_vs_S.png")

# --- fig 3b: CPU time vs S, with each size's optimum starred
fig, ax = plt.subplots(figsize=SIZE)
ax.axhspan(1.0, 1.02, color="gold", alpha=.20, zorder=0, label="within 2 % of best")
for i, n in enumerate(ns_c):
    rows = sorted(by_n2[n]); col = shade(i, len(ns_c))
    S = [x[0] for x in rows]; t = [x[2] for x in rows]
    bs = rows[t.index(min(t))][0]
    # best S goes in the legend too: several sizes share an optimum, so their
    # stars sit at the same point and would otherwise hide one another.
    ax.semilogx(S, [x / min(t) for x in t], "-", lw=1.1, color=col,
                label=f"n = {n:,}   (best S = {bs}, {min(t)*1000:,.3g} ms)")
for i, n in enumerate(ns_c):                       # stars on top of every line
    rows = sorted(by_n2[n]); col = shade(i, len(ns_c))
    t = [x[2] for x in rows]
    ax.plot([rows[t.index(min(t))][0]], [1.0], "*", ms=18, color=col,
            markeredgecolor="white", markeredgewidth=1.1, zorder=10, clip_on=False)
ax.set_xlabel("threshold S")
ax.set_ylabel("CPU time $\\div$ best for that n")
ax.set_title("(c)(iii) CPU time vs S   ($\\bigstar$ = that n's optimum)")
ax.legend(fontsize=7)
save(fig, "fig3b_time_vs_S_optimal.png")

# ================================================================= (d)
D   = load("exp_d.csv")
hyb = [r for r in D if r["algorithm"] == "hybrid"]
ms_ = [r for r in D if r["algorithm"] == "mergesort"]
def med(v): v = sorted(v); return v[len(v) // 2]
hC = med([int(r["comparisons"]) for r in hyb]); mC = med([int(r["comparisons"]) for r in ms_])
hT = med([float(r["cpu_time"]) for r in hyb]);  mT = med([float(r["cpu_time"]) for r in ms_])

fig, ax = plt.subplots(1, 2, figsize=(7.2, 3.2))
S_d = hyb[0]["S"]                      # read the threshold from the data, don't hardcode it
lbl = [f"Hybrid\n(S = {S_d})", "Original\nmerge sort"]
b1 = ax[0].bar(lbl, [hC / 1e6, mC / 1e6], color=["#1f77b4", "#d62728"], width=.55)
ax[0].set_ylabel("key comparisons (millions)")
ax[0].set_title("(d) n = 10,000,000: comparisons")
ax[0].bar_label(b1, fmt="%.1fM", fontsize=8)
b2 = ax[1].bar(lbl, [hT, mT], color=["#1f77b4", "#d62728"], width=.55)
ax[1].set_ylabel("CPU time (s)")
ax[1].set_title("(d) n = 10,000,000: CPU time")
ax[1].bar_label(b2, fmt="%.3f s", fontsize=8)
save(fig, "fig4_part_d.png")


# ================================================================= T7: f and f'
# Why "the slope has a minimum" is NOT "the function has a minimum".
# Smooth stand-in for the harmonic number, so the closed form can be differentiated.
Hc  = lambda m: math.log(m) + 0.5772156649 + 1/(2*m)
fp_full = lambda m: -1/(m*math.log(2)) + 0.25 + (1+Hc(m))/m**2    # slope of f(m)

grid   = [1 + i/200 for i in range(0, 3800)]
M_FLAT = 3.54            # where fp_full bottoms out (still positive)

# --- fig 6a: real cost per element for a concrete n, with the exact model overlaid
N6 = 10**6
LOG2N = math.log2(N6)
f_cost = lambda m: LOG2N - math.log2(m) + (m+3)/4 - (2+Hc(m))/m   # f(m) in real units

# Exact model, plotted against the TRUE average leaf size n/(number of leaves).
# Using n/2^k instead would be wrong at boundary S, where leaves span two depths.
import functools
@functools.lru_cache(maxsize=None)
def leafcount(n, S):
    if n <= 1 or n <= S: return 1
    p = (n + 1) // 2
    return leafcount(p, S) + leafcount(n - p, S)

exact_pts = {}
for S in range(1, 64):
    m = N6 / leafcount(N6, S)
    if m <= 19: exact_pts[round(m, 6)] = C_hyb(N6, S) / N6

fig, ax = plt.subplots(figsize=SIZE)
ax.plot(grid, [f_cost(m) for m in grid], "-", lw=1.6, color="#1f77b4",
        label="closed form $f(m)$ (§T5)")
xs = sorted(exact_pts)
ax.plot(xs, [exact_pts[x] for x in xs], "o", ms=5, color="#111", zorder=6,
        label="exact recurrence (theory)")
ax.plot([1], [f_cost(1)], "o", ms=7, color="#1f77b4", zorder=5)
ax.annotate("lowest at $m=1$, and\nonly climbs from there",
            xy=(1, f_cost(1)), xytext=(2.3, 17.55), fontsize=7.5, color="#1f77b4",
            arrowprops=dict(arrowstyle="->", color="#1f77b4", lw=.8))
ax.set_xlabel("effective leaf size $m$")
ax.set_ylabel("key comparisons per element   ($n=10^6$)")
ax.set_title("T7: the cost curve $f(m)$   (lower = cheaper)")
ax.set_xlim(0.5, 19); ax.set_ylim(17.3, 21.9)
ax.legend(fontsize=7.5, loc="lower right")
save(fig, "fig6a_cost_vs_leafsize.png")

# --- fig 6b: the slopes
fig, ax = plt.subplots(figsize=SIZE)
ax.plot(grid, [fp_full(m) for m in grid], "-", lw=1.6, color="#1f77b4", label="$f'(m)$")
ax.axhline(0, color="k", lw=1.2)
ax.plot([M_FLAT], [fp_full(M_FLAT)], "o", ms=7, color="#1f77b4", zorder=5)
ax.annotate("sags to $+0.08$ at $m=3.5$,\nthen turns back up — never\nreaches zero, so $f$ never\nstops rising",
            xy=(M_FLAT, fp_full(M_FLAT)), xytext=(5.4, 0.40), fontsize=7.5, color="#1f77b4",
            arrowprops=dict(arrowstyle="->", color="#1f77b4", lw=.8))
ax.set_xlabel("effective leaf size $m$")
ax.set_ylabel("slope of the cost curve,  $f'(m)$")
ax.set_title("T7: the slope $f'(m)$ stays positive for every $m \\geq 1$")
ax.set_xlim(0.5, 19); ax.set_ylim(-0.16, 0.62)
ax.legend(fontsize=7.5, loc="upper right")
save(fig, "fig6b_slope_vs_leafsize.png")

# ================================================================= base case
L  = load("leafstats.csv")
m  = [int(r["m"]) for r in L]
ia = [float(r["insertion_avg"]) for r in L]
ma = [float(r["mergesort_avg"]) for r in L]
fig, ax = plt.subplots(figsize=SIZE)
ax.plot(m, ia, "-",  lw=LW,  color="#ff7f0e", label="Insertion sort (measured)")
ax.plot(m, [C_ins(x) for x in m], "--", lw=LWR, color="k", label=r"$m(m+3)/4-H_m$")
ax.plot(m, ma, "-",  lw=LW,  color="#1f77b4", label="Merge sort (measured)")
ax.set_xlabel("subarray size m")
ax.set_ylabel("average key comparisons to sort m elements")
ax.set_title("Cost of the base case   (absolute counts)")
ax.legend(fontsize=7.5)
save(fig, "fig5_base_case.png")

print()
print(f"part d medians: hybrid {hC:,} cmp / {hT:.3f}s ; merge {mC:,} cmp / {mT:.3f}s")
print(f"comparison ratio {hC/mC:.4f}, time ratio {hT/mT:.4f}, speedup {(mT/hT-1)*100:.1f}%")
