// Cross-language benchmark, JavaScript version (same specification as x_c.c).
// JavaScript has no fast 64-bit integer array, so distances use Float64Array; every
// distance here is below 2^53, so values and comparisons are exact.
'use strict';
const fs = require('fs');

const INF = 2 ** 60;
const HMOD = 2147483647;

const buf = fs.readFileSync(process.argv[2]);
const ed = new Int32Array(buf.buffer.slice(buf.byteOffset, buf.byteOffset + buf.length));
const V = ed[0], E = ed[1];
const M = new Array(V);
for (let u = 0; u < V; u++) {
  const row = new Int32Array(V);
  for (let v = 0; v < V; v++) row[v] = -1;
  for (let v = 0; v < V; v++) row[v] = 0;
  M[u] = row;
}
const off = new Int32Array(V + 1), av = new Int32Array(E), aw = new Int32Array(E);
for (let e = 0; e < E; e++) { const u = ed[2 + 3 * e]; M[u][ed[3 + 3 * e]] = ed[4 + 3 * e]; off[u + 1]++; }
for (let u = 0; u < V; u++) off[u + 1] += off[u];
const cur = new Int32Array(V);
for (let u = 0; u < V; u++) cur[u] = off[u];
for (let e = 0; e < E; e++) { const u = ed[2 + 3 * e]; const k = cur[u]; cur[u] = k + 1; av[k] = ed[3 + 3 * e]; aw[k] = ed[4 + 3 * e]; }
const d = new Float64Array(V), vis = new Uint8Array(V), h = new Int32Array(V), pos = new Int32Array(V);
let upd = 0, hsh = 0;

function runA() {
  for (let i = 0; i < V; i++) { d[i] = INF; vis[i] = 0; }
  d[0] = 0; upd = 0; hsh = 0;
  for (let it = 0; it < V; it++) {
    let u = -1, best = INF;
    for (let v = 0; v < V; v++) if (vis[v] === 0 && d[v] < best) { best = d[v]; u = v; }
    if (u < 0) break;
    vis[u] = 1; hsh = (hsh * 31 + u) % HMOD;
    const row = M[u];
    for (let v = 0; v < V; v++) { const w = row[v]; if (w !== 0 && best + w < d[v]) { d[v] = best + w; upd++; } }
  }
}

function up(i) {
  const v = h[i], k = d[v];
  while (i > 0) { const p = Math.floor((i - 1) / 2); if (d[h[p]] <= k) break; h[i] = h[p]; pos[h[i]] = i; i = p; }
  h[i] = v; pos[v] = i;
}

function down(i, n) {
  const v = h[i], k = d[v];
  while (true) {
    const l = 2 * i + 1;
    if (l >= n) break;
    let c = l;
    if (l + 1 < n && d[h[l + 1]] < d[h[l]]) c = l + 1;
    if (d[h[c]] >= k) break;
    h[i] = h[c]; pos[h[i]] = i; i = c;
  }
  h[i] = v; pos[v] = i;
}

function runB() {
  for (let i = 0; i < V; i++) { d[i] = INF; h[i] = i; pos[i] = i; }
  d[0] = 0; upd = 0; hsh = 0;
  let n = V;
  while (n > 0) {
    const u = h[0]; pos[u] = -1; n--;
    if (n > 0) { h[0] = h[n]; pos[h[0]] = 0; down(0, n); }
    const du = d[u];
    if (du === INF) break;
    hsh = (hsh * 31 + u) % HMOD;
    for (let e = off[u]; e < off[u + 1]; e++) {
      const v = av[e], nd = du + aw[e];
      if (nd < d[v]) { d[v] = nd; upd++; up(pos[v]); }
    }
  }
}

function sumd() { let s = 0; for (let i = 0; i < V; i++) s += d[i]; return s; }

const now = () => Number(process.hrtime.bigint()) * 1e-9;
function measure(f) {
  const ws = now(); let k = 0;
  while (k === 0 || now() - ws < 0.5) { f(); k++; }
  let t0 = now(); f(); const once = now() - t0;
  const reps = once >= 0.03 ? 1 : Math.ceil(0.03 / once);
  const trials = once > 10.0 ? 1 : once > 1.0 ? 3 : 5;
  const ts = [];
  for (let t = 0; t < trials; t++) { t0 = now(); for (let r = 0; r < reps; r++) f(); ts.push((now() - t0) / reps); }
  ts.sort((a, b) => a - b);
  return ts[Math.floor(trials / 2)] * 1e3;
}

runA(); const sa = sumd(), ua = upd, ha = hsh;
runB(); const sb = sumd(), ub = upd, hb = hsh;
console.log(`CHECK ${sa} ${ua} ${ha} ${sb} ${ub} ${hb}`);
if (process.argv[3] !== 'check') {
  const ta = measure(runA), tb = measure(runB);
  console.log(`TIME ${ta.toFixed(6)} ${tb.toFixed(6)}`);
}
