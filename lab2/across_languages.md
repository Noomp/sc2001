# Dijkstra (a) vs (b) Across Eight Languages

**Question.** Does the comparison between (a) *adjacency matrix + array priority queue* and
(b) *adjacency lists + indexed binary min-heap* depend on the programming language? Earlier
results disagreed: our C runs had (b) winning or tying everywhere on random graphs, while a
teammate's Python notebook had (a) clearly winning on dense graphs. This experiment holds
everything fixed except the language, so any difference left over comes from the language.

**Short answer.** The pattern is the same in all eight languages. (b) is faster at every
density below a complete graph. On complete graphs the two are roughly tied. (a) wins clearly
only on the adversarial graph. The language changes *how much* (b) wins by, from about 1.8× to
15× at 50 % density, but not *who* wins. Python with flat typed arrays follows the same
pattern. The teammate's opposite result came from his Python data layout, not from Python
itself.

Code and raw data: [`across_languages/`](across_languages/).

---

## What was held constant

| | How it was enforced |
|---|---|
| **Input graphs** | Every language reads the same binary files, produced once by the generator in [`dijkstra.c`](dijkstra.c): a random spanning tree from vertex 0 plus uniformly random extra edges, with integer weights 1–10⁶. The adversarial graph is the same construction as in the main report. |
| **Data layout** | The same in every language. The matrix is \|V\| rows of int32 (0 = no edge). The adjacency lists are three flat int32 arrays (offsets, targets, weights). Distances are int64, the finished flags are bytes, and the heap is two int32 arrays. Python uses the standard `array` module and JavaScript uses typed arrays, so neither stores values as separate boxed objects. |
| **Algorithm logic** | Line-for-line the same: the same loops, tests, tie-breaking and heap sift procedures. (a) is the vertex-indexed array with a finished flag; (b) is the indexed heap with decrease-key. Neither has the redundant "already finished?" check during relaxation. No language-specific optimisations: no unsafe code, no SIMD, no library heaps. |
| **Logical equivalence, verified** | Each program reports three values for both (a) and (b): the sum of all distances, the number of successful relaxations, and a hash of the order in which vertices were finished. Before timing, the driver compares these with C's values for the same graph. **All 414 measurements matched C exactly.** Every language takes the same path through the algorithm, not just the same final answer. |
| **Timing rule** | The same in every language. Warm up until at least 0.5 s of runs have elapsed (minimum one run). Then repeat each run in a batch until the batch lasts at least 30 ms, and take the median of 5 batches (3 if one run exceeds 1 s, 1 if it exceeds 10 s). Times are wall-clock from a monotonic clock. Graph loading and construction are not timed. |
| **Build settings** | Each language's standard release setting: C and C++ `-O2` (gcc/g++ 15.2), Rust `-C opt-level=3` (rustc 1.98.1), Go 1.27.1 defaults, Java (OpenJDK 25) defaults, C# .NET 10 Release, Node.js 24.21 defaults, CPython 3.14.4. |
| **Machine** | Intel Core Ultra 7 265KF, 31 GB RAM, Ubuntu 26.04 under WSL2. Runs are sequential, one process at a time. |

**A fix made during the experiment.** The first version warmed up for at most 3 runs. On small
graphs that's only microseconds, too little for C#'s JIT to finish optimising, so C# looked 5–10×
worse than it is: a ratio of 1.47 against 21 for C at \|V\| = 250. That run was discarded and
replaced with the 0.5 s warm-up rule above, which gives every JIT time to reach steady state.

## Coverage

The planned sweep was \|V\| from 250 to 50,000, with 3 repeats for \|V\| ≤ 4,000. **It was
stopped part-way at the user's request.** The data covers:

* **one complete repeat** for \|V\| = 250, 500, 1,000, 2,000, 4,000 and 8,000;
* \|V\| = 16,000 at \|E\| = 8\|V\| only, for six languages (not JavaScript or Python);
* \|V\| = 32,000 and 50,000: not run.

