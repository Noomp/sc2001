// Cross-language benchmark, Rust version (same specification as x_c.c).
use std::time::Instant;

const INF: i64 = 1 << 60;
const HMOD: i64 = 2147483647;

struct G {
    v: usize,
    m: Vec<Vec<i32>>,
    off: Vec<i32>,
    av: Vec<i32>,
    aw: Vec<i32>,
    h: Vec<i32>,
    pos: Vec<i32>,
    d: Vec<i64>,
    vis: Vec<u8>,
    upd: i64,
    hsh: i64,
}

impl G {
    fn run_a(&mut self) {
        let nv = self.v;
        for i in 0..nv {
            self.d[i] = INF;
            self.vis[i] = 0;
        }
        self.d[0] = 0;
        self.upd = 0;
        self.hsh = 0;
        for _it in 0..nv {
            let mut u: i32 = -1;
            let mut best = INF;
            for v in 0..nv {
                if self.vis[v] == 0 && self.d[v] < best {
                    best = self.d[v];
                    u = v as i32;
                }
            }
            if u < 0 {
                break;
            }
            let uu = u as usize;
            self.vis[uu] = 1;
            self.hsh = (self.hsh * 31 + u as i64) % HMOD;
            let row = &self.m[uu];
            for v in 0..nv {
                let w = row[v];
                if w != 0 && best + (w as i64) < self.d[v] {
                    self.d[v] = best + w as i64;
                    self.upd += 1;
                }
            }
        }
    }

    fn up(&mut self, mut i: usize) {
        let v = self.h[i];
        let k = self.d[v as usize];
        while i > 0 {
            let p = (i - 1) / 2;
            if self.d[self.h[p] as usize] <= k {
                break;
            }
            self.h[i] = self.h[p];
            self.pos[self.h[i] as usize] = i as i32;
            i = p;
        }
        self.h[i] = v;
        self.pos[v as usize] = i as i32;
    }

    fn down(&mut self, mut i: usize, n: usize) {
        let v = self.h[i];
        let k = self.d[v as usize];
        loop {
            let l = 2 * i + 1;
            if l >= n {
                break;
            }
            let mut c = l;
            if l + 1 < n && self.d[self.h[l + 1] as usize] < self.d[self.h[l] as usize] {
                c = l + 1;
            }
            if self.d[self.h[c] as usize] >= k {
                break;
            }
            self.h[i] = self.h[c];
            self.pos[self.h[i] as usize] = i as i32;
            i = c;
        }
        self.h[i] = v;
        self.pos[v as usize] = i as i32;
    }

    fn run_b(&mut self) {
        let nv = self.v;
        for i in 0..nv {
            self.d[i] = INF;
            self.h[i] = i as i32;
            self.pos[i] = i as i32;
        }
        self.d[0] = 0;
        self.upd = 0;
        self.hsh = 0;
        let mut n = nv;
        while n > 0 {
            let u = self.h[0] as usize;
            self.pos[u] = -1;
            n -= 1;
            if n > 0 {
                self.h[0] = self.h[n];
                let r = self.h[0] as usize;
                self.pos[r] = 0;
                self.down(0, n);
            }
            let du = self.d[u];
            if du == INF {
                break;
            }
            self.hsh = (self.hsh * 31 + u as i64) % HMOD;
            let (lo, hi) = (self.off[u] as usize, self.off[u + 1] as usize);
            for e in lo..hi {
                let v = self.av[e] as usize;
                let nd = du + self.aw[e] as i64;
                if nd < self.d[v] {
                    self.d[v] = nd;
                    self.upd += 1;
                    let p = self.pos[v] as usize;
                    self.up(p);
                }
            }
        }
    }

    fn sumd(&self) -> i64 {
        let mut s = 0i64;
        for i in 0..self.v {
            s += self.d[i];
        }
        s
    }
}

fn measure(g: &mut G, which: u8) -> f64 {
    let f = |g: &mut G| if which == 0 { g.run_a() } else { g.run_b() };
    let ws = Instant::now();
    let mut k = 0;
    while k == 0 || ws.elapsed().as_secs_f64() < 0.5 {
        f(g);
        k += 1;
    }
    let t0 = Instant::now();
    f(g);
    let once = t0.elapsed().as_secs_f64();
    let reps = if once >= 0.03 { 1 } else { (0.03 / once).ceil() as usize };
    let trials = if once > 10.0 { 1 } else if once > 1.0 { 3 } else { 5 };
    let mut ts = Vec::with_capacity(trials);
    for _ in 0..trials {
        let t0 = Instant::now();
        for _ in 0..reps {
            f(g);
        }
        ts.push(t0.elapsed().as_secs_f64() / reps as f64);
    }
    ts.sort_by(|a, b| a.partial_cmp(b).unwrap());
    ts[trials / 2] * 1e3
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let b = std::fs::read(&args[1]).unwrap();
    let rd = |i: usize| i32::from_le_bytes([b[4 * i], b[4 * i + 1], b[4 * i + 2], b[4 * i + 3]]);
    let v = rd(0) as usize;
    let e = rd(1) as usize;
    let mut m: Vec<Vec<i32>> = Vec::with_capacity(v);
    for _u in 0..v {
        let mut row = vec![0i32; v];
        for x in 0..v {
            row[x] = -1;
        }
        for x in 0..v {
            row[x] = 0;
        }
        m.push(row);
    }
    let mut off = vec![0i32; v + 1];
    let mut av = vec![0i32; e];
    let mut aw = vec![0i32; e];
    for k in 0..e {
        let u = rd(2 + 3 * k) as usize;
        m[u][rd(3 + 3 * k) as usize] = rd(4 + 3 * k);
        off[u + 1] += 1;
    }
    for u in 0..v {
        off[u + 1] += off[u];
    }
    let mut cur = vec![0i32; v];
    for u in 0..v {
        cur[u] = off[u];
    }
    for k in 0..e {
        let u = rd(2 + 3 * k) as usize;
        let p = cur[u] as usize;
        cur[u] += 1;
        av[p] = rd(3 + 3 * k);
        aw[p] = rd(4 + 3 * k);
    }
    let mut g = G { v, m, off, av, aw, h: vec![0; v], pos: vec![0; v], d: vec![0; v], vis: vec![0; v], upd: 0, hsh: 0 };
    g.run_a();
    let (sa, ua, ha) = (g.sumd(), g.upd, g.hsh);
    g.run_b();
    let (sb, ub, hb) = (g.sumd(), g.upd, g.hsh);
    println!("CHECK {} {} {} {} {} {}", sa, ua, ha, sb, ub, hb);
    if args.len() > 2 && args[2] == "check" {
        return;
    }
    let ta = measure(&mut g, 0);
    let tb = measure(&mut g, 1);
    println!("TIME {:.6} {:.6}", ta, tb);
}
