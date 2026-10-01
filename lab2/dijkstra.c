/* dijkstra.c -- SC2001 Project 2: Dijkstra's algorithm under different
 * graph representations and priority queues.
 *
 * The two implementations the project asks for:
 *   (a) adjacency matrix + array priority queue           "mat_arr"
 *   (b) adjacency lists  + indexed binary min-heap        "list_heap"
 * Extra variants, used only for the report's "Extra information" section:
 *       (a) with a branch-free relaxation loop             "mat_arr_bf"
 *       adjacency lists  + array priority queue           "list_arr"
 *       adjacency matrix + indexed binary min-heap        "mat_heap"
 *
 * Build:  gcc -O2 -o dijkstra dijkstra.c -lm
 * Usage:  ./dijkstra verify                  correctness checks (vs Bellman-Ford), all variants
 *         ./dijkstra vsV    > exp_vsV.csv    vary |V| at three densities          (a), (b)
 *         ./dijkstra vsE    > exp_vsE.csv    vary |E| at fixed |V|                (a), (b)
 *         ./dijkstra sparse > exp_sparse.csv large sparse graphs, |E| = 8|V|      (a), (b)
 *         ./dijkstra adv    > exp_adv.csv    adversarial graph (every edge is a decrease-key)
 *         ./dijkstra sources > exp_sources.csv  (a), (b) from 20 different source vertices
 *         ./dijkstra vsV extra    > extra_vsV.csv     the same graphs, extra variants only
 *         ./dijkstra vsE extra    > extra_vsE.csv
 *         ./dijkstra sparse extra > extra_sparse.csv
 *
 * Counters (hardware-independent, reported next to CPU time):
 *   pq_ops   array PQ : array slots scanned by extract-min
 *            heap PQ  : key comparisons made inside the heap
 *   edge_ops adjacency entries examined during relaxation (matrix cells or list entries)
 *   updates  successful relaxations, i.e. d[v] lowered  (= decrease-key calls)
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>

typedef long long ll;
#define INF  (LLONG_MAX / 4)
#define WMAX 1000000            /* random edge weights are uniform in [1, WMAX] */
#define MATMAX 16384            /* largest |V| for which the V*V matrix is built */

/* ------------------------------------------------------------------ RNG */
static uint64_t rs = 88172645463325252ULL;
static inline uint64_t xr(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }
static inline uint64_t ru(uint64_t n) { return xr() % n; }
static inline double r01(void) { return (xr() >> 11) * (1.0 / 9007199254740992.0); }
static void seed(uint64_t s) { rs = s ? s : 1; for (int i = 0; i < 16; i++) xr(); }

/* ---------------------------------------------------------------- graph */
typedef struct { int u, v, w; } Edge;
typedef struct { int v, w; } Arc;

typedef struct {
    int V; ll E;
    int *M;            /* V*V adjacency matrix, M[u*V+v] = w, 0 = no edge (NULL if not built) */
    ll  *off;          /* adjacency lists: out-arcs of u are adj[off[u] .. off[u+1]) */
    Arc *adj;
    /* workspace, allocated once per graph (outside the timed region) */
    ll *d; char *vis; int *h, *pos;     /* d = distances (also the array PQ); vis = finished flags; h, pos = heap */
} Graph;

typedef struct { uint64_t pq, edge, upd; } Cnt;

