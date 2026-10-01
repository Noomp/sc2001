/* gen2.c V -- writes the 9 graphs for one |V| (same generator as lab2/dijkstra.c).
 * File format: int32 V, int32 E, then E x (u, v, w) int32, little-endian.
 * Prints "<file> <V> <E> <label>" per graph. */
#define main orig_main
#include "../dijkstra.c"
#undef main

static void emit(const char *fn, const char *label, int V, Edge *ed, ll E) {
    FILE *f = fopen(fn, "wb");
    int hdr[2] = {V, (int)E};
    fwrite(hdr, 4, 2, f);
    for (ll e = 0; e < E; e++) { int t[3] = {ed[e].u, ed[e].v, ed[e].w}; fwrite(t, 4, 3, f); }
    fclose(f);
    printf("%s %d %lld %s\n", fn, V, E, label);
    free(ed);
}

int main(int argc, char **argv) {
    int V = atoi(argv[1]);
    ll maxE = (ll)V * (V - 1);
    struct { const char *label; double frac; } s[] = {
        {"8V", -1}, {"1%", 0.01}, {"10%", 0.10}, {"25%", 0.25}, {"50%", 0.50},
        {"75%", 0.75}, {"90%", 0.90}, {"100%", 1.0}};
    for (int i = 0; i < 8; i++) {
        ll E = s[i].frac < 0 ? 8LL * V : llround(s[i].frac * maxE);
        if (E < V - 1) E = V - 1;
        if (E > maxE) E = maxE;
        seed(70000 + 100 * (ll)V + i);
        Edge *ed = gen_random(V, &E);
        char fn[64]; snprintf(fn, sizeof fn, "g_%d_%d.bin", V, i);
        emit(fn, s[i].label, V, ed, E);
    }
    ll E; Edge *ed = gen_adv(V, &E);
    char fn[64]; snprintf(fn, sizeof fn, "g_%d_adv.bin", V);
    emit(fn, "adv", V, ed, E);
    return 0;
}
