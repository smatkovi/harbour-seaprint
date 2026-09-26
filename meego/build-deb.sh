#!/bin/sh
# Packages the ARM build as a Harmattan .deb. Runs on the build machine after
# "meego/build.sh arm":
#
#   meego/build-deb.sh                 # -> build/meego/harbour-seaprint_<version>_armel.deb
#   VERSION=1.3.4-meego2 meego/build-deb.sh
#
# Installed layout on the N9:
#   /opt/harbour-seaprint/bin/harbour-seaprint
#   /opt/harbour-seaprint/{qml,translations}
#   /usr/share/applications/harbour-seaprint.desktop
#   /usr/share/icons/hicolor/80x80/apps/harbour-seaprint.png
#
# The .deb is written by mkdeb.py: Harmattan's dpkg is 1.15 and wants gzip
# members and no trailing slash on the ar names, which GNU ar writes.
set -e

HERE=$(cd "$(dirname "$0")/.." && pwd)
PKG=$HERE/meego
OUT=$HERE/build/meego
BIN=$OUT/arm/harbour-seaprint
XGCC=${XGCC:-/tmp/xgcc-harmattan}
VERSION=${VERSION:-$(sh "$PKG/version.sh")}

[ -x "$BIN" ] || { echo "ARM binary missing: $BIN (run meego/build.sh arm)" >&2; exit 1; }
[ -f "$PKG/qml/harbour-seaprint.qml" ] || { echo "QML missing (run meego/port-qml.sh)" >&2; exit 1; }

STAGE=$OUT/stage
rm -rf "$STAGE"
mkdir -p "$STAGE/DEBIAN" "$STAGE/opt/harbour-seaprint/bin" \
         "$STAGE/usr/share/applications" "$STAGE/usr/share/icons/hicolor/80x80/apps" \
         "$STAGE/usr/share/themes/base/meegotouch/icons" \
         "$STAGE/usr/share/doc/harbour-seaprint"

# --- program and data ------------------------------------------------------
cp "$BIN" "$STAGE/opt/harbour-seaprint/bin/harbour-seaprint"
"$XGCC/bin/arm-none-linux-gnueabi-strip" "$STAGE/opt/harbour-seaprint/bin/harbour-seaprint"
chmod 755 "$STAGE/opt/harbour-seaprint/bin/harbour-seaprint"

# The QML, without the pieces that are only there to make it: the hand-written
# originals port-qml.sh copies from, and the test stubs.
cp -a "$PKG/qml" "$STAGE/opt/harbour-seaprint/qml"
rm -rf "$STAGE/opt/harbour-seaprint/qml/hand"
cp -a "$OUT/arm/translations" "$STAGE/opt/harbour-seaprint/translations"

# --- icons: 80x80 for the launcher, 64x64 base64 for the package manager ---
# Both are cut to the exact silhouette of the stock apps -- rebuild them with
# meego/icons/make-icon.py, which takes the Sailfish artwork and the alpha
# channel of a stock icon; do not just rescale the Sailfish icon here.
cp "$PKG/icons/icon-80.png" "$STAGE/usr/share/icons/hicolor/80x80/apps/harbour-seaprint.png"
cp "$PKG/icons/icon-80.png" "$STAGE/usr/share/themes/base/meegotouch/icons/harbour-seaprint-80.png"

cp "$PKG/harbour-seaprint.desktop" "$STAGE/usr/share/applications/harbour-seaprint.desktop"
cp "$HERE/LICENSE" "$STAGE/usr/share/doc/harbour-seaprint/copyright"
gzip -9nc "$PKG/changelog" > "$STAGE/usr/share/doc/harbour-seaprint/changelog.gz"
find "$STAGE" -type f ! -path "*/bin/*" -exec chmod 644 {} +
find "$STAGE" -type d -exec chmod 755 {} +

# --- control from control.in -----------------------------------------------
# XB-Maemo-Icon-26 is the 64x64 PNG as base64, continuation lines indented by
# one space; without it the application manager shows no icon.
VERSION="$VERSION" ICON="$PKG/icons/icon-64.png" python3 - "$PKG/control.in" "$STAGE/DEBIAN/control" <<'PY'
import base64, os, sys, textwrap
src, dst = sys.argv[1], sys.argv[2]
with open(os.environ["ICON"], "rb") as f:
    b64 = base64.b64encode(f.read()).decode("ascii")
icon = "\n".join(" " + line for line in textwrap.wrap(b64, 76))
with open(src, "r", encoding="utf-8") as f:
    ctl = f.read()
ctl = ctl.replace("@VERSION@", os.environ["VERSION"]).replace("@ICON@", icon)
with open(dst, "w", encoding="utf-8") as f:
    f.write(ctl)
PY

DEB="$OUT/harbour-seaprint_${VERSION}_armel.deb"
python3 "$PKG/mkdeb.py" "$STAGE" "$DEB"
python3 "$PKG/mkdeb.py" --info "$DEB" | head -30
echo "== $DEB"
