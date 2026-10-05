# Dijkstra's Algorithm: Presentation Script

SC2001 Project 2 · Target length 8–9 minutes · Deck: `deck/Dijkstra Deck v2.dc.html`

## How to use this script

- Three speaking parts, split at the deck's own section boundaries so each handover lands on a new section.
- **[OPTIONAL]** slides (05, 15, 20) add about 1 minute in total. Include them for a 9-minute run; skip them for 8.
- Times are cumulative targets. Speaking rate assumed: about 140 words per minute.
- Handover lines are written so anyone can deliver them. To rebalance, move a slide across a boundary and keep the handover line with the new owner.

| Part | Slides | Content | Core time | With optional |
|---|---|---|---|---|
| Presenter A | 01–05 | Setup, algorithm, correctness | 2:00 | 2:20 |
| Presenter B | 06–12 | Analysis of (a) and (b) | 3:00 | 3:00 |
| Presenter C | 13–21 | Comparison, conclusions | 3:00 | 3:40 |
| **Total** | | | **8:00** | **9:00** |

**Two-presenter fallback:** A takes 01–09, C takes 10–21.

---

## Presenter A: Setup (0:00–2:00)

### Slide 01 · Title (0:00, ~15 s)

Good morning. We're [names]. Project 2 asks how the choice of graph representation and priority queue changes Dijkstra's running time. We implemented both required versions, derived their cost, measured it, and compared them.

### Slide 02 · Two implementations (0:15, ~25 s)

Version (a) stores the graph as an adjacency matrix and uses the distance array itself as the priority queue, scanning it for the minimum. Version (b) stores adjacency lists and uses an indexed binary min-heap with decrease-key.

The algorithm is the same in both. Only the graph store and the queue change. In theory, (a) is Theta of V squared and (b) is O of V plus E, times log V.

### Slide 03 · The algorithm (0:40, ~40 s)

Here are the two loops side by side.

In (a), each iteration does two full scans: all V slots of the array to find the minimum, then u's whole matrix row, because a matrix can't list only the real neighbours.

In (b), extract-min is a sift-down costing log V, and we only visit u's real out-edges. A successful relaxation calls decrease-key, which is a sift-up, also log V. The heap is indexed, so we can find any vertex's position in constant time.

Neither version checks whether v is already finished. It doesn't need to: a finished vertex already has the smaller distance, so the relaxation test fails on its own.

### Slide 04 · What we count and check (1:20, ~40 s)

Alongside CPU time, every run records three hardware-independent counts: priority-queue operations, edge operations, and successful relaxations. That lets us check each cost formula exactly, not just by curve shape.

For correctness, we tested 1,281 graphs. Every distance was checked against an independent Bellman–Ford. The count identities hold: for (a), both counts equal V squared; for (b), edge operations equal E. And in every timed run, (a) and (b) returned identical distances. All checks pass.

### Slide 05 · Test graphs [OPTIONAL] (2:00, ~20 s)

We used two graph families. Random graphs are a spanning tree from the source plus uniform random edges, with weights up to a million, from a tree to a complete graph. The adversarial graph is a complete DAG with weights chosen so every edge is a successful relaxation. Each timing is the median of five runs, with graph construction outside the timed region.

