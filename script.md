| Block | Presenter | Slides | Clock |
|---|---|---|---|
| 1 | **Swan** | 1, 2, 3 — what we built | 0:00 – 1:50 |
| 2 | **Zach** | 4, 5 — what the maths predicts | 1:50 – 4:15 |
| 3 | **Sakthivel** | 6, 7, 8, 9 — what the machine measures | 4:15 – 6:40 |
| 4 | **Zach** | 10, 11 — why more comparisons is still faster | 6:40 – 8:00 |
| 5 | **Swan** | 12, 13 — conclusions and limits | 8:00 – 8:55 |

The principle behind the split: **whoever derived the theory owns slides 4–5 and every theory
question in Q&A.** Zach returns for 10–11 because the resolution is a callback to his own slide 4 —
the levels the hybrid deletes are the cheapest ones. Swan bookends. Swap names freely; keep the
principle.

The one rule: **never switch presenter mid-argument**, and treat the 4:15 handoff as the hinge — it
is where the talk turns from "theory says don't hybridise" to "and yet it's faster".

---

## Block 1 — Swan

### Slide 1 — Title · 0:00 (25 s)

> This is SC2001 Project 1, integrating merge sort with insertion sort. I'm Swan, and I'm here with
> Sakthivel and Zach.
>
> In one sentence, we added a threshold S to merge sort and measured it, and the S that gives you the
> fewest comparisons and the S that gives you the fastest time turn out to be an **order of magnitude
> apart**.

### Slide 2 — Introduction: the idea, and how we measured it · 0:25 (45 s)

> Merge sort's recursion gets wasteful on tiny subarrays. You pay two function calls and a buffer copy
> just to order a couple of numbers, and insertion sort is cheap at that size. So we set a threshold
> **S**, and once a subarray gets down to S elements or fewer, we switch.
>
> Here's how we measured it. Thirteen sizes, from a thousand up to ten million, filled with uniform
> random integers from a seeded generator we wrote into our own source. That way both algorithms see
> byte-identical input on any compiler.
>
> And we checked correctness five ways, including an independent **Java** implementation that
> reproduces our C counts **bit for bit**.

### Slide 3 — (a) The algorithm · 1:10 (40 s)

> This is the whole modification. One extra branch. If the subarray is at most S, we insertion sort it
> and return. Otherwise we split, recurse and merge, exactly like before.
>
> The line to watch is **S equals 1**. Then the base case only fires on a single element, and insertion
> sort does nothing to a single element. So S equals 1 *is* the original merge sort. Every "S equals 1"
> column later is both our baseline and a self-check that our two implementations agree.

**Handoff at 1:50:**

> Zach's going to take what the maths predicts.

---

## Block 2 — Zach

### Slide 4 — Theory: merge sort and insertion sort · 1:50 (60 s)

> There are two costs to derive.
>
> First, merging. The loop stops as soon as one run empties, and the other run's tail gets copied for
> free. So merging two runs of length r costs about **2r minus 2**. That's two comparisons saved on
> every merge, and the saving is **biggest at the bottom** of the tree. Add it up and you get **1.26**,
> and the two lowest levels alone give you 0.83 of that. Hold onto that, because those are exactly the
> levels the hybrid throws away.
>
> Second, insertion sort. Every insert costs its shifts plus one failing test. Except when the key is a
> new minimum, where the scan runs off the left end and that test never happens. One comparison saved
> per step, and they add up to **H sub m**, which is tiny next to the quadratic.

### Slide 5 — Theory: the hybrid's cost, and why S is not the leaf size · 2:50 (85 s)

> Now we put them together. This is the **exact** average-case recurrence, and every theory curve in
> the deck comes from it.
>
> The closed form needs one more thing, **m**, the size of the piece insertion sort actually gets. And
> m is **not** S. Merge sort only ever halves, so starting from a thousand, the only sizes that exist
> are 1000, 500, 250, 125, 63, 32, 16 and 8. There's no subarray of size 12, and no value of S can make
> one. All S does is say stop when it's small enough, so m ends up between S over 2 and S.
>
> You can see it in the table. S of 8 all the way up to 14 give **the same 9,136 comparisons**, because
> they build the identical tree.
>
> Cost never falls as m grows, and m never falls as S grows. We checked **every S from 1 to 511 across
> five sizes**, and the count never decreases. The minimum is an exact tie at **S equals 1, 2 and 3**,
> and S equals 1 is pure merge sort.

**Handoff at 4:15. This is the hinge, so pause before you swap.**

> So if all you care about is comparisons, the best hybrid is the one that never calls insertion sort
> at all. Sakthivel has the measurements.

---

## Block 3 — Sakthivel

### Slide 6 — (c)(i) Comparisons vs n, S = 16 · 4:15 (40 s)

> First we fixed S at 16 and varied n. On a log-log plot everything is straight with slope just above
> one. But log-log can't tell n log n apart from n to the 1.03, so we divide by n instead. That turns
> a time complexity of n log n into a straight line, and it is one.
>
> Two things to notice. The hybrid sits **above** pure merge sort everywhere. And its curve is wavy,
> which is m oscillating as n grows.

