"""Summarise results_x.csv into the markdown tables used in across_languages.md."""
import csv
import statistics as st
import sys
from collections import defaultdict

LANGS = ["C", "C++", "Rust", "Go", "Java", "C#", "JavaScript", "Python"]
LABELS = ["8V", "1%", "10%", "25%", "50%", "75%", "90%", "100%", "adv"]
LABEL_NAME = {"8V": "\\|E\\| = 8\\|V\\|", "adv": "adversarial"}

rows = list(csv.DictReader(open(sys.argv[1] if len(sys.argv) > 1 else "results_x.csv")))
bad = [r for r in rows if r["check"] != "ok" or r["ta_ms"] == "NA" or r["tb_ms"] == "NA"]

cell = defaultdict(list)          # (V, label, lang) -> list of (ta, tb)
E_of = {}
for r in rows:
    if r in bad:
        continue
    k = (int(r["V"]), r["label"], r["lang"])
    cell[k].append((float(r["ta_ms"]), float(r["tb_ms"])))
    E_of[(int(r["V"]), r["label"])] = int(r["E"])

Vs = sorted({k[0] for k in cell})


def ratio(k):
    rs = [a / b for a, b in cell[k]]
    return st.median(rs), min(rs), max(rs), len(rs)


def fmt(x):
    if x >= 100:
        return f"{x:.0f}"
    if x >= 10:
        return f"{x:.1f}"
    return f"{x:.2f}"


print(f"<!-- rows: {len(rows)}, rejected (check mismatch or missing time): {len(bad)} -->\n")

# ---- ratio tables, one per |V|
for V in Vs:
    labels = [l for l in LABELS if any((V, l, g) in cell for g in LANGS)]
    print(f"#### \\|V\\| = {V:,}\n")
    print("| Graph | \\|E\\| | " + " | ".join(LANGS) + " |")
    print("|---|---:|" + "---:|" * len(LANGS))
    for l in labels:
        out = []
        for g in LANGS:
            k = (V, l, g)
            if k not in cell:
                out.append("—")
                continue
            m, lo, hi, n = ratio(k)
            s = fmt(m)
            if m < 1 / 1.25:
                s = f"**{s}**"          # (a) clearly faster
            elif m <= 1.25:
                s = f"*{s}*"            # within +-25 %: tie
            out.append(s)
        print(f"| {LABEL_NAME.get(l, l)} | {E_of[(V, l)]:,} | " + " | ".join(out) + " |")
    print()

# ---- where (a) wins
print("#### Where (a) is clearly faster (median ratio below 0.8)\n")
print("| Language | " + " | ".join(f"\\|V\\|={V:,}" for V in Vs) + " |")
print("|---|" + "---|" * len(Vs))
for g in LANGS:
    out = []
    for V in Vs:
        wins = [l for l in LABELS if (V, l, g) in cell and l != "adv" and ratio((V, l, g))[0] < 0.8]
        has = any((V, l, g) in cell for l in LABELS)
        out.append(", ".join(wins) if wins else ("none" if has else "—"))
    print(f"| {g} | " + " | ".join(out) + " |")
print()

# ---- absolute times at a few reference points
print("#### Absolute times (ms, median)\n")
refs = [(4000, "8V"), (4000, "100%"), (8000, "50%"), (16000, "8V")]
print("| Language | " + " | ".join(f"{LABEL_NAME.get(l, l)}, \\|V\\|={V:,}: (a) / (b)" for V, l in refs) + " |")
print("|---|" + "---:|" * len(refs))
for g in LANGS:
    out = []
    for V, l in refs:
        k = (V, l, g)
        if k in cell:
            a = st.median(x for x, _ in cell[k]); b = st.median(y for _, y in cell[k])
            out.append(f"{fmt(a)} / {fmt(b)}")
        else:
            out.append("—")
    print(f"| {g} | " + " | ".join(out) + " |")
print()

# ---- run-to-run spread (3 repeats, |V| <= 4000)
print("#### Run-to-run spread of the ratio (\\|V\\| ≤ 4,000, 3 repeats)\n")
print("| Language | median (max ÷ min) | worst (max ÷ min) |")
print("|---|---:|---:|")
for g in LANGS:
    spreads = [ratio(k)[2] / ratio(k)[1] for k in cell if k[2] == g and k[0] <= 4000 and len(cell[k]) > 1]
    if spreads:
        print(f"| {g} | {st.median(spreads):.2f} | {max(spreads):.2f} |")
print()
if bad:
    print("Rejected rows:")
    for r in bad:
        print("  ", r)
