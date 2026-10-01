#!/usr/bin/env bash
# Full cross-language sweep. Writes results_x.csv and progress to stdout.
cd "$(dirname "$0")"
source ./env.sh
build_all || { echo BUILD_FAIL; exit 1; }

VS=(250 500 1000 2000 4000 8000 16000 32000 50000)
EDGE_CAP=32000000        # graphs with more edges than this are skipped (file/memory size)
PY_MAX_V=16000           # Python is skipped above this |V| (one run would take minutes)
OUT=results_x.csv
echo "rep,V,E,label,lang,ta_ms,tb_ms,check" > $OUT

for rep in 1 2 3; do
    # rotate the language order each repeat
    order=("${LANGS[@]:$(( (rep - 1) * 3 % 8 ))}" "${LANGS[@]:0:$(( (rep - 1) * 3 % 8 ))}")
    for V in "${VS[@]}"; do
        if [ $rep -gt 1 ] && [ $V -gt 4000 ]; then continue; fi
        ./gen2 $V > list_$V.txt
        while read f VV E lab; do
            if [ $E -gt $EDGE_CAP ]; then rm -f $f; continue; fi
            ref=$(./x_c $f check)
            for lg in "${order[@]}"; do
                if [ "$lg" == "Python" ] && [ $V -gt $PY_MAX_V ]; then continue; fi
                out=$(timeout 7200 $(cmd_for $lg) $f 2>&1)
                chk=$(echo "$out" | grep '^CHECK')
                tim=$(echo "$out" | grep '^TIME')
                ok=$([ "$chk" == "$ref" ] && echo ok || echo MISMATCH)
                ta=$(echo "$tim" | awk '{print $2}'); tb=$(echo "$tim" | awk '{print $3}')
                echo "$rep,$V,$E,$lab,$lg,${ta:-NA},${tb:-NA},$ok" >> $OUT
                echo "rep$rep V=$V $lab $lg a=${ta:-NA} b=${tb:-NA} $ok"
            done
            rm -f $f
        done < list_$V.txt
    done
done
echo ALL_DONE
