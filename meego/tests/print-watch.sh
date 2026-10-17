#!/bin/sh
# Prints a file with the probe and writes down what it costs in memory -- to be
# run ON THE DEVICE, next to the probe (meego/remote-build.sh probe):
#
#   sh print-watch.sh ipp://<host>[:port]/<rp> <datei> [format]
#
# The device has about 70 MB to spare, and the conversion is the one step that
# can ask for more than that, so a print that ends with the app simply gone
# looks the same as a crash. This prints a line a second with the probe's
# resident size and the free memory, and says how the probe ended -- "Killed"
# is the kernel, not the code.
#
# A CUPS test printer makes a good target while looking into this, since it
# uses no paper:
#   ippeveprinter -r off -p 8632 -f application/pdf,image/pwg-raster,image/urf Test
URL=$1
FILE=$2
FORMAT=$3
PROBE=${PROBE:-./discovery_probe}
LOG=${LOG:-print-watch.log}

[ -n "$URL" ] && [ -n "$FILE" ] || { echo "usage: $0 <ipp-url> <datei> [format]" >&2; exit 2; }

"$PROBE" 180 "$URL" "$FILE" $FORMAT > "$LOG" 2>&1 &
PID=$!

peak=0
childpeak=0
while kill -0 "$PID" 2>/dev/null; do
    rss=$(awk '/^VmRSS:/{print $2}' "/proc/$PID/status" 2>/dev/null)
    free=$(awk '/^MemFree:/{print $2}' /proc/meminfo)
    swap=$(awk '/^SwapFree:/{print $2}' /proc/meminfo)
    [ -n "$rss" ] || break
    [ "$rss" -gt "$peak" ] && peak=$rss
    # The conversion runs in a child of its own (meego/convertproc.h), which is
    # the whole point of this measurement: it is the child that gets big.
    child=0
    for status in /proc/[0-9]*/status; do
        pid=${status%/status}; pid=${pid#/proc/}
        case "$(tr '\0' ' ' < /proc/$pid/cmdline 2>/dev/null)" in
            *--convert*) child=$(awk '/^VmRSS:/{print $2}' "$status" 2>/dev/null) ;;
        esac
    done
    [ -n "$child" ] || child=0
    [ "$child" -gt "$childpeak" ] && childpeak=$child
    echo "$(date +%H:%M:%S)  RSS ${rss} kB   Umwandler ${child} kB   frei ${free} kB   Swap frei ${swap} kB"
    sleep 1
done

wait "$PID"
rc=$?
echo "== Probe beendet, rc=$rc, Spitze RSS ${peak} kB, Spitze Umwandler ${childpeak} kB"
echo "== letzte Zeilen aus $LOG:"
tail -15 "$LOG"
