#!/bin/sh
# How reliably the search finds a printer -- to be run ON THE DEVICE, next to
# the discovery probe (meego/build.sh probe, then copy it to /home/user):
#
#   sh discovery-runs.sh [runs] [seconds per run]
#
# mDNS is one UDP packet each way and wifi drops some, so the only honest
# answer about discovery is a count over several tries. It prints one line per
# run and a tally at the end.
RUNS=${1:-10}
SECONDS_PER_RUN=${2:-6}
PROBE=${PROBE:-./discovery_probe}

ok=0
run=1
while [ "$run" -le "$RUNS" ]; do
    found=$("$PROBE" "$SECONDS_PER_RUN" 2>/dev/null | grep -c "ipp://")
    if [ "$found" -gt 0 ]; then
        ok=$((ok + 1))
        echo "Lauf $run: gefunden"
    else
        echo "Lauf $run: nichts"
    fi
    run=$((run + 1))
done
echo "== $ok von $RUNS Laeufen haben den Drucker gefunden"
