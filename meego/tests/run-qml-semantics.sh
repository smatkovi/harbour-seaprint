#!/bin/sh
# Builds and runs meego/tests/qml_semantics.cpp against a Qt 4 of the SDK.
# No display is needed: the probe creates only a QtObject.
set -e
HERE=$(cd "$(dirname "$0")/../.." && pwd)
# The Qt Simulator's Qt aborts without an X server even for a QCoreApplication
# (it tries to reach the simulator process), so the probe is built against the
# SDK's plain desktop Qt. That is 4.8.1 rather than the device's 4.7.4 -- the
# QtDeclarative 1 semantics this asks about are the same in both, and the
# Desktop/Qt/474 directory of the SDK carries only themes and imports, no
# libraries to link against.
PROBEQT=${PROBEQT:-$HOME/QtSDK/Desktop/Qt/4.8.1/gcc}
OUT=$HERE/build/meego/tests
mkdir -p "$OUT"
"$PROBEQT/bin/moc" "$HERE/meego/tests/qml_semantics.cpp" -o "$OUT/qml_semantics.moc"
g++ -O0 -g -I"$OUT" -I"$PROBEQT/include" -I"$PROBEQT/include/QtCore" -I"$PROBEQT/include/QtGui" \
    -I"$PROBEQT/include/QtDeclarative" -I"$PROBEQT/include/QtScript" \
    "$HERE/meego/tests/qml_semantics.cpp" -o "$OUT/qml_semantics" \
    -L"$PROBEQT/lib" -Wl,-rpath,"$PROBEQT/lib" -lQtDeclarative -lQtScript -lQtGui -lQtCore
"$OUT/qml_semantics"