**Handover:** [B's name] will now go through the analysis of each implementation, starting with (a).

---

## Presenter B: Analysis of (a) and (b) (2:00–5:00)

### Slide 06 · (a) Cost: theory (2:00, ~30 s)

For (a), each of the V iterations scans V array slots and then V row cells. That is exactly 2 V squared inspections on every graph, so best, average and worst case are the same. E only changes how many cells contain an edge.

We measured this directly: both counts equal V squared in all 82 runs, from a tree up to a complete graph with 100 million edges. Memory is also Theta of V squared.

### Slide 07 · (a) Time vs |V| (2:30, ~20 s)

On a log-log plot, V squared is a slope of 2. We measured 2.05 to 2.12. Notice the half-full graphs run 3 to 4 times slower, even though they do exactly the same number of operations. The next slide explains why.

### Slide 08 · (a) Time vs |E| (2:50, ~25 s)

Theory predicts a flat line here, and it is flat up to about 1% density. Then there's a hump that peaks near 50%. At that density, the "does this edge exist" test is a coin flip, so the CPU's branch predictor fails half the time. When we replaced the branch with a conditional move, the hump disappeared with identical operation counts. The hump comes from the CPU, not the algorithm.

### Slide 09 · (b) Cost: theory (3:15, ~30 s)

For (b), the cost has three terms. Every adjacency list is scanned once, so E edge checks. Every vertex is extracted once, at log V each. And each successful relaxation costs one decrease-key, also log V.

Call the number of successful relaxations D. Only the third term carries E log V, and only if D equals E, meaning nearly every relaxation succeeds. So D decides (b)'s real cost.

### Slide 10 · Successful relaxations (3:45, ~30 s)

On random weights, a relaxation succeeds only when it offers v a new running minimum. In a random sequence, the expected number of running minima grows like the natural log, so D per vertex grows like ln of E over V. Our measurements track that curve closely.

On a complete graph with 10,000 vertices, only 0.088% of relaxations succeed, and each decrease-key costs 1.84 comparisons against a heap height of 13. On random weights, the E log V term is almost dormant.

### Slide 11 · (b) Time vs |E| (4:15, ~25 s)

So in practice (b) is linear in E, at about 0.9 to 1.4 nanoseconds per edge. On sparse graphs the V log V extract-min term dominates. Heap comparisons stay at 1.94 to 1.97 times V log V all the way up to 4.2 million vertices. There's no visible log factor on E.

### Slide 12 · (b) Worst case (4:40, ~20 s)

But the worst case is real. On the adversarial graph, 100% of relaxations succeed, and each decrease-key sifts to the root: 11.3 comparisons against a heap height of 13. Total heap comparisons come to 0.87 E log V, so the bound is tight.

**Handover:** So which one should you use? [C's name] will compare them.

---

## Presenter C: Comparison and conclusions (5:00–8:00 / 9:00)

### Slide 13 · (c) Theory's prediction (5:00, ~25 s)

If we set the two worst-case bounds equal, V squared against E log V, the break-even is at a density of 1 over log V. At 10,000 vertices that is 7.5%. Below that, theory says (b) wins; above it, (a) wins. But this assumes every edge is a log V decrease-key, which we just saw doesn't happen on random weights.

### Slide 14 · Speed-up vs density (5:25, ~25 s)

And the measurements confirm that. At 10,000 vertices, (b) is 166 times faster on a tree, 14.8 times faster at 10% density, and still 1.42 times faster on the complete graph. On random graphs, the theoretical crossover never appears.

### Slide 15 · Operation counts [OPTIONAL] (5:50, ~20 s)

The same holds without the hardware. Even on the complete graph, (b) does about half the operations of (a), because it touches each edge once while (a) touches each cell twice. Only the adversarial graph puts (b) above (a), by about three times.

### Slide 16 · Memory (5:50 or 6:10, ~25 s)

Memory matters even more. On sparse graphs with E equal to 8V, the matrix for (a) needs 4 V squared bytes: 400 megabytes at 10,000 vertices, 40 gigabytes at 100,000, which doesn't fit on our machine. The lists for (b) need 7 megabytes at that size. Memory rules (a) out before time does.

### Slide 17 · Adversarial input (6:15, ~25 s)

Where (a) does win is adversarial input. Same edges, only the weights change: (b) goes from up to 3.7 times faster to 4.6 to 6.1 times slower. (a) barely moves, because its work is always 2 V squared. Its cost depends only on V; (b)'s depends on the weights.

### Slide 18 · Which to use (6:40, ~25 s)

To summarise part (c): (b) is the better choice for sparse graphs, dense random graphs, and is tied or better on complete graphs. (a) is better when many relaxations succeed, or when the running time must be predictable from V alone.

### Slide 19 · Conclusions (7:05, ~40 s)

To conclude. (a) is Theta of V squared, independent of E, confirmed exactly in every run. (b)'s worst case is real, but on random weights it almost never happens, so (b) runs in linear time in E. That makes (b) the better general-purpose choice: faster at every random density, and the only one that fits in memory beyond about 100,000 vertices. (a) wins when many relaxations succeed, and when time must be fixed by V. The 7.5% break-even comes from (b)'s worst case; on random graphs there is no crossover.

### Slide 20 · Limitations [OPTIONAL] (7:45, ~20 s)

A few limitations. Random weights are a modelling choice; real-world weights could fall anywhere between random and adversarial. Each timing comes from one graph instance. Cache and branch effects are machine-specific, though the operation counts are not. And we ran from one source vertex, though nineteen other sources gave the same results.

### Slide 21 · Questions (7:45 or 8:05)

Thank you. We're happy to take questions.

---

## Q&A: who answers what

| Question area | Owner | Backup slide |
|---|---|---|
| Why positive weights / negative edges | A | B1 |
| Correctness testing, Bellman–Ford | A | 04 |
| Large sparse scaling, cache effects | B | B2 |
| Branch misprediction, branch-free (a) | B | B4 |
| Sparse (a) vs (b) beyond 10⁴ | C | B3 |
| Does the source vertex matter | C | B5 |

## Cutting time on the day

1. Skip 05, 15, 20 (saves ~1:00).
2. Shorten 03 to the two bolded differences: full row scan vs real edges only.
3. On 19, read only points 1, 4 and 5.