At each \|V\|, the graphs are \|E\| = 8\|V\|, 1 %, 10 %, 25 %, 50 %, 75 %, 90 % and 100 % density,
plus the adversarial graph. Graphs with more than 32 million edges were skipped as too large
(file size and memory), which removes 75–100 % density at \|V\| = 8,000.

Because there is only one repeat, the run-to-run noise can't be measured from this data. The main
report's repeat test found ±8 % for (a) and up to ±25 % for (b) on the densest graphs. So ratios
within ±25 % of 1 are treated as **ties** below.

---

## Results: time (a) ÷ time (b)

Above 1, (b) is faster; below 1, (a) is faster. **Bold**: (a) faster by more than 25 %.
*Italic*: within ±25 %, a tie given the noise.

#### \|V\| = 250

| Graph | \|E\| | C | C++ | Rust | Go | Java | C# | JavaScript | Python |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| \|E\| = 8\|V\| | 2,000 | 21.3 | 20.7 | 17.6 | 11.6 | 9.34 | 9.57 | 7.83 | 4.75 |
| 1% | 623 | 21.0 | 20.8 | 21.1 | 13.3 | 10.9 | 12.7 | 8.56 | 5.89 |
| 10% | 6,225 | 21.2 | 19.1 | 16.6 | 6.92 | 9.18 | 7.84 | 6.23 | 3.50 |
| 25% | 15,563 | 20.0 | 19.1 | 14.3 | 7.91 | 9.17 | 6.45 | 5.60 | 2.39 |
| 50% | 31,125 | 15.4 | 15.8 | 12.5 | 6.26 | 7.63 | 5.27 | 4.63 | 1.77 |
| 75% | 46,688 | 7.49 | 7.77 | 5.45 | 3.07 | 3.46 | 2.74 | 2.60 | 1.37 |
| 90% | 56,025 | 3.87 | 3.72 | 2.68 | 1.80 | 1.70 | 1.35 | 1.46 | *1.24* |
| 100% | 62,250 | 2.15 | 2.13 | 1.59 | *1.12* | *0.80* | **0.70** | *0.85* | *1.10* |
| adversarial | 31,125 | **0.24** | **0.24** | **0.30** | **0.27** | **0.15** | **0.24** | **0.07** | **0.18** |

#### \|V\| = 500

| Graph | \|E\| | C | C++ | Rust | Go | Java | C# | JavaScript | Python |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| \|E\| = 8\|V\| | 4,000 | 36.2 | 36.7 | 34.6 | 20.1 | 19.2 | 16.4 | 13.7 | 8.12 |
| 1% | 2,495 | 34.9 | 32.3 | 37.5 | 19.9 | 22.9 | 17.9 | 15.4 | 8.97 |
| 10% | 24,950 | 24.9 | 19.8 | 16.7 | 11.8 | 12.8 | 9.68 | 7.57 | 4.22 |
| 25% | 62,375 | 17.7 | 15.0 | 13.2 | 7.97 | 9.60 | 7.12 | 6.33 | 2.66 |
| 50% | 124,750 | 14.9 | 14.0 | 12.1 | 6.81 | 7.20 | 5.56 | 5.01 | 1.82 |
| 75% | 187,125 | 6.79 | 6.27 | 5.68 | 3.32 | 3.66 | 2.75 | 2.73 | 1.54 |
| 90% | 224,550 | 4.10 | 3.94 | 3.10 | 1.96 | 1.95 | 1.53 | 1.48 | 1.33 |
| 100% | 249,500 | 1.77 | 1.77 | 1.46 | *1.21* | *1.13* | *0.86* | *0.98* | *1.21* |
| adversarial | 124,750 | **0.19** | **0.20** | **0.26** | **0.24** | **0.11** | **0.17** | **0.05** | **0.15** |

#### \|V\| = 1,000