/* duplicate-edge filter: a V*V bitmap when that is small, otherwise a hash set */
typedef struct { uint64_t *bits; uint64_t *tab; uint64_t mask; int V; } Seen;
static void seen_init(Seen *s, int V, ll E) {
    memset(s, 0, sizeof *s); s->V = V;
    if (V <= 50000) s->bits = calloc(((size_t)V * V + 63) / 64, 8);
    else {
        uint64_t sz = 1; while (sz < (uint64_t)(2 * E + 16)) sz <<= 1;
        s->tab = calloc(sz, 8); s->mask = sz - 1;
    }
}
/* returns 1 if (u,v) was new and is now marked */
static int seen_add(Seen *s, int u, int v) {
    if (s->bits) {
        size_t i = (size_t)u * s->V + v;
        if (s->bits[i >> 6] >> (i & 63) & 1) return 0;
        s->bits[i >> 6] |= 1ULL << (i & 63); return 1;
    }
    uint64_t key = ((uint64_t)u << 32 | (uint32_t)v) + 1;
    uint64_t i = (key * 0x9E3779B97F4A7C15ULL) & s->mask;
    while (s->tab[i]) { if (s->tab[i] == key) return 0; i = (i + 1) & s->mask; }
    s->tab[i] = key; return 1;
}
static int seen_has(Seen *s, int u, int v) {
    size_t i = (size_t)u * s->V + v;
    return s->bits[i >> 6] >> (i & 63) & 1;
}
static void seen_free(Seen *s) { free(s->bits); free(s->tab); }

/* Random directed graph with exactly E distinct edges, no self-loops, every
 * vertex reachable from 0.  A random spanning tree rooted at 0 is laid down
 * first (V-1 edges); the remaining E-(V-1) edges are drawn uniformly from all
 * other ordered pairs -- by rejection when the graph is at most half full,
 * by selection sampling (Knuth's Algorithm S) when it is denser. */
static Edge *gen_random(int V, ll *pE) {
    ll maxE = (ll)V * (V - 1), E = *pE;
    if (E < V - 1) E = V - 1;
    if (E > maxE) E = maxE;
    *pE = E;
    Edge *ed = malloc(sizeof(Edge) * (E > 0 ? E : 1));
    int *perm = malloc(sizeof(int) * V);
    for (int i = 0; i < V; i++) perm[i] = i;
    for (int i = V - 1; i >= 2; i--) { int j = 1 + (int)ru(i); int t = perm[i]; perm[i] = perm[j]; perm[j] = t; }
    Seen s; seen_init(&s, V, E);
    ll k = 0;
    for (int c = 1; c < V; c++) {
        int p = perm[ru(c)], q = perm[c];
        seen_add(&s, p, q);
        ed[k++] = (Edge){p, q, 1 + (int)ru(WMAX)};
    }
    ll need = E - (V - 1);
    if (need > 0 && E <= maxE / 2) {
        while (k < E) {
            int u = (int)ru(V), v = (int)ru(V);
            if (u != v && seen_add(&s, u, v)) ed[k++] = (Edge){u, v, 1 + (int)ru(WMAX)};
        }
    } else if (need > 0) {
        ll remaining = maxE - (V - 1);
        for (int u = 0; u < V && need > 0; u++)
            for (int v = 0; v < V && need > 0; v++) {
                if (u == v || seen_has(&s, u, v)) continue;
                if (r01() * remaining < need) { ed[k++] = (Edge){u, v, 1 + (int)ru(WMAX)}; need--; }
                remaining--;
            }
    }
    seen_free(&s); free(perm);
    return ed;
}

/* Adversarial DAG on V vertices, edges i->j for all i<j (E = V(V-1)/2).
 * w(i,i+1) = 1, so dist(i) = i and vertices are extracted in order 0,1,2,...
 * For j > i+1, w(i,j) = A(V-i) + (V-j) - i with A = 2V, so relaxing from i
 * offers j the key A(V-i) + V - j:
 *   - smaller than the key j got from i-1 (the A term drops by A > V), so
 *     EVERY edge is a successful relaxation (a decrease-key);
 *   - smaller than every key already in the heap and, because i's list is
 *     scanned in increasing j, smaller than every key lowered just before it,
 *     so each decrease-key sifts the vertex all the way to the top of the heap.
 * This realises the O(E log V) worst case of (b). */
