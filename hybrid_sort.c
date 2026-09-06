/*
 * SC2001 Algorithm Design and Analysis
 * Project 1: Integration of Merge Sort & Insertion Sort
 *
 * Implements:
 *   - Insertion Sort (with key-comparison counter)
 *   - Original (pure) Merge Sort (with key-comparison counter)
 *   - Hybrid Merge Sort: recurse until subarray size <= S, then Insertion Sort
 *
 * A "key comparison" is any comparison between two array elements (keys).
 * Index/bound tests such as (j >= left) or (i <= mid) are NOT counted.
 *
 * Build:  gcc -O2 -o hybrid_sort hybrid_sort.c
 * Usage:  ./hybrid_sort <experiment>
 *         experiment in {a, b, c, d, e, leafstats, verify}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------------------------------------------------ */
/* Global key-comparison counter                                       */
/* ------------------------------------------------------------------ */
static long long comparisons = 0;

/* ------------------------------------------------------------------ */
/* Reproducible pseudo-random number generator (xorshift64*)           */
/* ------------------------------------------------------------------ */
static unsigned long long rng_state = 88172645463325252ULL;

static void rng_seed(unsigned long long s) {
    rng_state = s ? s : 88172645463325252ULL;
}

static unsigned long long rng_next(void) {
    unsigned long long x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;
    return x;
}

/* random dataset of integers in [1 .. MAXVAL] */
#define MAXVAL 10000000

static void generate(int *a, long long n, unsigned long long seed) {
    long long i;
    rng_seed(seed);
    for (i = 0; i < n; i++)
        a[i] = (int)(rng_next() % (unsigned long long)MAXVAL) + 1;
}

/* ------------------------------------------------------------------ */
/* Insertion Sort on a[left..right] (inclusive)                        */
/* ------------------------------------------------------------------ */
static void insertion_sort(int *a, long long left, long long right) {
    long long i, j;
    for (i = left + 1; i <= right; i++) {
        int key = a[i];
        j = i - 1;
        /* bound test j >= left is not a key comparison;
           a[j] > key is a key comparison and is counted every time it runs */
        while (j >= left) {
            comparisons++;
            if (a[j] > key) {
                a[j + 1] = a[j];
                j--;
            } else {
                break;
            }
        }
        a[j + 1] = key;
    }
}

/* ------------------------------------------------------------------ */
/* Merge a[left..mid] and a[mid+1..right] using scratch buffer         */
/* ------------------------------------------------------------------ */
static void merge(int *a, int *buf, long long left, long long mid, long long right) {
    long long i = left, j = mid + 1, k = left;

    while (i <= mid && j <= right) {
        comparisons++;                 /* one key comparison per iteration */
        if (a[i] <= a[j]) buf[k++] = a[i++];
        else              buf[k++] = a[j++];
    }
    while (i <= mid)   buf[k++] = a[i++];   /* copying: no key comparisons */
    while (j <= right) buf[k++] = a[j++];

    memcpy(a + left, buf + left, (size_t)(right - left + 1) * sizeof(int));
}

/* ------------------------------------------------------------------ */
/* Original Merge Sort (as in lecture)                                 */
/* ------------------------------------------------------------------ */
static void merge_sort(int *a, int *buf, long long left, long long right) {
    if (left < right) {
        long long mid = left + (right - left) / 2;
        merge_sort(a, buf, left, mid);
        merge_sort(a, buf, mid + 1, right);
        merge(a, buf, left, mid, right);
    }
}

/* ------------------------------------------------------------------ */
/* Hybrid Merge Sort: switch to Insertion Sort when size <= S          */
/* ------------------------------------------------------------------ */
static void hybrid_sort(int *a, int *buf, long long left, long long right, long long S) {
    if (left >= right) return;
    if (right - left + 1 <= S) {
        insertion_sort(a, left, right);
        return;
    }
    long long mid = left + (right - left) / 2;
    hybrid_sort(a, buf, left, mid, S);
    hybrid_sort(a, buf, mid + 1, right, S);
    merge(a, buf, left, mid, right);
}

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */
static int is_sorted(const int *a, long long n) {
    long long i;
    for (i = 1; i < n; i++)
        if (a[i - 1] > a[i]) return 0;
    return 1;
}