| Graph | \|E\| | C | C++ | Rust | Go | Java | C# | JavaScript | Python |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| \|E\| = 8\|V\| | 8,000 | 16.3 | 15.2 | 19.2 | 15.8 | 13.3 | 16.5 | 10.6 | 15.9 |
| 1% | 9,990 | 17.2 | 14.9 | 23.5 | 15.3 | 17.0 | 16.1 | 10.3 | 15.2 |
| 10% | 99,900 | 13.3 | 13.8 | 11.8 | 9.28 | 9.20 | 7.16 | 8.21 | 5.74 |
| 25% | 249,750 | 14.2 | 14.7 | 12.1 | 7.75 | 8.44 | 6.44 | 6.30 | 3.23 |
| 50% | 499,500 | 13.1 | 12.9 | 11.9 | 6.52 | 7.06 | 5.33 | 5.03 | 2.09 |
| 75% | 749,250 | 6.84 | 6.80 | 5.58 | 3.24 | 3.37 | 2.64 | 2.71 | 1.63 |
| 90% | 899,100 | 3.75 | 3.71 | 2.97 | 1.92 | 1.83 | 1.55 | 1.52 | 1.42 |
| 100% | 999,000 | 1.83 | 1.63 | 1.48 | *1.16* | *0.89* | **0.78** | *0.94* | 1.27 |
| adversarial | 499,500 | **0.17** | **0.17** | **0.22** | **0.23** | **0.11** | **0.15** | **0.07** | **0.14** |

#### \|V\| = 2,000

| Graph | \|E\| | C | C++ | Rust | Go | Java | C# | JavaScript | Python |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| \|E\| = 8\|V\| | 16,000 | 29.3 | 27.9 | 24.5 | 26.2 | 18.4 | 21.3 | 19.1 | 29.1 |
| 1% | 39,980 | 25.5 | 25.1 | 20.9 | 20.8 | 14.2 | 17.2 | 15.8 | 23.3 |
| 10% | 399,800 | 18.4 | 17.5 | 14.5 | 11.3 | 9.79 | 8.60 | 8.57 | 7.02 |
| 25% | 999,500 | 17.9 | 17.0 | 14.2 | 8.74 | 9.19 | 7.17 | 8.37 | 3.83 |
| 50% | 1,999,000 | 11.5 | 12.0 | 11.5 | 6.85 | 7.57 | 6.01 | 5.59 | 2.26 |
| 75% | 2,998,500 | 4.80 | 4.44 | 4.85 | 3.22 | 3.55 | 2.70 | 2.71 | 1.75 |
| 90% | 3,598,200 | 2.42 | 2.58 | 2.60 | 1.93 | 1.86 | 1.45 | 1.89 | 1.47 |
| 100% | 3,998,000 | 1.30 | *1.16* | 1.35 | *1.17* | *0.92* | **0.76** | 1.28 | 1.36 |
| adversarial | 1,999,000 | **0.21** | **0.21** | **0.25** | **0.22** | **0.12** | **0.14** | **0.07** | **0.12** |

#### \|V\| = 4,000

| Graph | \|E\| | C | C++ | Rust | Go | Java | C# | JavaScript | Python |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| \|E\| = 8\|V\| | 32,000 | 57.2 | 57.9 | 46.1 | 44.2 | 34.2 | 37.4 | 34.8 | 56.6 |
| 1% | 159,960 | 38.8 | 36.0 | 31.1 | 29.7 | 21.8 | 23.3 | 21.9 | 33.6 |
| 10% | 1,599,600 | 19.8 | 19.5 | 15.9 | 12.5 | 11.2 | 9.62 | 9.68 | 8.38 |
| 25% | 3,999,000 | 13.0 | 12.3 | 13.4 | 9.17 | 8.89 | 7.13 | 7.96 | 4.10 |
| 50% | 7,998,000 | 9.66 | 9.41 | 11.6 | 7.18 | 7.35 | 6.14 | 6.21 | 2.52 |
| 75% | 11,997,000 | 4.03 | 3.99 | 4.71 | 3.19 | 3.22 | 2.66 | 2.82 | 1.79 |
| 90% | 14,396,400 | 2.20 | 2.07 | 2.64 | 1.90 | 1.65 | 1.43 | 1.57 | 1.52 |
| 100% | 15,996,000 | *1.25* | 1.27 | 1.43 | *1.23* | *0.90* | *0.80* | *0.98* | 1.39 |
| adversarial | 7,998,000 | **0.21** | **0.20** | **0.24** | **0.21** | **0.13** | **0.14** | **0.04** | **0.11** |