static Edge *gen_adv(int V, ll *pE) {
    ll E = (ll)V * (V - 1) / 2, k = 0, A = 2LL * V;
    *pE = E;
    Edge *ed = malloc(sizeof(Edge) * (E > 0 ? E : 1));
    for (int i = 0; i < V; i++)
        for (int j = i + 1; j < V; j++)
            ed[k++] = (Edge){i, j, j == i + 1 ? 1 : (int)(A * (V - i) + (V - j) - i)};
    return ed;
}

static Graph *build(int V, Edge *ed, ll E, int wantM) {
    Graph *g = calloc(1, sizeof *g);
    g->V = V; g->E = E;
    g->off = calloc(V + 1, sizeof(ll));
    g->adj = malloc(sizeof(Arc) * (E > 0 ? E : 1));
    for (ll e = 0; e < E; e++) g->off[ed[e].u + 1]++;
    for (int u = 0; u < V; u++) g->off[u + 1] += g->off[u];
    ll *cur = malloc(sizeof(ll) * V);
    memcpy(cur, g->off, sizeof(ll) * V);
    for (ll e = 0; e < E; e++) g->adj[cur[ed[e].u]++] = (Arc){ed[e].v, ed[e].w};
    free(cur);
    if (wantM && V <= MATMAX) {
        /* malloc + memset rather than calloc: calloc'd pages that are never written stay
         * mapped to the kernel's shared zero page, and on this machine reading those was
         * several times slower per cell than reading ordinary memory -- which made very sparse
         * matrices look *slower* than moderately sparse ones.  Touching every page gives
         * a matrix that behaves like one that has actually been filled in. */
        g->M = malloc((size_t)V * V * sizeof(int));
        memset(g->M, 0, (size_t)V * V * sizeof(int));
        for (ll e = 0; e < E; e++) g->M[(size_t)ed[e].u * V + ed[e].v] = ed[e].w;
    }
    g->d = malloc(sizeof(ll) * V);
    g->vis = malloc(V);
    g->h = malloc(sizeof(int) * V);
    g->pos = malloc(sizeof(int) * V);
    return g;
}
static void destroy(Graph *g) {
    free(g->M); free(g->off); free(g->adj); free(g->d); free(g->vis);
    free(g->h); free(g->pos); free(g);
}

/* ------------------------------------------------------- indexed min-heap */
/* h[0..n) holds vertex ids ordered by d[]; pos[v] = index of v in h, -1 once extracted */
static inline void heap_up(int *h, int *pos, const ll *d, int i, uint64_t *cmp) {
    int v = h[i]; ll k = d[v];
    while (i > 0) {
        int p = (i - 1) >> 1;
        ++*cmp; if (d[h[p]] <= k) break;
        h[i] = h[p]; pos[h[i]] = i; i = p;
    }
    h[i] = v; pos[v] = i;
}
static inline void heap_down(int *h, int *pos, const ll *d, int n, int i, uint64_t *cmp) {
    int v = h[i]; ll k = d[v];
    for (;;) {
        int l = 2 * i + 1, c = l;
        if (l >= n) break;
        if (l + 1 < n) { ++*cmp; if (d[h[l + 1]] < d[h[l]]) c = l + 1; }
        ++*cmp; if (d[h[c]] >= k) break;
        h[i] = h[c]; pos[h[i]] = i; i = c;
    }
    h[i] = v; pos[v] = i;
}
/* every vertex enters the heap at the start, source at the root, the rest at infinity */
static inline int heap_init(int *h, int *pos, ll *d, int V, int s) {
    for (int i = 0; i < V; i++) d[i] = INF;
    d[s] = 0; h[0] = s; pos[s] = 0;
    int n = 1;
    for (int i = 0; i < V; i++) if (i != s) { h[n] = i; pos[i] = n; n++; }
    return n;
}
static inline int heap_pop(int *h, int *pos, const ll *d, int *n, uint64_t *cmp) {
    int u = h[0]; pos[u] = -1; --*n;
    if (*n > 0) { h[0] = h[*n]; pos[h[0]] = 0; heap_down(h, pos, d, *n, 0, cmp); }
    return u;
}

