# Cross-language benchmark, Python version (same specification as x_c.c).
# Uses the standard-library `array` module so the data layout matches the other languages:
# flat, contiguous int32 / int64 / byte arrays instead of lists of boxed objects.
import sys
import time
from array import array

INF = 1 << 60
HMOD = 2147483647


def main():
    ed = array('i')
    with open(sys.argv[1], 'rb') as f:
        ed.frombytes(f.read())
    V, E = ed[0], ed[1]
    M = []
    for u in range(V):
        row = array('i', bytes(4 * V))
        for v in range(V):
            row[v] = -1
        for v in range(V):
            row[v] = 0
        M.append(row)
    off = array('i', bytes(4 * (V + 1)))
    av = array('i', bytes(4 * E))
    aw = array('i', bytes(4 * E))
    for e in range(E):
        u = ed[2 + 3 * e]
        M[u][ed[3 + 3 * e]] = ed[4 + 3 * e]
        off[u + 1] += 1
    for u in range(V):
        off[u + 1] += off[u]
    cur = array('i', bytes(4 * V))
    for u in range(V):
        cur[u] = off[u]
    for e in range(E):
        u = ed[2 + 3 * e]
        k = cur[u]
        cur[u] = k + 1
        av[k] = ed[3 + 3 * e]
        aw[k] = ed[4 + 3 * e]
    d = array('q', bytes(8 * V))
    vis = bytearray(V)
    h = array('i', bytes(4 * V))
    pos = array('i', bytes(4 * V))
    state = [0, 0]  # upd, hsh

    def run_a():
        for i in range(V):
            d[i] = INF
            vis[i] = 0
        d[0] = 0
        upd = 0
        hsh = 0
        for it in range(V):
            u = -1
            best = INF
            for v in range(V):
                if vis[v] == 0 and d[v] < best:
                    best = d[v]
                    u = v
            if u < 0:
                break
            vis[u] = 1
            hsh = (hsh * 31 + u) % HMOD
            row = M[u]
            for v in range(V):
                w = row[v]
                if w != 0 and best + w < d[v]:
                    d[v] = best + w
                    upd += 1
        state[0] = upd
        state[1] = hsh

    def up(i):
        v = h[i]
        k = d[v]
        while i > 0:
            p = (i - 1) // 2
            if d[h[p]] <= k:
                break
            h[i] = h[p]
            pos[h[i]] = i
            i = p
        h[i] = v
        pos[v] = i

    def down(i, n):
        v = h[i]
        k = d[v]
        while True:
            l = 2 * i + 1
            if l >= n:
                break
            c = l
            if l + 1 < n and d[h[l + 1]] < d[h[l]]:
                c = l + 1
            if d[h[c]] >= k:
                break
            h[i] = h[c]
            pos[h[i]] = i
            i = c
        h[i] = v
        pos[v] = i

    def run_b():
        for i in range(V):
            d[i] = INF
            h[i] = i
            pos[i] = i
        d[0] = 0
        upd = 0
        hsh = 0
        n = V
        while n > 0:
            u = h[0]
            pos[u] = -1
            n -= 1
            if n > 0:
                h[0] = h[n]
                pos[h[0]] = 0
                down(0, n)
            du = d[u]
            if du == INF:
                break
            hsh = (hsh * 31 + u) % HMOD
            for e in range(off[u], off[u + 1]):
                v = av[e]
                nd = du + aw[e]
                if nd < d[v]:
                    d[v] = nd
                    upd += 1
                    up(pos[v])
        state[0] = upd
        state[1] = hsh

    def sumd():
        s = 0
        for i in range(V):
            s += d[i]
        return s

    def measure(f):
        ws = time.perf_counter()
        k = 0
        while k == 0 or time.perf_counter() - ws < 0.5:
            f()
            k += 1
        t0 = time.perf_counter()
        f()
        once = time.perf_counter() - t0
        reps = 1 if once >= 0.03 else int(-(-0.03 // once))
        trials = 1 if once > 10.0 else 3 if once > 1.0 else 5
        ts = []
        for _ in range(trials):
            t0 = time.perf_counter()
            for _ in range(reps):
                f()
            ts.append((time.perf_counter() - t0) / reps)
        ts.sort()
        return ts[trials // 2] * 1e3

    run_a()
    sa, ua, ha = sumd(), state[0], state[1]
    run_b()
    sb, ub, hb = sumd(), state[0], state[1]
    print(f"CHECK {sa} {ua} {ha} {sb} {ub} {hb}", flush=True)
    if len(sys.argv) > 2 and sys.argv[2] == 'check':
        return
    ta = measure(run_a)
    tb = measure(run_b)
    print(f"TIME {ta:.6f} {tb:.6f}")


main()