static double now_cpu(void) {
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

/* Run one trial. mode 0 = pure merge sort, mode 1 = hybrid with S.
   Returns CPU seconds; *cmp receives key comparisons.                 */
static double run_once(int *a, int *buf, long long n, int mode, long long S,
                       long long *cmp, unsigned long long seed) {
    double t0, t1;
    generate(a, n, seed);
    comparisons = 0;
    t0 = now_cpu();
    if (mode == 0) merge_sort(a, buf, 0, n - 1);
    else           hybrid_sort(a, buf, 0, n - 1, S);
    t1 = now_cpu();
    if (!is_sorted(a, n)) { fprintf(stderr, "ERROR: array not sorted!\n"); exit(1); }
    *cmp = comparisons;
    return t1 - t0;
}

/* number of repetitions so small inputs are timed meaningfully */
static int reps_for(long long n) {
    if (n <= 10000)    return 50;
    if (n <= 100000)   return 20;
    if (n <= 1000000)  return 5;
    if (n <= 5000000)  return 3;
    return 3;
}

/* ------------------------------------------------------------------ */
/* Experiments                                                         */
/* ------------------------------------------------------------------ */

static const long long SIZES[] = {
    1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000,
    1000000, 2000000, 5000000, 10000000
};
static const int NSIZES = sizeof(SIZES) / sizeof(SIZES[0]);

/* (c)(i)  S fixed, vary n. Also record pure merge sort for reference. */
static void exp_a(int *a, int *buf) {
    const long long S_FIXED = 16;
    // const long long S_FIXED = 32;
    int i, r, R;
    printf("n,S,hybrid_comparisons,hybrid_time,merge_comparisons,merge_time\n");
    for (i = 0; i < NSIZES; i++) {
        long long n = SIZES[i], c1 = 0, c2 = 0;
        double th = 0, tm = 0;
        R = reps_for(n);
        for (r = 0; r < R; r++) th += run_once(a, buf, n, 1, S_FIXED, &c1, 1000 + r);
        for (r = 0; r < R; r++) tm += run_once(a, buf, n, 0, 0,       &c2, 1000 + r);
        printf("%lld,%lld,%lld,%.6f,%lld,%.6f\n", n, S_FIXED, c1, th / R, c2, tm / R);
        fflush(stdout);
    }
}

/* (c)(ii) n fixed, vary S */
static void exp_b(int *a, int *buf) {
    /* Every integer S from 1 to 256 is swept, so that the staircase risers land
       exactly. That is expensive at large n, which is why this experiment uses a
       handful of sizes and exp_c covers more sizes at sampled S values instead. */
    const long long NS[] = {1000, 10000, 100000, 1000000, 10000000};
    const int NNS = sizeof(NS) / sizeof(NS[0]);
    int ni, r, R;
    long long S;
    printf("n,S,comparisons,time\n");
    for (ni = 0; ni < NNS; ni++) {
        long long n = NS[ni];
        R = reps_for(n);
        for (S = 1; S <= 256; S++) {
            long long c = 0; double t = 0;
            for (r = 0; r < R; r++) t += run_once(a, buf, n, 1, S, &c, 2000 + r);
            printf("%lld,%lld,%lld,%.6f\n", n, S, c, t / R);
            fflush(stdout);
        }
    }
}

/* (c)(iii) optimal S across many input sizes */
static void exp_c(int *a, int *buf) {
    const long long NS[] = {1000, 10000, 100000, 1000000, 10000000};
    /* Sweep must extend well past the optimum, or the "best S" reported is just
       the edge of the search range. On fast modern CPUs the CPU-time basin sits
       far higher than the classic 16-64, so the range runs to 512.            */
    const long long SVALS[] = {1,2,3,4,5,6,7,8,9,10,12,14,16,20,24,28,32,40,48,
                               64,80,96,112,128,160,192,224,256,320,384,448,512};
    const int NSV = sizeof(SVALS) / sizeof(SVALS[0]);
    int ni, si, r, R;
    printf("n,S,comparisons,time\n");
    for (ni = 0; ni < 5; ni++) {
        long long n = NS[ni];
        R = reps_for(n);
        for (si = 0; si < NSV; si++) {
            long long S = SVALS[si], c = 0; double t = 0;
            for (r = 0; r < R; r++) t += run_once(a, buf, n, 1, S, &c, 3000 + r);
            printf("%lld,%lld,%lld,%.6f\n", n, S, c, t / R);
            fflush(stdout);
        }
    }
}

/* (d) head-to-head on 10 million integers */
static void exp_d(int *a, int *buf, long long S_OPT) {
    const long long n = 10000000;
    int r;
    printf("algorithm,n,S,trial,comparisons,cpu_time\n");
    for (r = 0; r < 5; r++) {
        long long c;
        double t = run_once(a, buf, n, 1, S_OPT, &c, 4000 + r);
        printf("hybrid,%lld,%lld,%d,%lld,%.6f\n", n, S_OPT, r + 1, c, t);
        fflush(stdout);
    }
    for (r = 0; r < 5; r++) {
        long long c;
        double t = run_once(a, buf, n, 0, 0, &c, 4000 + r);
        printf("mergesort,%lld,0,%d,%lld,%.6f\n", n, r + 1, c, t);
        fflush(stdout);
    }
}

/* refined timing sweep at n = 10 million, more repetitions */
static void exp_e(int *a, int *buf) {
    const long long SVALS[] = {1,8,12,16,20,24,28,32,40,48,64,80,96,128,160,192,256,384,512};
    const int NSV = sizeof(SVALS) / sizeof(SVALS[0]);
    int si, r;
    printf("n,S,trial,comparisons,time\n");
    for (si = 0; si < NSV; si++) {
        for (r = 0; r < 5; r++) {
            long long c; double t;
            t = run_once(a, buf, 10000000, 1, SVALS[si], &c, 5000 + r);
            printf("10000000,%lld,%d,%lld,%.6f\n", SVALS[si], r + 1, c, t);
            fflush(stdout);
        }
    }
}

/* Base-case calibration (data behind fig5 / leafstats.csv).

   For each small subarray size m, measure the AVERAGE number of key comparisons
   that insertion sort and merge sort each need on random arrays of that size.
   This is what tells us where the two algorithms cross over, and it is what the
   formula  C_ins(m) = m(m+3)/4 - H_m  is checked against.

   Both algorithms are run on the SAME array each trial (a paired comparison),
   so the difference between them is not polluted by sampling noise.           */
static void leafstats(int *a, int *buf) {
    const int TRIALS = 50000;
    const int MAXM   = 40;
    int orig[64];                      /* MAXM is well under 64 */
    int m, t, i;

    printf("m,insertion_avg,mergesort_avg,quarter_m2\n");
    rng_seed(20240501ULL);             /* one fixed seed for the whole sweep */

    for (m = 1; m <= MAXM; m++) {
        double sum_ins = 0.0, sum_ms = 0.0;
        for (t = 0; t < TRIALS; t++) {
            /* draw one random array, keep a pristine copy in orig[] */
            for (i = 0; i < m; i++)
                orig[i] = (int)(rng_next() % (unsigned long long)MAXVAL) + 1;

            memcpy(a, orig, (size_t)m * sizeof(int));
            comparisons = 0;
            insertion_sort(a, 0, m - 1);
            sum_ins += (double)comparisons;

            memcpy(a, orig, (size_t)m * sizeof(int));
            comparisons = 0;
            merge_sort(a, buf, 0, m - 1);
            sum_ms += (double)comparisons;
        }
        /* quarter_m2 = m^2/4 is printed as a crude reference for insertion sort */
        printf("%d,%.4f,%.4f,%.4f\n",
               m, sum_ins / TRIALS, sum_ms / TRIALS, m * m / 4.0);
        fflush(stdout);
    }
}

/* correctness checks on small arrays */
static void verify(int *a, int *buf) {
    long long n, S, c;
    for (n = 1; n <= 300; n++) {
        for (S = 1; S <= 20; S += 3) {
            run_once(a, buf, n, 1, S, &c, n * 31 + S);
        }
        run_once(a, buf, n, 0, 0, &c, n);
    }
    printf("verify: all hybrid (S=1..20) and merge sort runs for n=1..300 produced sorted output\n");
}

int main(int argc, char **argv) {
    long long cap = 10000000;
    int *a = malloc((size_t)cap * sizeof(int));
    int *buf = malloc((size_t)cap * sizeof(int));
    if (!a || !buf) { fprintf(stderr, "allocation failed\n"); return 1; }

    if (argc < 2) {
        fprintf(stderr, "usage: %s {a|b|c|d|e|leafstats|verify}\n", argv[0]);
        return 1;
    }

    switch (argv[1][0]) {
        case 'a': exp_a(a, buf); break;
        case 'b': exp_b(a, buf); break;
        case 'c': exp_c(a, buf); break;
        case 'd': exp_d(a, buf, argc > 2 ? atoll(argv[2]) : 16); break;
        case 'e': exp_e(a, buf); break;
        case 'l': leafstats(a, buf); break;
        case 'v': verify(a, buf); break;
        default:  fprintf(stderr, "unknown experiment\n"); return 1;
    }
    free(a); free(buf);
    return 0;
}