/* --------------------------------------------------- array priority queue */
/* The distance array itself is the queue: slot v holds d[v], the key of vertex v, and a
 * flag vis[v] marks vertices already extracted (finished).  Nothing is kept in order:
 *   extract-min : one pass over all |V| slots, skipping finished vertices -> O(|V|)
 *   lower key   : d[v] = new value -> O(1)
 * This is the vertex-indexed array analysed in Cormen et al. (3rd ed., §24.3). */
static inline void arr_init(ll *d, char *vis, int V, int s) {
    for (int i = 0; i < V; i++) { d[i] = INF; vis[i] = 0; }
    d[s] = 0;
}
/* returns the unfinished vertex with the smallest d (marking it finished), or -1 if every
 * unfinished vertex is still at INF */
static inline int arr_pop(const ll *d, char *vis, int V, uint64_t *scan) {
    int u = -1; ll best = INF;
    for (int v = 0; v < V; v++) if (!vis[v] && d[v] < best) { best = d[v]; u = v; }
    *scan += V;
    if (u >= 0) vis[u] = 1;
    return u;
}

/* ------------------------------------------------------------ algorithms */
/* A note on the relaxation test, used in every variant below: a vertex v that is already
 * finished never needs to be excluded explicitly.  Its distance is final and at most d[u]
 * (vertices are finished in increasing distance), so d[u] + w > d[v] for any w >= 1 and the
 * test "d[u] + w < d[v]" already fails.  Leaving out a separate visited check removes work
 * (and an unpredictable branch) from both implementations equally. */

/* (a) adjacency matrix + array PQ: extract-min scans all |V| slots, relaxation scans the
 * whole matrix row */
static void dij_mat_arr(Graph *g, int s, Cnt *c) {
    int V = g->V; ll *d = g->d; char *vis = g->vis; const int *M = g->M;
    uint64_t pq = 0, ed = 0, up = 0;
    arr_init(d, vis, V, s);
    for (int it = 0; it < V; it++) {
        int u = arr_pop(d, vis, V, &pq);
        if (u < 0) break;
        ll du = d[u];
        const int *row = M + (size_t)u * V;
        for (int v = 0; v < V; v++) {
            int w = row[v];
            if (w && du + w < d[v]) { d[v] = du + w; up++; }
        }
        ed += V;
    }
    c->pq = pq; c->edge = ed; c->upd = up;
}

/* (b) adjacency lists + indexed binary min-heap with decrease-key */
static void dij_list_heap(Graph *g, int s, Cnt *c) {
    int V = g->V; ll *d = g->d; int *h = g->h, *pos = g->pos;
    const ll *off = g->off; const Arc *adj = g->adj;
    uint64_t cmp = 0, ed = 0, up = 0;
    int n = heap_init(h, pos, d, V, s);
    while (n > 0) {
        int u = heap_pop(h, pos, d, &n, &cmp);
        ll du = d[u];
        if (du == INF) break;
        for (ll e = off[u]; e < off[u + 1]; e++) {
            int v = adj[e].v; ll nd = du + adj[e].w;
            if (nd < d[v]) { d[v] = nd; up++; heap_up(h, pos, d, pos[v], &cmp); }
        }
        ed += off[u + 1] - off[u];
    }
    c->pq = cmp; c->edge = ed; c->upd = up;
}

/* ------------------------------------------- extra variants (not in the brief) */
/* extra: adjacency matrix + heap */
static void dij_mat_heap(Graph *g, int s, Cnt *c) {
    int V = g->V; ll *d = g->d; int *h = g->h, *pos = g->pos; const int *M = g->M;
    uint64_t cmp = 0, ed = 0, up = 0;
    int n = heap_init(h, pos, d, V, s);
    while (n > 0) {
        int u = heap_pop(h, pos, d, &n, &cmp);
        ll du = d[u];
        if (du == INF) break;
        const int *row = M + (size_t)u * V;
        for (int v = 0; v < V; v++) {
            int w = row[v];
            if (w && du + w < d[v]) { d[v] = du + w; up++; heap_up(h, pos, d, pos[v], &cmp); }
        }
        ed += V;
    }
    c->pq = cmp; c->edge = ed; c->upd = up;
}