### Slide 7 — (c)(ii) Comparisons vs S, n fixed · 4:55 (35 s)

> Now we fix n and sweep S. The comparison curve is a **staircase**, with risers at 16, 31, 62, 123 and
> 245, and the model overlays it to within 0.19 per cent. Every S inside a flat tread builds the
> identical tree, so S of 128 and S of 192 give **exactly** the same 44.22 million.
>
> Now look at the time, from those same runs. Completely different. It falls steeply to about 30, then
> floors, then turns back up past 200.

### Slide 8 — (c)(iii) CPU time vs S — the U · 5:30 (25 s)

> Here's that time curve properly. Every input size traces the same **U**. A steep drop to about 10,
> then a long shallow floor, then a sharp rise past 256. The optima scatter across **80 to 192** with no
> single winner. **The shape is the result, not the star.**

### Slide 9 — (c)(iii) Choosing S · 5:55 (45 s)

> So how do you pick one? We judged each S by its **worst case across all five sizes**. On that test,
> **S of 128 stays within 0.6 per cent of the best time at every size we tried**, and that's our
> recommendation.
>
> And S of 80, 96 and 128 all build the identical tree at ten million, so they differ only by timing
> noise. Anything from 80 to 192 is defensible.
>
> *[cut if slow]* And if comparisons matter to you, S of 64 gives up about 2.6 per cent of the time
> saving, and it more than halves the comparison penalty.

**Handoff at 6:40:**

> Which leaves the obvious question. Zach?

---

## Block 4 — Zach

### Slide 10 — (d) More comparisons, less time · 6:40 (40 s)

> At ten million with S of 128, the hybrid does **66.9 per cent more** key comparisons. So is that a
> bad threshold, or is it forced?
>
> It's forced. We measured both algorithms directly on small arrays. Insertion sort **ties** merge sort
> for m up to 3, and it's consistently worse from 4 onwards. It's never cheaper at any size in terms of comparisons.
>
> And yet on the same data the hybrid takes **14.1 per cent less** CPU time. That's a 1.16 times
> speed-up, and across S from 20 up to 128 the speed-up rises **monotonically** with the comparison
> count.

### Slide 11 — (d) Why: the accounting · 7:20 (40 s)

> So here's where it all goes. Cutting 6.3 merge levels saves us 50.4 million comparisons, but it costs
> **197.6 million** in insertion sort. That's a net loss of 147 million. And going back to slide 4, the
> levels we deleted were merge sort's **cheapest** ones.
>
> So the win isn't in comparisons at all. It's that **comparisons are not equal**. At ten million the
> array is **40 megabytes**, so every merge level streams through DRAM and every comparison drags a
> cache line with it. A leaf of 76 integers is **304 bytes**, and that sits entirely in L1.

**Handoff at 8:00:**

> Swan's going to close.

---

## Block 5 — Swan

### Slide 12 — Conclusions · 8:00 (40 s)

> So to pull it together. The hybrid costs theta of n log n over S, plus n S, which is theta n log n
> for any constant S, and our model matches every measurement to **0.17 per cent**. In S the count is a
> staircase, because S only reaches the algorithm through an integer recursion depth.
>
> Comparisons turn out to be the **wrong objective** for tuning S. The count never decreases in S, and
> it ties at 1, 2 and 3. CPU time is the right objective, and it says **S of about 128**.

### Slide 13 — Limitations · 8:40 (15 s)

> Two limits. The optimal S is a property of the **machine**. On a one-vCPU VM the same code wanted
> about 32, while the comparison counts stayed bit-identical. And all of this assumes uniformly random
> data. Thank you.

---

## Delivery notes

- **Rehearse out loud with a timer.** Silent reading runs ~40 % faster. If you land past 8:45, take
  the **[cut if slow]** passage on slide 9.
- **Slide 5 is the densest in the deck** — recurrence, m ≠ S, the plateau table and the argmin, all in
  85 seconds. Rehearse it twice as often as anything else. If you're behind, the table is the part to
  gesture at rather than read out.
- **Never compress slides 10 and 11** — that pair is the result.
- **Never say "the hybrid is better"** without the tension. It does *more* work by one measure and
  *less* by another.
- **Never call `model.py` a fit.** It is the exact recurrence evaluated. If anyone says "we fitted the
  model to the data", the whole agreement claim becomes circular. B1 is the slide to jump to.
- Bold numbers are the ones to land on; everything else can be paraphrased.

**Two things to fix in the deck itself:**
- Slide 2's **speaker notes** say "xorshift64\*" — the starred variant is a different algorithm (it
  adds a final multiply). The slide body correctly says "xorshift64"; only the note is stale.
- Slide 14 currently reads "The End / bye bye / see you later / have a good one / smile and wave".
  Fine if that's the tone you want for an assessed talk — just make sure it's deliberate.

---

## Q&A — likely questions, with answers

Two minutes is about **three questions**. Routing: **theory → Zach**, **measurement and hardware →
Sakthivel**, **code and method → Swan**. Each answer names the backup slide to jump to.

### Most likely

