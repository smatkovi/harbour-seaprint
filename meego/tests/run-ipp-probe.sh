#!/bin/sh
# Starts CUPS's own IPP test printer and asks it for its attributes with the
# ported code, as an ARM binary under qemu-arm. See meego/tests/ipp_probe.cpp.
set -e
HERE=$(cd "$(dirname "$0")/../.." && pwd)
SYSROOT=${SYSROOT:-$HOME/QtSDK/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim}
PORT=${PORT:-8632}

command -v ippeveprinter >/dev/null || { echo "ippeveprinter (cups) not found" >&2; exit 2; }

sh "$HERE/meego/build.sh" tools >/dev/null 2>&1

# -r off: without a running avahi-daemon ippeveprinter dies in the DNS-SD
# registration, and the probe talks to it directly anyway.
ippeveprinter -r off -p "$PORT" -f application/pdf,image/pwg-raster,image/urf SeaPrintTest \
    >"$HERE/build/meego/ippeveprinter.log" 2>&1 &
PRINTER=$!
trap 'kill $PRINTER 2>/dev/null' EXIT
# Give it a moment to bind the port.
i=0
while [ $i -lt 50 ] && ! (exec 3<>/dev/tcp/127.0.0.1/$PORT) 2>/dev/null; do i=$((i+1)); sleep 0.1; done

qemu-arm -L "$SYSROOT" "$HERE/build/meego/tools/ipp_probe" "http://127.0.0.1:$PORT/ipp/print"