/* extra: adjacency lists + array PQ */
static void dij_list_arr(Graph *g, int s, Cnt *c) {
    int V = g->V; ll *d = g->d; char *vis = g->vis;
    const ll *off = g->off; const Arc *adj = g->adj;
    uint64_t pq = 0, ed = 0, up = 0;
    arr_init(d, vis, V, s);
    for (int it = 0; it < V; it++) {
        int u = arr_pop(d, vis, V, &pq);
        if (u < 0) break;
        ll du = d[u];
        for (ll e = off[u]; e < off[u + 1]; e++) {
            int v = adj[e].v; ll nd = du + adj[e].w;
            if (nd < d[v]) { d[v] = nd; up++; }
        }
        ed += off[u + 1] - off[u];
    }
    c->pq = pq; c->edge = ed; c->upd = up;
}

/* extra: (a) with the "is there an edge?" test made branch-free.  Identical work to
 * dij_mat_arr; the only difference is that a missing edge is turned into an infinite
 * candidate distance with a conditional move, instead of being skipped by a branch.  The
 * one branch left ("is the candidate shorter?") is almost never taken, so it is easy for
 * the CPU to predict at every density. */
static void dij_mat_arr_bf(Graph *g, int s, Cnt *c) {
    int V = g->V; ll *d = g->d; char *vis = g->vis; const int *M = g->M;
    uint64_t pq = 0, ed = 0, up = 0;
    arr_init(d, vis, V, s);
    for (int it = 0; it < V; it++) {
        int u = arr_pop(d, vis, V, &pq);
        if (u < 0) break;
        ll du = d[u];
        const int *row = M + (size_t)u * V;
        for (int v = 0; v < V; v++) {
            ll w = row[v], nd = w ? du + w : INF;
            if (nd < d[v]) { d[v] = nd; up++; }
        }
        ed += V;
    }
    c->pq = pq; c->edge = ed; c->upd = up;
}

typedef void (*Algo)(Graph *, int, Cnt *);
enum { MAT_ARR, LIST_HEAP, MAT_ARR_BF, LIST_ARR, MAT_HEAP, NALG };
static const struct { const char *name; Algo f; int needM; } ALG[NALG] = {
    {"mat_arr", dij_mat_arr, 1}, {"list_heap", dij_list_heap, 0},
    {"mat_arr_bf", dij_mat_arr_bf, 1}, {"list_arr", dij_list_arr, 0}, {"mat_heap", dij_mat_heap, 1},
};

/* --------------------------------------------------------------- harness */
static double cpu_now(void) {
    struct timespec t; clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
    return t.tv_sec + 1e-9 * t.tv_nsec;
}
static int cmpd(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b; return (x > y) - (x < y);
}

/* Runs algorithm a once (counts + correctness against ref), then times it:
 * enough repetitions per trial to fill >= 30 ms, 5 trials (3 if one run > 1 s), median. */