#### \|V\| = 8,000 (75–100 % density skipped: more than 32 million edges)

| Graph | \|E\| | C | C++ | Rust | Go | Java | C# | JavaScript | Python |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| \|E\| = 8\|V\| | 64,000 | 91.3 | 90.6 | 77.2 | 77.0 | 62.7 | 61.6 | 66.8 | 109 |
| 1% | 639,920 | 56.3 | 54.1 | 40.8 | 38.6 | 30.0 | 30.9 | 31.3 | 45.8 |
| 10% | 6,399,200 | 16.6 | 15.8 | 15.5 | 13.2 | 11.3 | 9.80 | 10.9 | 9.27 |
| 25% | 15,998,000 | 12.3 | 12.6 | 13.6 | 10.5 | 9.77 | 7.65 | 9.83 | 4.34 |
| 50% | 31,996,000 | 9.21 | 10.0 | 12.3 | 7.85 | 7.91 | 6.55 | 6.95 | 2.49 |
| adversarial | 31,996,000 | **0.17** | **0.19** | **0.22** | **0.18** | **0.14** | **0.13** | **0.04** | **0.10** |

#### \|V\| = 16,000 (partial: stopped here)

| Graph | \|E\| | C | C++ | Rust | Go | Java | C# | JavaScript | Python |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| \|E\| = 8\|V\| | 128,000 | 239 | 250 | 207 | 232 | 130 | 188 | — | — |

### Absolute times (ms)

| Language | \|E\| = 8\|V\|, \|V\| = 4,000: (a) / (b) | complete, \|V\| = 4,000: (a) / (b) | 50 %, \|V\| = 8,000: (a) / (b) | \|E\| = 8\|V\|, \|V\| = 16,000: (a) / (b) |
|---|---:|---:|---:|---:|
| C | 18.5 / 0.32 | 16.3 / 13.1 | 234 / 25.4 | 412 / 1.73 |
| C++ | 19.3 / 0.33 | 17.0 / 13.4 | 247 / 24.6 | 421 / 1.69 |
| Rust | 15.0 / 0.33 | 15.3 / 10.7 | 279 / 22.7 | 346 / 1.67 |
| Go | 18.2 / 0.41 | 19.0 / 15.5 | 270 / 34.4 | 487 / 2.10 |
| Java | 13.2 / 0.39 | 12.7 / 14.0 | 232 / 29.3 | 264 / 2.04 |
| C# | 14.6 / 0.39 | 14.9 / 18.5 | 270 / 41.2 | 366 / 1.94 |
| JavaScript | 18.5 / 0.53 | 20.8 / 21.4 | 307 / 44.2 | — |
| Python | 598 / 10.6 | 859 / 620 | 3,224 / 1,295 | — |

---

## Findings

1. **The verdict doesn't depend on the language.** All eight languages show the same shape:
   * (b) wins at every density below complete.
   * The two are roughly tied on complete graphs: ratios from 0.70 to 2.15, mostly 0.8–1.4.
   * (a) wins clearly only on the adversarial graph, by 3× to 25×.

   None of the eight languages shows (a) clearly ahead on random graphs at any density below
   100 %.

