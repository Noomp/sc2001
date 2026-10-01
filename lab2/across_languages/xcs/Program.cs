// Cross-language benchmark, C# version (same specification as x_c.c).
using System;
using System.Diagnostics;
using System.IO;

static class XBench
{
    const long INF = 1L << 60;
    const long HMOD = 2147483647L;

    static int V, E;
    static int[][] M;
    static int[] off, av, aw, h, pos;
    static long[] d;
    static byte[] vis;
    static long upd, hsh;

    static void RunA()
    {
        for (int i = 0; i < V; i++) { d[i] = INF; vis[i] = 0; }
        d[0] = 0; upd = 0; hsh = 0;
        for (int it = 0; it < V; it++)
        {
            int u = -1; long best = INF;
            for (int v = 0; v < V; v++) if (vis[v] == 0 && d[v] < best) { best = d[v]; u = v; }
            if (u < 0) break;
            vis[u] = 1; hsh = (hsh * 31 + u) % HMOD;
            int[] row = M[u];
            for (int v = 0; v < V; v++) { int w = row[v]; if (w != 0 && best + w < d[v]) { d[v] = best + w; upd++; } }
        }
    }

    static void Up(int i)
    {
        int v = h[i]; long k = d[v];
        while (i > 0) { int p = (i - 1) / 2; if (d[h[p]] <= k) break; h[i] = h[p]; pos[h[i]] = i; i = p; }
        h[i] = v; pos[v] = i;
    }

    static void Down(int i, int n)
    {
        int v = h[i]; long k = d[v];
        while (true)
        {
            int l = 2 * i + 1;
            if (l >= n) break;
            int c = l;
            if (l + 1 < n && d[h[l + 1]] < d[h[l]]) c = l + 1;
            if (d[h[c]] >= k) break;
            h[i] = h[c]; pos[h[i]] = i; i = c;
        }
        h[i] = v; pos[v] = i;
    }

    static void RunB()
    {
        for (int i = 0; i < V; i++) { d[i] = INF; h[i] = i; pos[i] = i; }
        d[0] = 0; upd = 0; hsh = 0;
        int n = V;
        while (n > 0)
        {
            int u = h[0]; pos[u] = -1; n--;
            if (n > 0) { h[0] = h[n]; pos[h[0]] = 0; Down(0, n); }
            long du = d[u];
            if (du == INF) break;
            hsh = (hsh * 31 + u) % HMOD;
            for (int e = off[u]; e < off[u + 1]; e++)
            {
                int v = av[e]; long nd = du + aw[e];
                if (nd < d[v]) { d[v] = nd; upd++; Up(pos[v]); }
            }
        }
    }

    static long SumD() { long s = 0; for (int i = 0; i < V; i++) s += d[i]; return s; }

    static double Measure(Action f)
    {
        var ws = Stopwatch.StartNew(); int k = 0;
        while (k == 0 || ws.Elapsed.TotalSeconds < 0.5) { f(); k++; }
        var sw = Stopwatch.StartNew(); f(); double once = sw.Elapsed.TotalSeconds;
        int reps = once >= 0.03 ? 1 : (int)Math.Ceiling(0.03 / once);
        int trials = once > 10.0 ? 1 : once > 1.0 ? 3 : 5;
        var ts = new double[trials];
        for (int t = 0; t < trials; t++) { sw.Restart(); for (int r = 0; r < reps; r++) f(); ts[t] = sw.Elapsed.TotalSeconds / reps; }
        Array.Sort(ts);
        return ts[trials / 2] * 1e3;
    }

    static void Main(string[] args)
    {
        var bytes = File.ReadAllBytes(args[0]);
        V = BitConverter.ToInt32(bytes, 0); E = BitConverter.ToInt32(bytes, 4);
        var ed = new int[3 * E];
        for (int i = 0; i < 3 * E; i++) ed[i] = BitConverter.ToInt32(bytes, 8 + 4 * i);
        M = new int[V][];
        for (int u = 0; u < V; u++)
        {
            M[u] = new int[V];
            for (int v = 0; v < V; v++) M[u][v] = -1;
            for (int v = 0; v < V; v++) M[u][v] = 0;
        }
        off = new int[V + 1]; av = new int[E]; aw = new int[E];
        for (int e = 0; e < E; e++) { int u = ed[3 * e]; M[u][ed[3 * e + 1]] = ed[3 * e + 2]; off[u + 1]++; }
        for (int u = 0; u < V; u++) off[u + 1] += off[u];
        var cur = new int[V];
        for (int u = 0; u < V; u++) cur[u] = off[u];
        for (int e = 0; e < E; e++) { int u = ed[3 * e]; int k = cur[u]; cur[u] = k + 1; av[k] = ed[3 * e + 1]; aw[k] = ed[3 * e + 2]; }
        d = new long[V]; vis = new byte[V]; h = new int[V]; pos = new int[V];

        RunA(); long sa = SumD(), ua = upd, ha = hsh;
        RunB(); long sb = SumD(), ub = upd, hb = hsh;
        Console.WriteLine($"CHECK {sa} {ua} {ha} {sb} {ub} {hb}");
        if (args.Length > 1 && args[1] == "check") return;
        double ta = Measure(RunA), tb = Measure(RunB);
        Console.WriteLine($"TIME {ta:F6} {tb:F6}");
    }
}