static double time_median(Graph *g, int a, int s, double once, int *preps, int *ptrials) {
    int reps = once >= 0.03 ? 1 : (int)ceil(0.03 / (once > 1e-7 ? once : 1e-7));
    int trials = once > 1.0 ? 3 : 5;
    double ts[5];
    for (int t = 0; t < trials; t++) {
        Cnt tmp; double t0 = cpu_now();
        for (int r = 0; r < reps; r++) ALG[a].f(g, s, &tmp);
        ts[t] = (cpu_now() - t0) / reps;
    }
    qsort(ts, trials, sizeof(double), cmpd);
    *preps = reps; *ptrials = trials;
    return ts[trials / 2];
}
static void measure(const char *mode, const char *label, Graph *g, int a, ll *ref) {
    Cnt c;
    double t0 = cpu_now(); ALG[a].f(g, 0, &c); double once = cpu_now() - t0;
    if (ref) {
        for (int i = 0; i < g->V; i++)
            if (g->d[i] != ref[i]) {
                fprintf(stderr, "MISMATCH %s V=%d E=%lld v=%d: %lld vs %lld\n",
                        ALG[a].name, g->V, g->E, i, g->d[i], ref[i]);
                exit(1);
            }
    }
    int reps, trials;
    double t = time_median(g, a, 0, once, &reps, &trials);
    printf("%s,%s,%d,%lld,%s,%.9f,%llu,%llu,%llu,%d,%d\n", mode, label, g->V, g->E, ALG[a].name,
           t, (unsigned long long)c.pq, (unsigned long long)c.edge,
           (unsigned long long)c.upd, reps, trials);
    fflush(stdout);
}

/* run the selected algorithms on one graph; list_heap (always run first) provides the reference */
static void run_point(const char *mode, const char *label, int V, Edge *ed, ll E, unsigned mask) {
    int wantM = (mask & (1u << MAT_ARR | 1u << MAT_HEAP | 1u << MAT_ARR_BF)) != 0;
    Graph *g = build(V, ed, E, wantM);
    free(ed);
    Cnt c; dij_list_heap(g, 0, &c);
    ll *ref = malloc(sizeof(ll) * V); memcpy(ref, g->d, sizeof(ll) * V);
    for (int a = 0; a < NALG; a++) {
        if (!(mask >> a & 1)) continue;
        if (ALG[a].needM && !g->M) continue;
        measure(mode, label, g, a, ref);
    }
    fprintf(stderr, "  done %s %s V=%d E=%lld\n", mode, label, V, E);
    free(ref); destroy(g);
}

static void header(void) {
    puts("mode,label,V,E,variant,time_s,pq_ops,edge_ops,updates,reps,trials");
}

#define BIT(a) (1u << (a))
#define FORMAL (BIT(MAT_ARR) | BIT(LIST_HEAP))               /* the two implementations asked for */
#define EXTRA  (BIT(MAT_ARR_BF) | BIT(LIST_ARR) | BIT(MAT_HEAP))
static int extra_run;      /* set by the "extra" argument: run the extra variants instead */

