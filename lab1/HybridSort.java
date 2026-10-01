/*
 * SC2001 Algorithm Design and Analysis -- Project 1
 * Hybrid Merge Sort + Insertion Sort (Java version).
 *
 * Uses the same xorshift64 generator as the C version, so the key-comparison
 * counts produced here are identical to those in the report.
 *
 * Run:  java HybridSort.java            (quick demo + small sweep)
 *       java -Xmx2g HybridSort.java 10000000 20
 */
public class HybridSort {

    /** Number of key comparisons performed by the most recent sort. */
    public static long comparisons = 0;

    /* ---------------- Insertion sort on a[left..right] ---------------- */
    public static void insertionSort(int[] a, int left, int right) {
        for (int i = left + 1; i <= right; i++) {
            int key = a[i];
            int j = i - 1;
            while (j >= left) {
                comparisons++;                 // key comparison
                if (a[j] > key) { a[j + 1] = a[j]; j--; }
                else break;
            }
            a[j + 1] = key;
        }
    }

    /* ---------------- Merge a[left..mid] with a[mid+1..right] --------- */
    private static void merge(int[] a, int[] buf, int left, int mid, int right) {
        int i = left, j = mid + 1, k = left;
        while (i <= mid && j <= right) {
            comparisons++;                     // key comparison
            if (a[i] <= a[j]) buf[k++] = a[i++];
            else              buf[k++] = a[j++];
        }
        while (i <= mid)   buf[k++] = a[i++];
        while (j <= right) buf[k++] = a[j++];
        System.arraycopy(buf, left, a, left, right - left + 1);
    }

    /* ---------------- Original merge sort ----------------------------- */
    public static void mergeSort(int[] a, int[] buf, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSort(a, buf, left, mid);
            mergeSort(a, buf, mid + 1, right);
            merge(a, buf, left, mid, right);
        }
    }

    /* ---------------- Hybrid: insertion sort when size <= S ----------- */
    public static void hybridSort(int[] a, int[] buf, int left, int right, int S) {
        if (left >= right) return;
        if (right - left + 1 <= S) { insertionSort(a, left, right); return; }
        int mid = left + (right - left) / 2;
        hybridSort(a, buf, left, mid, S);
        hybridSort(a, buf, mid + 1, right, S);
        merge(a, buf, left, mid, right);
    }

    /* ---------------- Reproducible random data ------------------------ */
    private static long state;
    private static long next() {
        long x = state;
        x ^= x << 13; x ^= x >>> 7; x ^= x << 17;
        return state = x;
    }
    public static int[] generate(int n, long seed) {
        state = (seed == 0) ? 88172645463325252L : seed;
        int[] a = new int[n];
        for (int i = 0; i < n; i++)
            a[i] = (int) Long.remainderUnsigned(next(), 10_000_000L) + 1;
        return a;
    }

    private static boolean sorted(int[] a) {
        for (int i = 1; i < a.length; i++) if (a[i - 1] > a[i]) return false;
        return true;
    }

    public static void main(String[] args) {
        int n = args.length > 0 ? Integer.parseInt(args[0]) : 1_000_000;
        int S = args.length > 1 ? Integer.parseInt(args[1]) : 16;

        int[] buf = new int[n];

        int[] a = generate(n, 1000);
        comparisons = 0;
        long t0 = System.nanoTime();
        hybridSort(a, buf, 0, n - 1, S);
        long t1 = System.nanoTime();
        System.out.printf("hybrid    n=%d S=%d  comparisons=%d  sorted=%b  time=%.3f s%n",
                n, S, comparisons, sorted(a), (t1 - t0) / 1e9);

        a = generate(n, 1000);
        comparisons = 0;
        t0 = System.nanoTime();
        mergeSort(a, buf, 0, n - 1);
        t1 = System.nanoTime();
        System.out.printf("mergesort n=%d      comparisons=%d  sorted=%b  time=%.3f s%n",
                n, comparisons, sorted(a), (t1 - t0) / 1e9);
    }
}
