// Cross-language benchmark, Java version (same specification as x_c.c).
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;

public class XBench {
    static final long INF = 1L << 60;
    static final long HMOD = 2147483647L;

    static int V, E;
    static int[][] M;
    static int[] off, av, aw, h, pos;
    static long[] d;
    static byte[] vis;
    static long upd, hsh;

    static void runA() {
        for (int i = 0; i < V; i++) { d[i] = INF; vis[i] = 0; }
        d[0] = 0; upd = 0; hsh = 0;
        for (int it = 0; it < V; it++) {
            int u = -1; long best = INF;
            for (int v = 0; v < V; v++) if (vis[v] == 0 && d[v] < best) { best = d[v]; u = v; }
            if (u < 0) break;
            vis[u] = 1; hsh = (hsh * 31 + u) % HMOD;
            int[] row = M[u];
            for (int v = 0; v < V; v++) { int w = row[v]; if (w != 0 && best + w < d[v]) { d[v] = best + w; upd++; } }
        }
    }

    static void up(int i) {
        int v = h[i]; long k = d[v];
        while (i > 0) { int p = (i - 1) / 2; if (d[h[p]] <= k) break; h[i] = h[p]; pos[h[i]] = i; i = p; }
        h[i] = v; pos[v] = i;
    }

    static void down(int i, int n) {
        int v = h[i]; long k = d[v];
        while (true) {
            int l = 2 * i + 1;
            if (l >= n) break;
            int c = l;
            if (l + 1 < n && d[h[l + 1]] < d[h[l]]) c = l + 1;
            if (d[h[c]] >= k) break;
            h[i] = h[c]; pos[h[i]] = i; i = c;
        }
        h[i] = v; pos[v] = i;
    }

    static void runB() {
        for (int i = 0; i < V; i++) { d[i] = INF; h[i] = i; pos[i] = i; }
        d[0] = 0; upd = 0; hsh = 0;
        int n = V;
        while (n > 0) {
            int u = h[0]; pos[u] = -1; n--;
            if (n > 0) { h[0] = h[n]; pos[h[0]] = 0; down(0, n); }
            long du = d[u];
            if (du == INF) break;
            hsh = (hsh * 31 + u) % HMOD;
            for (int e = off[u]; e < off[u + 1]; e++) {
                int v = av[e]; long nd = du + aw[e];
                if (nd < d[v]) { d[v] = nd; upd++; up(pos[v]); }
            }
        }
    }

    static long sumd() { long s = 0; for (int i = 0; i < V; i++) s += d[i]; return s; }

    static double measure(Runnable f) {
        long ws = System.nanoTime(); int k = 0;
        while (k == 0 || (System.nanoTime() - ws) * 1e-9 < 0.5) { f.run(); k++; }
        long t0 = System.nanoTime(); f.run(); double once = (System.nanoTime() - t0) * 1e-9;
        int reps = once >= 0.03 ? 1 : (int) Math.ceil(0.03 / once);
        int trials = once > 10.0 ? 1 : once > 1.0 ? 3 : 5;
        double[] ts = new double[trials];
        for (int t = 0; t < trials; t++) { t0 = System.nanoTime(); for (int r = 0; r < reps; r++) f.run(); ts[t] = (System.nanoTime() - t0) * 1e-9 / reps; }
        Arrays.sort(ts);
        return ts[trials / 2] * 1e3;
    }

    public static void main(String[] args) throws Exception {
        ByteBuffer bb = ByteBuffer.wrap(Files.readAllBytes(Path.of(args[0]))).order(ByteOrder.LITTLE_ENDIAN);
        V = bb.getInt(); E = bb.getInt();
        int[] ed = new int[3 * E];
        for (int i = 0; i < 3 * E; i++) ed[i] = bb.getInt();
        M = new int[V][];
        for (int u = 0; u < V; u++) {
            M[u] = new int[V];
            for (int v = 0; v < V; v++) M[u][v] = -1;
            for (int v = 0; v < V; v++) M[u][v] = 0;
        }
        off = new int[V + 1]; av = new int[E]; aw = new int[E];
        for (int e = 0; e < E; e++) { int u = ed[3 * e]; M[u][ed[3 * e + 1]] = ed[3 * e + 2]; off[u + 1]++; }
        for (int u = 0; u < V; u++) off[u + 1] += off[u];
        int[] cur = new int[V];
        for (int u = 0; u < V; u++) cur[u] = off[u];
        for (int e = 0; e < E; e++) { int u = ed[3 * e]; int k = cur[u]; cur[u] = k + 1; av[k] = ed[3 * e + 1]; aw[k] = ed[3 * e + 2]; }
        d = new long[V]; vis = new byte[V]; h = new int[V]; pos = new int[V];

        runA(); long sa = sumd(), ua = upd, ha = hsh;
        runB(); long sb = sumd(), ub = upd, hb = hsh;
        System.out.println("CHECK " + sa + " " + ua + " " + ha + " " + sb + " " + ub + " " + hb);
        if (args.length > 1 && args[1].equals("check")) return;
        double ta = measure(XBench::runA), tb = measure(XBench::runB);
        System.out.printf("TIME %.6f %.6f%n", ta, tb);
    }
}