/* ---------------------------------------------------------------- verify */
static void bellman_ford(int V, Edge *ed, ll E, ll *d) {
    for (int i = 0; i < V; i++) d[i] = INF;
    d[0] = 0;
    for (int it = 0; it < V; it++) {
        int ch = 0;
        for (ll e = 0; e < E; e++)
            if (d[ed[e].u] < INF && d[ed[e].u] + ed[e].w < d[ed[e].v]) { d[ed[e].v] = d[ed[e].u] + ed[e].w; ch = 1; }
        if (!ch) break;
    }
}
static int check_graph(int V, Edge *ed, ll E, int adv) {
    ll *bf = malloc(sizeof(ll) * (V ? V : 1));
    bellman_ford(V, ed, E, bf);
    Graph *g = build(V, ed, E, 1);
    int ok = 1;
    for (int a = 0; a < NALG; a++) {
        Cnt c; ALG[a].f(g, 0, &c);
        for (int i = 0; i < V; i++) if (g->d[i] != bf[i] || g->d[i] >= INF) ok = 0;
        if ((a == MAT_ARR || a == MAT_ARR_BF) && (c.pq != (uint64_t)V * V || c.edge != (uint64_t)V * V)) ok = 0;
        if (a == LIST_HEAP && c.edge != (uint64_t)E) ok = 0;
        if (adv) {
            for (int i = 0; i < V; i++) if (g->d[i] != i) ok = 0;
            if (c.upd != (uint64_t)E) ok = 0;
        }
    }
    destroy(g); free(bf);
    return ok;
}
static void verify(void) {
    int graphs = 0, fails = 0;
    seed(12345);
    for (int V = 1; V <= 80; V++) {
        ll maxE = (ll)V * (V - 1);
        ll Es[5] = {V - 1, (V - 1) + maxE / 8, maxE / 2, maxE / 2 + 1 + maxE / 4, maxE};
        for (int k = 0; k < 5; k++)
            for (int rep = 0; rep < 3; rep++) {
                ll E = Es[k]; Edge *ed = gen_random(V, &E);
                /* generator invariants: exact edge count, no self loops, no duplicates */
                Seen s; seen_init(&s, V, E);
                for (ll e = 0; e < E; e++) if (ed[e].u == ed[e].v || !seen_add(&s, ed[e].u, ed[e].v)) fails++;
                seen_free(&s);
                if (!check_graph(V, ed, E, 0)) { fails++; fprintf(stderr, "FAIL random V=%d E=%lld\n", V, E); }
                free(ed); graphs++;
            }
        ll E; Edge *ed = gen_adv(V, &E);
        if (!check_graph(V, ed, E, 1)) { fails++; fprintf(stderr, "FAIL adv V=%d\n", V); }
        free(ed); graphs++;
    }
    /* one large sparse graph through the hash-set path of the generator */
    { ll E = 8LL * 60000; seed(7); Edge *ed = gen_random(60000, &E);
      Seen s; seen_init(&s, 60000, E); int dup = 0;
      for (ll e = 0; e < E; e++) if (ed[e].u == ed[e].v || !seen_add(&s, ed[e].u, ed[e].v)) dup++;
      seen_free(&s);
      Graph *g = build(60000, ed, E, 0); Cnt c; ll *r = malloc(sizeof(ll) * 60000);
      dij_list_heap(g, 0, &c); memcpy(r, g->d, sizeof(ll) * 60000);
      for (int i = 0; i < 60000; i++) if (r[i] >= INF) dup++;
      dij_list_arr(g, 0, &c);  for (int i = 0; i < 60000; i++) if (g->d[i] != r[i]) dup++;
      if (dup) { fails++; fprintf(stderr, "FAIL large sparse\n"); }
      free(r); free(ed); destroy(g); graphs++; }
    printf("verify: %d graphs, %d failures -> %s\n", graphs, fails, fails ? "FAIL" : "all OK");
}

/* ------------------------------------------------------------ experiments */
/* The formal runs time (a) and (b); "<mode> extra" rebuilds the identical graphs
 * (same seeds) and times only the extra variants. */
