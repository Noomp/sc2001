"""Regenerate every figure in the report from the raw CSV measurements.

Run it from anywhere:  python3 plots.py
It reads the CSVs from, and writes the PNGs to, the directory this file lives in.

fig1 ... fig11    the formal figures: implementations (a) and (b) only   (exp_*.csv)
extra1 ... extra3 supporting figures for the report's "Extra information"
                  section, using variants outside the brief              (extra_*.csv)
"""
import csv, math, os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

HERE = os.path.dirname(os.path.abspath(__file__))
plt.rcParams.update({
    "figure.dpi": 140, "font.size": 9, "axes.grid": True,
    "grid.alpha": .3, "axes.spines.top": False, "axes.spines.right": False,
})
SIZE = (6.4, 4.2)
LW, LWR = 1.4, 0.9

# one fixed colour per implementation, used in every figure
COL = {"mat_arr": "#2a78d6", "list_heap": "#eb6834",
       "mat_arr_bf": "#1baf7a", "list_arr": "#eda100", "mat_heap": "#e87ba4"}
NAME = {"mat_arr": "(a) matrix + array", "list_heap": "(b) lists + heap",
        "mat_arr_bf": "(a) with branch-free relaxation", "list_arr": "lists + array",
        "mat_heap": "matrix + heap"}
SHADE = ["#9cc3ee", "#2a78d6", "#0d3f7a"]          # one hue, light -> dark (|V| small -> large)
SHADE_O = ["#f5b394", "#eb6834", "#8a3110"]


def load(f):
    rows = list(csv.DictReader(open(os.path.join(HERE, f))))
    for r in rows:
        for k in ("V", "E", "pq_ops", "edge_ops", "updates"):
            r[k] = int(r[k])
        r["time_s"] = float(r["time_s"])
    return rows


def series(rows, **kw):
    out = [r for r in rows if all(r[k] == v for k, v in kw.items())]
    return sorted(out, key=lambda r: (r["V"], r["E"]))


def save(fig, name):
    fig.tight_layout()
    fig.savefig(os.path.join(HERE, name))
    plt.close(fig)
    print("  wrote", name)


vsV, vsE, sp, adv = (load(f"exp_{m}.csv") for m in ("vsV", "vsE", "sparse", "adv"))
xvsE = load("extra_vsE.csv")
DENS = [("deg8", "|E| = 8|V| (sparse)"), ("half", "|E| = |V|(|V|-1)/2"), ("full", "|E| = |V|(|V|-1) (complete)")]
VE = [1000, 3000, 10000]

# ---------------------------------------------------------------- (a)
fig, ax = plt.subplots(figsize=SIZE)
for (lab, txt), c in zip(DENS, SHADE):
    s = series(vsV, label=lab, variant="mat_arr")
    ax.plot([r["V"] for r in s], [r["time_s"] for r in s], color=c, lw=LW, label=txt)
xs = [500, 10000]
ax.plot(xs, [1.1e-9 * x * x for x in xs], "k--", lw=LWR, label=r"$1.1\,\mathrm{ns}\times|V|^2$")
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_xlabel("|V|"); ax.set_ylabel("CPU time (s)")
ax.set_title("(a) matrix + array: time vs |V| at three densities")
ax.legend(frameon=False)
save(fig, "fig1_a_time_vs_V.png")

fig, ax = plt.subplots(figsize=SIZE)
for (lab, txt), c in zip(DENS, SHADE):
    s = series(vsV, label=lab, variant="mat_arr")
    ax.plot([r["V"] for r in s], [r["time_s"] / r["V"] ** 2 * 1e9 for r in s], color=c, lw=LW, label=txt)
ax.set_xscale("log")
ax.set_xlabel("|V|"); ax.set_ylabel(r"CPU time / $|V|^2$  (ns)")
ax.set_ylim(bottom=0)
ax.set_title("(a) matrix + array: time per $|V|^2$")
ax.legend(frameon=False)
save(fig, "fig2_a_time_per_V2.png")

fig, ax = plt.subplots(figsize=SIZE)
for V, c in zip(VE, SHADE):
    s = series(vsE, label=f"V{V}", variant="mat_arr")
    ax.plot([r["E"] for r in s], [r["time_s"] for r in s], color=c, lw=LW, label=f"|V| = {V:,}")
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_xlabel("|E|"); ax.set_ylabel("CPU time (s)")
ax.set_title("(a) matrix + array: time vs |E| at fixed |V|")
ax.legend(frameon=False)
save(fig, "fig3_a_time_vs_E.png")

# ---------------------------------------------------------------- (b)
fig, ax = plt.subplots(figsize=SIZE)
for V, c in zip(VE, SHADE_O):
    s = series(vsE, label=f"V{V}", variant="list_heap")
    ax.plot([r["E"] for r in s], [r["time_s"] for r in s], color=c, lw=LW, label=f"|V| = {V:,}")
