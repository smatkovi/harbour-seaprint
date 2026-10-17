#!/bin/sh
# Builds and runs the IPP parse test over saved responses.
# See meego/tests/ipp_parse_test.cpp.
#
#   meego/tests/run-parse-test.sh <datei.raw> ...
#
# A capture comes either from the device (the app's own dump of a response it
# could not parse) or from meego/tests/ipp-capture.py against a live printer.
set -e
HERE=$(cd "$(dirname "$0")/../.." && pwd)
LOG=$HERE/build/meego/parse-build.log
mkdir -p "$(dirname "$LOG")"
if ! sh "$HERE/meego/build.sh" parse > "$LOG" 2>&1; then
    grep -E "error:|Error [0-9]" "$LOG" | head -30
    echo "== build failed, full log in $LOG" >&2
    exit 1
fi
exec "$HERE/build/meego/parse/ipp_parse_test" "$@"