2. **The language changes the size of (b)'s lead, not its direction.** At 50 % density:

   | Group | (b) faster by |
   |---|---:|
   | C, C++, Rust | 9–16× |
   | Go, Java, C#, JavaScript | 4.6–8× |
   | Python | 1.8–2.5× |

   This follows from how much each language's per-operation overhead adds to each of (b)'s
   heap operations compared with (a)'s simple array scans. The more overhead per operation,
   the smaller (b)'s lead.

3. **Complete graphs are the only place languages disagree, and only within noise.** On complete
   graphs, C, C++ and Rust keep (b) 1.2–2.2× ahead; Go, Java and JavaScript are close to a tie;
   C# is the only language with (a) ahead at every size where complete graphs ran (0.70–0.86
   across all five sizes). With a single repeat and ±25 % noise, this is a lean towards (a) in C#, not
   an established win.

4. **(b)'s advantage on sparse graphs grows with \|V\| in every language**, as the asymptotics
   predict (\|V\|² against \|V\| log \|V\|). At \|E\| = 8\|V\| in C it goes from 21× at
   \|V\| = 250 to 91× at 8,000 and 239× at 16,000. The trend isn't strictly monotonic: most
   compiled languages show a dip at \|V\| = 1,000 (C: 36× at 500, 16× at 1,000, 29× at 2,000).
   That dip isn't explained by this data.

5. **Python with flat arrays agrees with the other languages.** With `array`-based storage, (b)
   is faster than (a) at every random density, including 90 % (1.24–1.52×) and complete graphs
   (1.10–1.39×). The teammate's notebook found (a) faster at 50–90 % density. A separate scratch
   test of his code traced most of that to Python's object model, not the algorithm:
   * **Lists instead of arrays.** He stores values in Python lists, where each number is a
     separate object rather than a raw value.
   * **Small weights.** Weights from 1 to 100 are pre-built shared integers in CPython, which
     makes the matrix unusually cheap to read.
   * **Shuffled tuples.** His adjacency tuples were created in shuffled order, so each list's
     entries are scattered across memory.

   Those are Python data-layout choices. With the same layout as every other language here,
   Python gives the same answer.

6. **Absolute speed differs far more than the comparison does.** At \|V\| = 4,000 on a complete
   graph, Python takes 859 ms for (a) and 620 ms for (b), against 11–21 ms in the other
   languages: about 30–70× slower. The *ratio* between (a) and (b) is still similar.

## Limitations

* **One repeat.** Medians come from a single run of the sweep, so per-cell noise isn't
  measured. Use the ±25 % band as a guide. Individual cells near 1, including C#'s complete-graph
  results, are not conclusive.
* **Coverage stops at \|V\| = 8,000**, plus one sparse point at 16,000 for six languages. Dense
  graphs above 32 million edges and \|V\| = 32,000 and 50,000 were never run.
* **Random weights only, plus the one adversarial construction.** The conclusions are about
  inputs where few relaxations succeed.
* **Standard release settings only.** Compiler flags matter: the main report's audit found that
  `-O3 -march=native` brings (a) level with (b) on complete graphs in C, and within 2× at about
  50 % density. Each language was
  run only at its standard release setting.
* **Flat typed arrays in every language.** This isolates the language, but idiomatic code in
  some languages (Python lists, C++ `std::priority_queue`, Java `PriorityQueue`) would use
  different data structures and could give different results. That's a comparison of
  implementations, which this experiment deliberately excludes.

## Reproducing

```
cd lab2/across_languages
source env.sh && build_all    # needs gcc/g++, rustc, go, javac, dotnet, node, python3 on PATH
bash run_all.sh               # full planned sweep (|V| up to 50,000, 3 repeats); several hours
python3 analyse_x.py results_x.csv
```

`results_x.csv` contains the 414 measurements used above: one row per language, graph and
repeat, with both times and the equivalence check. `env.sh` expects the toolchains under
`~/sc2001-langs`. Edit the paths there if they're installed elsewhere.
