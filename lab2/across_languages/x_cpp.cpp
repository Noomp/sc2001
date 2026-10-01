// Cross-language benchmark, C++ version (same specification as x_c.c).
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

using ll = long long;
static const ll INF = 1LL << 60;
static const ll HMOD = 2147483647LL;

static int V, E;
static std::vector<std::vector<int>> M;
static std::vector<int> off, av, aw, h, pos;
static std::vector<ll> d;
static std::vector<unsigned char> vis;
static ll upd, hsh;

static double now() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }

static void run_a() {
    for (int i = 0; i < V; i++) { d[i] = INF; vis[i] = 0; }
    d[0] = 0; upd = 0; hsh = 0;
    for (int it = 0; it < V; it++) {
        int u = -1; ll best = INF;
        for (int v = 0; v < V; v++) if (vis[v] == 0 && d[v] < best) { best = d[v]; u = v; }
        if (u < 0) break;
        vis[u] = 1; hsh = (hsh * 31 + u) % HMOD;
        const std::vector<int> &row = M[u];
        for (int v = 0; v < V; v++) { int w = row[v]; if (w != 0 && best + w < d[v]) { d[v] = best + w; upd++; } }
    }
}

static void up(int i) {
    int v = h[i]; ll k = d[v];
    while (i > 0) { int p = (i - 1) / 2; if (d[h[p]] <= k) break; h[i] = h[p]; pos[h[i]] = i; i = p; }
    h[i] = v; pos[v] = i;
}

static void down(int i, int n) {
    int v = h[i]; ll k = d[v];
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

static void run_b() {
    for (int i = 0; i < V; i++) { d[i] = INF; h[i] = i; pos[i] = i; }
    d[0] = 0; upd = 0; hsh = 0;
    int n = V;
    while (n > 0) {
        int u = h[0]; pos[u] = -1; n--;
        if (n > 0) { h[0] = h[n]; pos[h[0]] = 0; down(0, n); }
        ll du = d[u];
        if (du == INF) break;
        hsh = (hsh * 31 + u) % HMOD;
        for (int e = off[u]; e < off[u + 1]; e++) {
            int v = av[e]; ll nd = du + aw[e];
            if (nd < d[v]) { d[v] = nd; upd++; up(pos[v]); }
        }
    }
}

static ll sumd() { ll s = 0; for (int i = 0; i < V; i++) s += d[i]; return s; }

static double measure(void (*f)()) {
    double ws = now(); int k = 0;
    while (k == 0 || now() - ws < 0.5) { f(); k++; }
    double t0 = now(); f(); double once = now() - t0;
    int reps = once >= 0.03 ? 1 : static_cast<int>(std::ceil(0.03 / once));
    int trials = once > 10.0 ? 1 : once > 1.0 ? 3 : 5;
    std::vector<double> ts(trials);
    for (int t = 0; t < trials; t++) { t0 = now(); for (int r = 0; r < reps; r++) f(); ts[t] = (now() - t0) / reps; }
    std::sort(ts.begin(), ts.end());
    return ts[trials / 2] * 1e3;
}

int main(int argc, char **argv) {
    FILE *f = std::fopen(argv[1], "rb");
    int hdr[2];
    if (std::fread(hdr, 4, 2, f) != 2) return 1;
    V = hdr[0]; E = hdr[1];
    std::vector<int> ed(3 * static_cast<size_t>(E));
    if (std::fread(ed.data(), 4, ed.size(), f) != ed.size()) return 1;
    std::fclose(f);
    M.resize(V);
    for (int u = 0; u < V; u++) {
        M[u].resize(V);
        for (int v = 0; v < V; v++) M[u][v] = -1;
        for (int v = 0; v < V; v++) M[u][v] = 0;
    }
    off.assign(V + 1, 0); av.resize(E); aw.resize(E);
    for (int e = 0; e < E; e++) { int u = ed[3 * e]; M[u][ed[3 * e + 1]] = ed[3 * e + 2]; off[u + 1]++; }
    for (int u = 0; u < V; u++) off[u + 1] += off[u];
    std::vector<int> cur(V);
    for (int u = 0; u < V; u++) cur[u] = off[u];
    for (int e = 0; e < E; e++) { int u = ed[3 * e]; int k = cur[u]; cur[u] = k + 1; av[k] = ed[3 * e + 1]; aw[k] = ed[3 * e + 2]; }
    d.resize(V); vis.resize(V); h.resize(V); pos.resize(V);

    run_a(); ll sa = sumd(), ua = upd, ha = hsh;
    run_b(); ll sb = sumd(), ub = upd, hb = hsh;
    std::printf("CHECK %lld %lld %lld %lld %lld %lld\n", sa, ua, ha, sb, ub, hb);
    std::fflush(stdout);
    if (argc > 2 && std::strcmp(argv[2], "check") == 0) return 0;
    double ta = measure(run_a), tb = measure(run_b);
    std::printf("TIME %.6f %.6f\n", ta, tb);
    return 0;
}
