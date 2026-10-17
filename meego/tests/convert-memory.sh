#!/bin/sh
# What a conversion costs in memory, measured with the device's own poppler and
# cairo -- on the build machine, under qemu-arm. Run it inside the synced tree:
#
#   meego/tests/convert-memory.sh <datei.pdf> [formate...]   # default: pdf urf pwg
#
# The phone has about 70 MB free in practice, so a print that ends with the app
# simply gone is first of all a question of how much the conversion asks for.
# This answers it without the device and without paper: peak resident size per
# target format, from the kernel, for the very code the app runs.
#
# qemu-arm adds a few MB of its own, and the raster formats depend on the
# resolution (-r), which is where the big numbers come from: A4 at 600 dpi is
# 4960x7016 pixels, three bytes each, in one allocation.
set -e
HERE=$(cd "$(dirname "$0")/../.." && pwd)
SYSROOT=${SYSROOT:-$HOME/QtSDK/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim}
RESOLUTION=${RESOLUTION:-600}
PDF=$1
[ -n "$PDF" ] || { echo "usage: $0 <datei.pdf> [formate...]" >&2; exit 2; }
shift
FORMATS=${*:-pdf urf pwg}

sh "$HERE/meego/build.sh" tools > "$HERE/build/meego/tools-build.log" 2>&1 ||
    { tail -5 "$HERE/build/meego/tools-build.log"; echo "== build failed" >&2; exit 1; }

TOOL=$HERE/build/meego/tools/pdf2printable
OUT=$(mktemp -d)
trap 'rm -rf "$OUT"' EXIT

echo "Datei: $PDF ($(wc -c < "$PDF") Byte), Auflösung ${RESOLUTION} dpi"
for format in $FORMATS; do
    # The peak comes from the kernel by way of wait4(); GNU time would do the
    # same thing but is not installed everywhere.
    python3 - "$SYSROOT" "$TOOL" "$format" "$RESOLUTION" "$PDF" "$OUT/out.$format" <<'PY'
import os, resource, subprocess, sys
sysroot, tool, fmt, res, pdf, out = sys.argv[1:7]
cmd = ["qemu-arm", "-L", sysroot, tool, "-f", fmt, "-r", res,
       "--paper-size", "iso_a4_210x297mm", pdf, out]
before = resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
rc = subprocess.call(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
peak = resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
size = os.path.getsize(out) if os.path.exists(out) else 0
print("  %-4s Spitze %6d kB, Ausgabe %9d Byte%s"
      % (fmt, max(peak, before), size, "" if rc == 0 else "  (rc=%d)" % rc))
PY
    rm -f "$OUT/out.$format"
done