**1. Why is your optimal S so much higher than the 16 libraries use?** *(Sakthivel)*
> Three reasons. Libraries compare arbitrary types through a comparator, so a string compare or a user
> lambda costs far more than comparing two ints, and the theta n S term bites much sooner. Our merge is
> deliberately the plain one, it merges into a buffer then copies back, so a tuned version that
> ping-pongs between buffers would halve the per-level cost and pull the optimum down. And libraries
> have to be robust across input distributions, not tuned to uniform random data. So the optimal S
> really depends on comparison cost, merge implementation and memory hierarchy, and we fixed all three
> at one setting.

**2. So is 128 "the" answer?** *(Sakthivel → slide 13)*
> For this machine, yes. On a single-vCPU cloud VM the same code put the basin at about 32, so a four
> times shift. What's robust is the shape, a wide forgiving basin an order of magnitude above 16. The
> comparison counts, on the other hand, are hardware-independent. They came out identical on both
> systems.

**3. Why does the hybrid do more comparisons? Isn't that a bad threshold?** *(Zach → slide 10)*
> It's forced, not a bad choice. Insertion sort ties merge sort for m up to 3 and loses from 4 onwards,
> so it's never cheaper at any size. Every extra comparison is just the price of running insertion sort
> at the bottom of the tree. The real question isn't whether the hybrid does more, it's whether the
> comparisons it trades away were worth more than the ones it takes on.

**4. What's the difference between S and m?** *(Zach → B9, slide 23)*
> S is the threshold we set. m is what insertion sort actually receives. Merge sort only halves, so only
> certain sizes can ever exist. There's no subarray of size 12 when n is 1000. All S does is say stop
> when it's small enough, so m lands between S over 2 and S. That's also why the count is a staircase
> rather than a smooth curve.

### Quite likely

**5. Why write your own random number generator instead of `rand()`?** *(Swan)*
> Because rand gives you different sequences on different compilers. We wanted the experiment
> reproducible, and we wanted to check our counts with a second implementation in Java, and both of
> those need the same input array. So the generator had to live in our own source. And once you're
> writing it yourself, xorshift64 is about as simple as it gets. Three XORs and three shifts on one
> 64-bit number.

**6. How do you know the data is random enough?** *(Swan → B2, slide 16)*
> Our theory assumes uniformly random input, and the measured counts match it to 0.012 per cent at ten
> million. That agreement is the check. If the data were even slightly pre-sorted, insertion sort would
> get dramatically cheaper. At 40 elements it drops from about 426 comparisons to 39. You can't match a
> formula that closely on input that breaks its assumption.

**7. How is a "key comparison" defined?** *(Swan → B4, slide 18)*
> Any comparison between two array elements. In merge it's exactly one per iteration of the main loop,
> and the tail loops that copy the remainder do none. In insertion sort we count a of j greater than key
> every time it's evaluated, including the final failing one. Index and bound tests never count. That
> last detail is what produces the minus H sub m term.

**8. Where does 1.26 come from — is it fitted?** *(Zach → B6, slide 20)*
> No, it's a convergent series. A half plus a third plus a fifth plus a ninth and so on, which sums to
> 1.2645. It comes from every merge finishing early and copying the rest of one run for free. And our
> least-squares fit to the measured data recovers it to within 0.3 per cent.

**9. Your time curve looks noisy — how do you know the optimum is real?** *(Sakthivel → slide 8)*
> We don't claim the individual optimum is meaningful, and we say so on the slide. S enters the
> algorithm in exactly one line, so every S inside a plateau runs identical instructions on identical
> data. Any difference there is measurement noise by construction. What's robust is the shape of the
> basin, and that it sits an order of magnitude above 16.

### Possible

**10. How did you find the minimum — calculus?** *(Zach → B10 and B11, slides 24–25)*
> We did it exactly, with integers only. Evaluate the recurrence for each leaf size, then map back to S.
> Both maps are monotone, so the count never decreases in S, and we checked every S from 1 to 511 at
> five sizes with zero violations. Differentiating the smooth closed form agrees. The slope stays
> positive for every m at least 1, so there's no interior minimum.

**11. Why 23.32 merge levels? Levels should be whole numbers.** *(Zach → slide 11)*
> Levels there means total elements passed through a merge, divided by n, which is the quantity that
> actually drives cost. It's only whole when every leaf sits at the same depth. The hybrid stops at
> depth 17 with all its leaves at 17, so that's exactly 17. Pure merge sort recurses down to single
> elements, and because 2 to the 23 is less than 10 million is less than 2 to the 24, the bottom of its
> tree straddles depths 23 and 24.

**12. Is the hybrid asymptotically better?** *(Zach)*
> No, both are theta n log n. The benefit is a constant factor. Fewer passes over DRAM, 19.7 million
> fewer recursive calls, and comparisons paid out of L1 instead of main memory.

**13. What about nearly-sorted or reversed input?** *(Swan → slide 13)*
> All our results assume uniformly random keys. On nearly-sorted input insertion sort goes near-linear,
> so the optimal S would be far larger, and that's the observation Timsort is built on. On
> reverse-sorted input insertion sort hits its worst case and small S would win.