xs = [3e4, 1e8]
ax.plot(xs, [1.2e-9 * x for x in xs], "k--", lw=LWR, label=r"$1.2\,\mathrm{ns}\times|E|$")
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_xlabel("|E|"); ax.set_ylabel("CPU time (s)")
ax.set_title("(b) lists + heap: time vs |E| at fixed |V|")
ax.legend(frameon=False)
save(fig, "fig4_b_time_vs_E.png")

fig, ax = plt.subplots(figsize=SIZE)
s = series(sp, variant="list_heap")
ax.plot([r["V"] for r in s], [r["time_s"] / ((r["V"] + r["E"]) * math.log2(r["V"])) * 1e9 for r in s],
        color=COL["list_heap"], lw=LW, label=r"CPU time (ns) / $((|V|+|E|)\log_2|V|)$")
ax.plot([r["V"] for r in s], [r["pq_ops"] / (r["V"] * math.log2(r["V"])) for r in s],
        color="grey", lw=LW, ls="--", label=r"heap comparisons / $(|V|\log_2|V|)$   (a pure count)")
ax.set_xscale("log", base=2)
ax.set_ylim(bottom=0)
ax.set_xlabel("|V|   (|E| = 8|V|)"); ax.set_ylabel("normalised value")
ax.set_title("(b) lists + heap on large sparse graphs")
ax.legend(frameon=False)
save(fig, "fig5_b_sparse_normalised.png")

fig, ax = plt.subplots(figsize=SIZE)
for V, c in zip(VE, SHADE_O):
    s = series(vsE, label=f"V{V}", variant="list_heap")
    ax.plot([r["E"] / r["V"] for r in s], [r["updates"] / r["V"] for r in s], color=c, lw=LW,
            label=f"measured, |V| = {V:,}")
ks = [10 ** (x * 4 / 199) for x in range(200)]
ax.plot(ks, [math.log(k) for k in ks], "k--", lw=LWR, label=r"$\ln(|E|/|V|)$")
ax.plot(ks, ks, color="grey", lw=LWR, ls=":", label=r"worst case $|E|/|V|$ (every edge succeeds)")
ax.set_xscale("log"); ax.set_ylim(0, 12)
ax.set_xlabel("average out-degree |E|/|V|"); ax.set_ylabel("decrease-keys per vertex")
ax.set_title("(b) how many relaxations actually succeed (random weights)")
ax.legend(frameon=False)
save(fig, "fig6_b_decrease_keys.png")

# ---------------------------------------------------------------- (c)
fig, ax = plt.subplots(figsize=SIZE)
for V, ls in ((1000, ":"), (10000, "-")):
    for a in ("mat_arr", "list_heap"):
        s = series(vsE, label=f"V{V}", variant=a)
        ax.plot([r["E"] for r in s], [r["time_s"] for r in s], color=COL[a], lw=LW, ls=ls,
                label=f"{NAME[a]}, |V| = {V:,}")
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_xlabel("|E|"); ax.set_ylabel("CPU time (s)")
ax.set_title("(c) (a) vs (b): time vs |E|, tree to complete graph")
ax.legend(frameon=False, fontsize=7.5)
save(fig, "fig7_c_a_vs_b.png")

fig, ax = plt.subplots(figsize=SIZE)
for V, c in zip(VE, SHADE):
    b = {r["E"]: r for r in series(vsE, label=f"V{V}", variant="list_heap")}
    s = series(vsE, label=f"V{V}", variant="mat_arr")
    ax.plot([r["E"] / (V * (V - 1)) for r in s], [r["time_s"] / b[r["E"]]["time_s"] for r in s], color=c,
            lw=LW, label=f"|V| = {V:,}")
ax.axhline(1, color="k", lw=LWR)
ax.text(1.2e-4, 1.12, "above the line: (b) is faster", fontsize=8)
ax.text(1.2e-4, 0.80, "below: (a) is faster", fontsize=8)
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_ylim(0.6, 300)
ax.set_xlabel("density |E| / (|V|(|V|-1))"); ax.set_ylabel("time(a) / time(b)")
ax.set_title("(c) speed-up of (b) over (a) on random graphs")
ax.legend(frameon=False)
save(fig, "fig8_c_speedup_vs_density.png")

fig, ax = plt.subplots(figsize=SIZE)
for lab, ls, txt in (("adv", "-", "adversarial weights"), ("dag_random", "--", "random weights")):
    for a in ("mat_arr", "list_heap"):
        s = series(adv, label=lab, variant=a)
        ax.plot([r["V"] for r in s], [r["time_s"] for r in s], color=COL[a], lw=LW, ls=ls,
                label=f"{NAME[a]}, {txt}")
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_xlabel("|V|   (complete DAG, |E| = |V|(|V|-1)/2)"); ax.set_ylabel("CPU time (s)")
ax.set_title("(c) the worst case of (b): every edge a full-height decrease-key")
ax.legend(frameon=False)
save(fig, "fig9_c_adversarial.png")

