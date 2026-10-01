// Cross-language benchmark, Go version (same specification as x_c.c).
package main

import (
	"encoding/binary"
	"fmt"
	"math"
	"os"
	"sort"
	"time"
)

const INF int64 = 1 << 60
const HMOD int64 = 2147483647

var (
	V, E                     int
	M                        [][]int32
	off, av, aw, h, pos      []int32
	d                        []int64
	vis                      []uint8
	upd, hsh                 int64
)

func runA() {
	for i := 0; i < V; i++ {
		d[i] = INF
		vis[i] = 0
	}
	d[0] = 0
	upd = 0
	hsh = 0
	for it := 0; it < V; it++ {
		u := -1
		best := INF
		for v := 0; v < V; v++ {
			if vis[v] == 0 && d[v] < best {
				best = d[v]
				u = v
			}
		}
		if u < 0 {
			break
		}
		vis[u] = 1
		hsh = (hsh*31 + int64(u)) % HMOD
		row := M[u]
		for v := 0; v < V; v++ {
			w := row[v]
			if w != 0 && best+int64(w) < d[v] {
				d[v] = best + int64(w)
				upd++
			}
		}
	}
}

func up(i int) {
	v := h[i]
	k := d[v]
	for i > 0 {
		p := (i - 1) / 2
		if d[h[p]] <= k {
			break
		}
		h[i] = h[p]
		pos[h[i]] = int32(i)
		i = p
	}
	h[i] = v
	pos[v] = int32(i)
}

func down(i, n int) {
	v := h[i]
	k := d[v]
	for {
		l := 2*i + 1
		if l >= n {
			break
		}
		c := l
		if l+1 < n && d[h[l+1]] < d[h[l]] {
			c = l + 1
		}
		if d[h[c]] >= k {
			break
		}
		h[i] = h[c]
		pos[h[i]] = int32(i)
		i = c
	}
	h[i] = v
	pos[v] = int32(i)
}

func runB() {
	for i := 0; i < V; i++ {
		d[i] = INF
		h[i] = int32(i)
		pos[i] = int32(i)
	}
	d[0] = 0
	upd = 0
	hsh = 0
	n := V
	for n > 0 {
		u := int(h[0])
		pos[u] = -1
		n--
		if n > 0 {
			h[0] = h[n]
			pos[h[0]] = 0
			down(0, n)
		}
		du := d[u]
		if du == INF {
			break
		}
		hsh = (hsh*31 + int64(u)) % HMOD
		for e := int(off[u]); e < int(off[u+1]); e++ {
			v := int(av[e])
			nd := du + int64(aw[e])
			if nd < d[v] {
				d[v] = nd
				upd++
				up(int(pos[v]))
			}
		}
	}
}

func sumd() int64 {
	var s int64
	for i := 0; i < V; i++ {
		s += d[i]
	}
	return s
}

func measure(f func()) float64 {
	ws := time.Now()
	k := 0
	for k == 0 || time.Since(ws).Seconds() < 0.5 {
		f()
		k++
	}
	t0 := time.Now()
	f()
	once := time.Since(t0).Seconds()
	reps := 1
	if once < 0.03 {
		reps = int(math.Ceil(0.03 / once))
	}
	trials := 5
	if once > 1.0 {
		trials = 3
	}
	if once > 10.0 {
		trials = 1
	}
	ts := make([]float64, trials)
	for t := 0; t < trials; t++ {
		t0 = time.Now()
		for r := 0; r < reps; r++ {
			f()
		}
		ts[t] = time.Since(t0).Seconds() / float64(reps)
	}
	sort.Float64s(ts)
	return ts[trials/2] * 1e3
}

func main() {
	b, err := os.ReadFile(os.Args[1])
	if err != nil {
		panic(err)
	}
	rd := func(i int) int32 { return int32(binary.LittleEndian.Uint32(b[4*i:])) }
	V, E = int(rd(0)), int(rd(1))
	M = make([][]int32, V)
	for u := 0; u < V; u++ {
		M[u] = make([]int32, V)
		for v := 0; v < V; v++ {
			M[u][v] = -1
		}
		for v := 0; v < V; v++ {
			M[u][v] = 0
		}
	}
	off = make([]int32, V+1)
	av = make([]int32, E)
	aw = make([]int32, E)
	for e := 0; e < E; e++ {
		u := int(rd(2 + 3*e))
		M[u][int(rd(3+3*e))] = rd(4 + 3*e)
		off[u+1]++
	}
	for u := 0; u < V; u++ {
		off[u+1] += off[u]
	}
	cur := make([]int32, V)
	for u := 0; u < V; u++ {
		cur[u] = off[u]
	}
	for e := 0; e < E; e++ {
		u := int(rd(2 + 3*e))
		k := cur[u]
		cur[u] = k + 1
		av[k] = rd(3 + 3*e)
		aw[k] = rd(4 + 3*e)
	}
	d = make([]int64, V)
	vis = make([]uint8, V)
	h = make([]int32, V)
	pos = make([]int32, V)

	runA()
	sa, ua, ha := sumd(), upd, hsh
	runB()
	sb, ub, hb := sumd(), upd, hsh
	fmt.Printf("CHECK %d %d %d %d %d %d\n", sa, ua, ha, sb, ub, hb)
	if len(os.Args) > 2 && os.Args[2] == "check" {
		return
	}
	ta := measure(runA)
	tb := measure(runB)
	fmt.Printf("TIME %.6f %.6f\n", ta, tb)
}