static void exp_vsV(void) {
    int Vs[] = {500, 1000, 2000, 3000, 4000, 6000, 8000, 10000};
    const char *lab[] = {"deg8", "half", "full"};
    header();
    for (int dsel = 0; dsel < 3; dsel++)
        for (size_t i = 0; i < sizeof Vs / sizeof *Vs; i++) {
            int V = Vs[i]; ll maxE = (ll)V * (V - 1);
            ll E = dsel == 0 ? 8LL * V : dsel == 1 ? maxE / 2 : maxE;
            seed(1000 + 10 * V + dsel);
            Edge *ed = gen_random(V, &E);
            run_point("vsV", lab[dsel], V, ed, E, extra_run ? EXTRA : FORMAL);
        }
}
static void exp_vsE(void) {
    int Vs[] = {1000, 3000, 10000};
    header();
    for (size_t i = 0; i < sizeof Vs / sizeof *Vs; i++) {
        int V = Vs[i];
        for (int k = 0; k <= 12; k++) {
            ll E = llround((V - 1) * pow((double)V, k / 12.0));
            char lab[32]; snprintf(lab, sizeof lab, "V%d", V);
            seed(2000 + 100 * V + k);
            Edge *ed = gen_random(V, &E);
            run_point("vsE", lab, V, ed, E, extra_run ? EXTRA : FORMAL);
        }
    }
}
static void exp_sparse(void) {
    header();
    for (int p = 10; p <= 22; p++) {
        int V = 1 << p; ll E = 8LL * V;
        unsigned mask;
        if (!extra_run) mask = BIT(LIST_HEAP) | (V <= MATMAX ? BIT(MAT_ARR) : 0);
        else mask = (V <= MATMAX ? BIT(MAT_HEAP) : 0) | (V <= 32768 ? BIT(LIST_ARR) : 0);
        if (!mask) break;
        seed(3000 + p);
        Edge *ed = gen_random(V, &E);
        run_point("sparse", "deg8", V, ed, E, mask);
    }
}
static void exp_adv(void) {
    int Vs[] = {500, 1000, 2000, 3000, 4000, 6000, 8000};
    header();
    for (size_t i = 0; i < sizeof Vs / sizeof *Vs; i++) {
        ll E; Edge *ed = gen_adv(Vs[i], &E);
        run_point("adv", "adv", Vs[i], ed, E, FORMAL);
    }
    /* the same sizes with random weights on the same edge set (complete DAG i<j), as a control */
    for (size_t i = 0; i < sizeof Vs / sizeof *Vs; i++) {
        ll E; Edge *ed = gen_adv(Vs[i], &E);
        seed(4000 + Vs[i]);
        for (ll e = 0; e < E; e++) ed[e].w = 1 + (int)ru(WMAX);
        run_point("adv", "dag_random", Vs[i], ed, E, FORMAL);
    }
}

/* Does the choice of source matter?  Every other experiment runs from vertex 0 (the root of
 * the generator's spanning tree, so it reaches every vertex).  Here (a) and (b) are run from
 * vertex 0 and from 19 randomly chosen vertices of the same graphs. */
static void exp_sources(void) {
    int V = 10000, ks[] = {8, 100, 1000};
    puts("mode,label,V,E,source,variant,time_s,pq_ops,edge_ops,updates,reached");
    for (int i = 0; i < 3; i++) {
        ll E = (ll)ks[i] * V;
        seed(6000 + ks[i]);
        Edge *ed = gen_random(V, &E);
        Graph *g = build(V, ed, E, 1);
        free(ed);
        int src[20]; src[0] = 0;
        for (int j = 1; j < 20; j++) src[j] = (int)ru(V);
        char lab[16]; snprintf(lab, sizeof lab, "deg%d", ks[i]);
        for (int j = 0; j < 20; j++)
            for (int a = MAT_ARR; a <= LIST_HEAP; a++) {
                Cnt c; int reps, trials;
                double t0 = cpu_now(); ALG[a].f(g, src[j], &c); double once = cpu_now() - t0;
                int reached = 0;
                for (int v = 0; v < V; v++) reached += g->d[v] < INF;
                double t = time_median(g, a, src[j], once, &reps, &trials);
                printf("sources,%s,%d,%lld,%d,%s,%.9f,%llu,%llu,%llu,%d\n", lab, V, E, src[j], ALG[a].name,
                       t, (unsigned long long)c.pq, (unsigned long long)c.edge,
                       (unsigned long long)c.upd, reached);
            }
        fprintf(stderr, "  done sources %s\n", lab);
        destroy(g);
    }
}

int main(int argc, char **argv) {
    const char *m = argc > 1 ? argv[1] : "verify";
    extra_run = argc > 2 && !strcmp(argv[2], "extra");
    if (!strcmp(m, "verify")) verify();
    else if (!strcmp(m, "vsV")) exp_vsV();
    else if (!strcmp(m, "vsE")) exp_vsE();
    else if (!strcmp(m, "sparse")) exp_sparse();
    else if (!strcmp(m, "adv")) exp_adv();
    else if (!strcmp(m, "sources")) exp_sources();
    else { fprintf(stderr, "unknown mode %s\n", m); return 1; }
    return 0;
}
