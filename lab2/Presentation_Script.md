# Dijkstra's Algorithm: Presentation Script

SC2001 Project 2 · Target length 8–9 minutes · Deck: `deck/Dijkstra Deck v2.dc.html`

## How to use this script

- Three speaking parts, split at the deck's own section boundaries so each handover lands on a new section.
- **[OPTIONAL]** slides (05, 15, 20) take about 70 s in total. Skip them all for an 8-minute run, or include two of them for a 9-minute run.
- Times are cumulative targets at about 140 words per minute, and assume the optional slides are skipped. Each optional slide you include adds its own length (20–25 s) to every later time. The core run is about 8:05, or 9:15 with all three optional slides.
- Handover lines are written so anyone can deliver them. To rebalance, move a slide across a boundary and keep the handover line with the new owner.
- Every term is explained the first time it is spoken on the core path, so the script still works when the optional slides are skipped.
- The script is written to be said aloud: short sentences, and numbers rounded wherever the exact figure doesn't matter. The exact figures are on the slides. In speech, (a) and (b) are "version A" and "version B", and (c) is "part C".

| Part | Slides | Content | Core time | With optional |
|---|---|---|---|---|
| Presenter A | 01–05 | Setup, algorithm, correctness | 2:05 | 2:30 |
| Presenter B | 06–12 | Analysis of (a) and (b) | 3:25 | 3:25 |
| Presenter C | 13–21 | Comparison, conclusions | 2:35 | 3:20 |
| **Total** | | | **8:05** | **9:15** |

**Two-presenter fallback:** A takes 01–09, C takes 10–21.

---

## Presenter A: Setup (0:00–2:05)

### Slide 01 · Title (0:00, ~15 s)

Good morning. We're [names]. Our project asks how much Dijkstra's speed depends on how you store the graph, and which priority queue you use. We built both versions, worked out their costs, timed them, and compared them.

### Slide 02 · Two implementations (0:15, ~35 s)

V is the number of vertices, and E is the number of edges.

Version A stores the graph as an adjacency matrix. That's a V-by-V table of edge weights. Its queue is just the distance array, which it scans for the smallest value.

Version B gives each vertex a list of its own edges. Its queue is a binary min-heap, which keeps the smallest distance on top.

In theory, A always costs about V squared. B costs at most V plus E, times log V.

### Slide 03 · The algorithm (0:50, ~50 s)

Each round, we take the closest vertex we haven't finished, and call it u. Then we relax its edges. For each neighbour, is it shorter to go through u? If so, the relaxation succeeds, and we lower that neighbour's distance.

Version A finds u by scanning all V slots of the array, then reads u's whole matrix row, empty cells and all.

Version B takes u off the heap, which is called extract-min, in log V. Then it only checks u's real edges. Each successful relaxation is a decrease-key: we lower the distance, and move the vertex up the heap, also in log V. A side array tracks each vertex's place in the heap, so we never search for it.

### Slide 04 · What we count and check (1:40, ~25 s)

Every run also counts queue operations, edge operations, and successful relaxations. None of these depend on the machine, so we can check each cost formula exactly. And they held. A always did V squared of each. B always did exactly E edge operations.

We also checked both against Bellman–Ford, a different shortest-path algorithm. On all 1,281 test graphs, the distances matched.

### Slide 05 · Test graphs [OPTIONAL] (2:05, ~25 s)

Each random graph starts as a tree grown from the source, so every vertex is reachable. Then we add random edges. The adversarial graph is a directed graph with no cycles, with weights chosen so every relaxation succeeds. Each time is the median of five runs.

