#!/bin/sh
# Turns the Sailfish QML in qml/ into QtQuick 1.1 + com.nokia.meego under
# meego/qml/. qml/ is copied, never edited: the Sailfish build keeps QtQuick
# 2.6 and Silica.
#
# What changes mechanically is in meego/fix-qml.py, which this calls; the parts
# that have no mechanical answer are kept as whole files in meego/qml/hand/ and
# copied over the generated ones afterwards:
#
#   harbour-seaprint.qml  ApplicationWindow -> PageStackWindow, and the file
#                         pickers, notifications and database that came out of
#                         Sailfish and Nemo modules
#   FirstPage.qml         the two picker pages
#   IntegerSetting.qml    a slider inside a MenuItem, which a MeeGo Menu is not
#   RangeSetting.qml      able to show
#
# The Silica types that have no counterpart are written out as components in
# meego/qml/silica/ and imported as a directory after com.nokia.meego, so that
# they win over the MeeGo types of the same name and the pages keep their
# spelling.
set -e
cd "$(dirname "$0")/.."
OUT=meego/qml

mkdir -p "$OUT/pages" "$OUT/components"

python3 meego/fix-qml.py qml "$OUT"

# The hand-written files replace what the transform cannot do.
for f in meego/qml/hand/*.qml; do
    name=$(basename "$f")
    case "$name" in
        harbour-seaprint.qml) cp "$f" "$OUT/$name" ;;
        FirstPage.qml|PrinterPage.qml) cp "$f" "$OUT/pages/$name" ;;
        *) cp "$f" "$OUT/components/$name" ;;
    esac
done

echo "== $(ls qml/*.qml qml/pages/*.qml qml/components/*.qml | wc -l) files -> $OUT"