fig, ax = plt.subplots(figsize=SIZE)
for a in ("mat_arr", "list_heap"):
    s = series(sp, variant=a)
    ax.plot([r["V"] for r in s], [r["time_s"] for r in s], color=COL[a], lw=LW, label=NAME[a])
ax.set_xscale("log", base=2); ax.set_yscale("log")
ax.set_xlabel("|V|   (|E| = 8|V|)"); ax.set_ylabel("CPU time (s)")
ax.set_title("(c) sparse graphs up to 4 million vertices")
ax.legend(frameon=False)
save(fig, "fig10_c_sparse_scaling.png")

fig, ax = plt.subplots(figsize=SIZE)
for V, ls in ((3000, ":"), (10000, "-")):
    for a in ("mat_arr", "list_heap"):
        s = series(vsE, label=f"V{V}", variant=a)
        ax.plot([r["E"] for r in s], [r["pq_ops"] + r["edge_ops"] for r in s], color=COL[a], lw=LW, ls=ls,
                label=f"{NAME[a]}, random, |V| = {V:,}")
for a, mk in (("mat_arr", "o"), ("list_heap", "s")):
    s = series(adv, label="adv", variant=a)
    ax.plot([r["E"] for r in s], [r["pq_ops"] + r["edge_ops"] for r in s], color=COL[a], lw=0, marker=mk, ms=6,
            markeredgecolor="white", markeredgewidth=1.2, label=f"{NAME[a]}, adversarial DAG")
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_xlabel("|E|"); ax.set_ylabel("PQ operations + adjacency entries examined")
ax.set_title(r"(c) hardware-independent work: (a) $= 2|V|^2$ always, (b) $\approx |E|$ + heap")
ax.set_ylim(3e4, 1e11)
ax.legend(frameon=False, fontsize=7, loc="upper left", ncol=2, columnspacing=1)
save(fig, "fig11_c_operation_counts.png")

# ------------------------------------------------------------ extra information
fig, ax = plt.subplots(figsize=SIZE)
for V, c in zip(VE, SHADE):
    s = series(vsE, label=f"V{V}", variant="mat_arr")
    ax.plot([r["E"] for r in s], [r["time_s"] for r in s], color=c, lw=LW, label=f"(a), |V| = {V:,}")
    s = series(xvsE, label=f"V{V}", variant="mat_arr_bf")
    ax.plot([r["E"] for r in s], [r["time_s"] for r in s], color=c, lw=LW, ls="--",
            label=f"(a) branch-free, |V| = {V:,}")
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_xlabel("|E|"); ax.set_ylabel("CPU time (s)")
ax.set_title("Extra 1: (a) vs the same algorithm with branch-free relaxation")
ax.legend(frameon=False, fontsize=7.5)
save(fig, "extra1_a_branch_free.png")

fig, ax = plt.subplots(figsize=SIZE)
for V, c in zip(VE, SHADE):
    b = {r["E"]: r for r in series(vsE, label=f"V{V}", variant="list_heap")}
    s = series(xvsE, label=f"V{V}", variant="mat_arr_bf")
    ax.plot([r["E"] / (V * (V - 1)) for r in s], [r["time_s"] / b[r["E"]]["time_s"] for r in s], color=c,
            lw=LW, ls="--", label=f"|V| = {V:,}")
ax.axhline(1, color="k", lw=LWR)
ax.text(1.2e-4, 1.12, "above the line: (b) is faster", fontsize=8)
ax.text(1.2e-4, 0.80, "below: branch-free (a) is faster", fontsize=8)
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_ylim(0.6, 300)
ax.set_xlabel("density |E| / (|V|(|V|-1))"); ax.set_ylabel("time(branch-free a) / time(b)")
ax.set_title("Extra 2: speed-up of (b) over a branch-free (a)")
ax.legend(frameon=False)
save(fig, "extra2_c_speedup_branch_free.png")

fig, ax = plt.subplots(figsize=SIZE)
for a, src in (("mat_arr", vsE), ("list_arr", xvsE), ("mat_heap", xvsE), ("list_heap", vsE)):
    s = series(src, label="V10000", variant=a)
    ax.plot([r["E"] for r in s], [r["time_s"] for r in s], color=COL[a], lw=LW, label=NAME[a],
            ls="-" if a in ("mat_arr", "list_heap") else "--")
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_xlabel("|E|   (|V| = 10,000)"); ax.set_ylabel("CPU time (s)")
ax.set_title("Extra 3: changing only the representation, or only the queue")
ax.legend(frameon=False)
save(fig, "extra3_c_one_change_at_a_time.png")