**Handover:** Now [B's name] will go through each version in detail, starting with version A.

---

## Presenter B: Analysis of (a) and (b) (2:05–5:30)

### Slide 06 · (a) Cost: theory (2:05, ~30 s)

Let's start with version A. Each of its V rounds reads V array slots and V matrix cells. That's exactly two V squared reads, on every graph. E only changes how many cells hold an edge.

That held in all 82 runs, from a tree, the sparsest graph that's still connected, to a complete graph, where every pair of vertices is joined. Memory grows with V squared too.

### Slide 07 · (a) Time vs |V| (2:35, ~20 s)

On a log-log plot, V squared is a straight line with slope two, and we measured just over two. So far, so good. But graphs where half the possible edges exist run three to four times slower, with exactly the same operations. So what's going on?

### Slide 08 · (a) Time vs |E| (2:55, ~40 s)

Here, V is fixed and E grows. We show it as density, the fraction of possible edges that exist. Time is flat up to about one percent, then rises to a hump around fifty.

The cause is branch prediction. The CPU guesses how each if-statement will go before it knows. At fifty percent, "is there an edge?" is a coin flip, so half the guesses are wrong, and each one costs time. When we replaced that if with a conditional move, which picks a value without branching, the hump went flat. The operation counts didn't change at all.

### Slide 09 · (b) Cost: theory (3:35, ~30 s)

Now version B. Its cost has three parts: E edge checks, V extract-mins at log V each, and a log V decrease-key for every successful relaxation.

We'll call the number of successful relaxations D. It's how many times any distance actually gets lowered. Only that third part can reach E log V, and only if D is close to E. So it's D that decides B's cost.

### Slide 10 · Successful relaxations (4:05, ~40 s)

So how big is D? With random weights, a relaxation only succeeds if it beats every earlier offer to that vertex, like setting a new record. In a random sequence, records get rarer and rarer. Their count grows like the natural log of the length.

Each vertex gets about E over V offers, so its distance drops about log of E over V times. On a complete graph with ten thousand vertices, fewer than one relaxation in a thousand succeeds. And each decrease-key takes about two comparisons, not the thirteen of a full climb up the heap.

### Slide 11 · (b) Time vs |E| (4:45, ~20 s)

So in practice, B's time just grows in step with E, at roughly one nanosecond per edge. On sparse graphs, the extract-mins take over instead. And heap comparisons stay at about two V log V, right up to four million vertices, just as a binary heap should.

### Slide 12 · (b) Worst case (5:05, ~25 s)

But the worst case does exist. We built an adversarial graph where every relaxation succeeds, and every decrease-key climbs nearly to the top: about eleven comparisons out of thirteen. Total heap work reaches nearly ninety percent of E log V. So the worst case really can happen.

**Handover:** So which one should you actually use? [C's name] will compare them.

---

## Presenter C: Comparison and conclusions (5:30–8:05)

### Slide 13 · (c) Theory's prediction (5:30, ~25 s)

If you set the two worst-case costs equal, V squared against E log V, they break even at a density of one over log V. At ten thousand vertices, that's seven and a half percent. Below that, theory says B wins, and above it, A. But that assumes every edge triggers a full decrease-key, and random weights don't do that.

### Slide 14 · Speed-up vs density (5:55, ~20 s)

The measurements bear that out. At ten thousand vertices, B is over a hundred and sixty times faster on a tree, about fifteen times faster at ten percent density, and still about forty percent faster on a complete graph. On random graphs, the crossover never shows up.

### Slide 15 · Operation counts [OPTIONAL] (6:15, ~20 s)

Counting operations tells the same story. Even on a complete graph, B does about half of A's work, because B reads each edge once, and A reads each matrix cell twice. Only the adversarial graph pushes B above A, by about three times.

### Slide 16 · Memory (6:15, ~25 s)

Memory matters even more. With eight edges per vertex, A's matrix still needs four bytes for every cell. That's four hundred megabytes at ten thousand vertices, and forty gigabytes at a hundred thousand, which won't fit on our machine. B's lists need just seven megabytes. So memory rules A out before time does.

### Slide 17 · Adversarial input (6:40, ~25 s)

A does win on the adversarial graph, though. On the same edges, B goes from up to almost four times faster with random weights, to five or six times slower with adversarial ones. A barely moves. Its work is always two V squared, so it only depends on V. B's depends on the weights.

### Slide 18 · Which to use (7:05, ~20 s)

So, for part C: B is the better choice for sparse graphs and dense random graphs, and on complete graphs it's tied or better. A is better when lots of relaxations succeed, or when you need the running time to depend on V alone.

### Slide 19 · Conclusions (7:25, ~40 s)

To wrap up. A's cost grows with V squared, whatever E is, and every run confirmed it. B's worst case is real, but random weights almost never trigger it, so its time just grows with E.

So B is the better general choice. It's faster at every random density, and it's the only one that fits in memory past about a hundred thousand vertices. A wins when lots of relaxations succeed.

And that seven and a half percent break-even comes from B's worst case. On random graphs, there's no crossover.

### Slide 20 · Limitations [OPTIONAL] (8:05, ~25 s)

A few limitations. Random weights are one modelling choice, and real weights could sit anywhere between random and adversarial. Each timing comes from a single graph. The cache and branch effects are specific to our machine, but the operation counts aren't. And nineteen other source vertices gave the same results as vertex zero.

### Slide 21 · Questions (8:05)

Thank you. We're happy to take any questions.

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

1. Skip 05, 15, 20 (saves ~1:10).
2. Shorten 03 to the key contrast: version A reads u's whole matrix row, version B reads only u's real edges.
3. On 19, read only points 1, 4 and 5.
