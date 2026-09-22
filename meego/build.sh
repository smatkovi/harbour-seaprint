#!/bin/sh
# Builds the MeeGo Harmattan (Nokia N9 / N950) edition of SeaPrint. Runs on the
# build machine, inside the synced source tree:
#
#   meego/build.sh arm     N9 binary      -> build/meego/arm/harbour-seaprint
#   meego/build.sh x86     the same code against the Qt Simulator's Qt 4.7.4,
#                          to run and look at it on the desktop
#   meego/build.sh tools   pdf2printable and ippposter as ARM binaries, for the
#                          conversion and IPP tests under qemu-arm
#   meego/build.sh check   the QML checker (meego/tests/qml_check.cpp) against
#                          the SDK's desktop Qt 4; meego/tests/check-qml.sh
#                          runs it
#
# The ARM build needs the cross toolchain from meego/toolchain.sh (XGCC, GCC 14
# for the C++17 in ppm2pwg) and the MADDE sysroot (SYSROOT); moc and lrelease
# come from the Qt Simulator's Qt, which is the same Qt 4.7.4 as on the device.
#
# poppler, cairo and libjpeg are never linked: ppm2pwg dlopen()s them, so the
# build only needs their headers (meego/compat/cairo, meego/compat/poppler) and
# the device supplies the libraries it has -- libpoppler-glib.so.6 rather than
# Sailfish's .so.8, which is what POPPLER_GLIB_SO is for.
set -e

HERE=$(cd "$(dirname "$0")/.." && pwd)
MODE=${1:-arm}
XGCC=${XGCC:-/tmp/xgcc-harmattan}
SYSROOT=${SYSROOT:-$HOME/QtSDK/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim}
SIMQT=${SIMQT:-$HOME/QtSDK/Simulator/Qt/gcc}
JOBS=${JOBS:-8}
VERSION=${VERSION:-$(sh "$HERE/meego/version.sh")}
OUT=$HERE/build/meego/$MODE
mkdir -p "$OUT"

# --- sources ---------------------------------------------------------------
# Upstream's src/harbour-seaprint.cpp is replaced by meego/main.cpp; everything
# else in src/ and ppm2pwg/ is built as it stands.
APP_SRC="src/dbusadaptorbase.cpp \
 src/freedesktopdbusadaptor.cpp \
 src/rangelistchecker.cpp \
 src/convertchecker.cpp \
 src/imageitem.cpp \
 src/ippdiscovery.cpp \
 src/ippmsg.cpp \
 src/ippprinter.cpp \
 src/mimer.cpp \
 src/overrider.cpp \
 src/printerworker.cpp \
 src/seaprintdbusadaptor.cpp \
 src/settings.cpp \
 ppm2pwg/lib/printparameters.cpp \
 ppm2pwg/lib/ppm2pwg.cpp \
 ppm2pwg/lib/pdf2printable.cpp \
 ppm2pwg/lib/baselinify.cpp \
 ppm2pwg/lib/curlrequester.cpp \
 ppm2pwg/bytestream/bytestream.cpp \
 meego/main.cpp \
 meego/wifichecker.cpp \
 meego/tintedimage.cpp \
 meego/themeicons.cpp \
 meego/filebrowser.cpp \
 meego/clipboard.cpp \
 meego/compat/qt4json.cpp \
 meego/compat/qt4mime.cpp \
 meego/compat/qt4logging.cpp \
 meego/compat/exif.cpp \
 meego/compat/mgconfitem.cpp"

# Headers with a Q_OBJECT in them.
MOC_HEADERS="src/convertchecker.h \
 src/freedesktopdbusadaptor.h \
 src/imageitem.h \
 src/ippdiscovery.h \
 src/ippprinter.h \
 src/mimer.h \
 src/printerworker.h \
 src/rangelistchecker.h \
 src/seaprintdbusadaptor.h \
 src/settings.h \
 meego/wifichecker.h \
 meego/tintedimage.h \
 meego/filebrowser.h \
 meego/clipboard.h \
 meego/ipptags.h \
 meego/compat/mgconfitem.h"

TOOLS_SRC="ppm2pwg/lib/printparameters.cpp ppm2pwg/lib/ppm2pwg.cpp \
 ppm2pwg/lib/pdf2printable.cpp ppm2pwg/lib/baselinify.cpp \
 ppm2pwg/lib/curlrequester.cpp ppm2pwg/bytestream/bytestream.cpp"

INCLUDES="-I$HERE -I$HERE/src -I$HERE/ppm2pwg/lib -I$HERE/ppm2pwg/bytestream \
 -I$HERE/meego -I$HERE/meego/compat -I$HERE/meego/compat/cairo -I$HERE/meego/compat/poppler"

DEFINES="-DSEAPRINT_VERSION='\"$VERSION\"' -DPDF_CREATOR='\"SeaPrint $VERSION\"' -DMADNESS=1 -DQT_NO_DEBUG"

# -Wno-register and friends: Qt 4.7 headers and libjpeg 6b are older than the
# language the rest is compiled as.
COMMON_FLAGS="-std=gnu++17 -O2 -Wall -Wno-register -Wno-deprecated-declarations \
 -Wno-unused-parameter -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS \
 $DEFINES $INCLUDES -include $HERE/meego/compat/qt4compat.h"

QT4_MODULES="QtCore QtGui QtNetwork QtDeclarative QtDBus QtSvg"

case "$MODE" in
arm|tools)
    CXX=$XGCC/bin/arm-none-linux-gnueabi-g++
    [ -x "$CXX" ] || { echo "cross compiler missing: $CXX (run meego/toolchain.sh)" >&2; exit 1; }
    MOC=$SIMQT/bin/moc
    QTINC=$SYSROOT/usr/include/qt4
    GLIBINC="-I$SYSROOT/usr/include/glib-2.0 -I$SYSROOT/usr/lib/glib-2.0/include"
    CXXFLAGS="--sysroot=$SYSROOT $COMMON_FLAGS $GLIBINC -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    # Harmattan is hard-float but kept the old loader name; and the static
    # libstdc++ of GCC 14 stays private to the binary so that Qt, poppler and
    # cairo on the device keep using their own (GCC 4.4) runtime.
    LDFLAGS="--sysroot=$SYSROOT -static-libstdc++ -static-libgcc -Wl,-O1 -Wl,--as-needed \
 -Wl,--exclude-libs,ALL -Wl,--dynamic-linker=/lib/ld-linux.so.3"
    LIBS="-lQtDeclarative -lQtDBus -lQtSvg -lQtGui -lQtNetwork -lQtCore -lcurl -lglib-2.0 -lgobject-2.0 -ldl -lpthread"
    ;;
x86)
    CXX=${CXX:-g++}
    MOC=$SIMQT/bin/moc
    QTINC=$SIMQT/include
    CXXFLAGS="$COMMON_FLAGS $(pkg-config --cflags glib-2.0) -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="-L$SIMQT/lib -Wl,-rpath,$SIMQT/lib"
    LIBS="-lQtDeclarative -lQtDBus -lQtSvg -lQtGui -lQtNetwork -lQtCore -lcurl -lglib-2.0 -lgobject-2.0 -ldl -lpthread"
    ;;
guiprobe)
    # A small GUI test for the device: it needs X, which the N9 has and the
    # build machine has not, so it is only cross-compiled here.
    CXX=$XGCC/bin/arm-none-linux-gnueabi-g++
    MOC=$SIMQT/bin/moc
    QTINC=$SYSROOT/usr/include/qt4
    # No DEFINES here: this one is compiled straight, not through a Makefile,
    # and the version strings in them carry quotes the shell would eat.
    CXXFLAGS="--sysroot=$SYSROOT -std=gnu++17 -O2 -Wno-register -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="--sysroot=$SYSROOT -static-libstdc++ -static-libgcc -Wl,-O1 \
 -Wl,--exclude-libs,ALL -Wl,--dynamic-linker=/lib/ld-linux.so.3"
    LIBS="-lQtDeclarative -lQtGui -lQtCore -lpthread"
    mkdir -p "$OUT"
    $MOC "$HERE/meego/tests/model_probe.cpp" -o "$OUT/model_probe.moc"
    $CXX $CXXFLAGS -I"$OUT" $LDFLAGS -o "$OUT/model_probe" "$HERE/meego/tests/model_probe.cpp" $LIBS
    echo "== built $OUT/model_probe (run it on the device with DISPLAY=:0)"
    exit 0
    ;;
probe)
    # The discovery probe is the app without its user interface: same objects,
    # meego/tests/discovery_probe.cpp in place of main.cpp. It runs on the
    # device, where mDNS can actually be tried.
    CXX=$XGCC/bin/arm-none-linux-gnueabi-g++
    [ -x "$CXX" ] || { echo "cross compiler missing: $CXX" >&2; exit 1; }
    MOC=$SIMQT/bin/moc
    QTINC=$SYSROOT/usr/include/qt4
    GLIBINC="-I$SYSROOT/usr/include/glib-2.0 -I$SYSROOT/usr/lib/glib-2.0/include"
    CXXFLAGS="--sysroot=$SYSROOT $COMMON_FLAGS $GLIBINC -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="--sysroot=$SYSROOT -static-libstdc++ -static-libgcc -Wl,-O1 -Wl,--as-needed \
 -Wl,--exclude-libs,ALL -Wl,--dynamic-linker=/lib/ld-linux.so.3"
    LIBS="-lQtDeclarative -lQtDBus -lQtSvg -lQtGui -lQtNetwork -lQtCore -lcurl -lglib-2.0 -lgobject-2.0 -ldl -lpthread"
    APP_SRC=$(echo "$APP_SRC" | sed 's| meego/main.cpp | meego/tests/discovery_probe.cpp |')
    ;;
check)
    # The QML checker replaces meego/main.cpp and is built against the SDK's
    # plain desktop Qt (4.8.1): the Simulator's Qt 4.7.4 aborts without a
    # display even for a non-GUI application, and the device's Qt cannot run
    # here at all. QtDeclarative 1 parses the same either way, which is what
    # the check is about.
    CXX=${CXX:-g++}
    PROBEQT=${PROBEQT:-$HOME/QtSDK/Desktop/Qt/4.8.1/gcc}
    MOC=$PROBEQT/bin/moc
    QTINC=$PROBEQT/include
    APP_SRC=$(echo "$APP_SRC" | sed 's| meego/main.cpp | meego/tests/qml_check.cpp |')
    CXXFLAGS="$COMMON_FLAGS $(pkg-config --cflags glib-2.0) -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="-L$PROBEQT/lib -Wl,-rpath,$PROBEQT/lib"
    LIBS="-lQtDeclarative -lQtDBus -lQtSvg -lQtGui -lQtNetwork -lQtCore -lcurl -lglib-2.0 -lgobject-2.0 -ldl -lpthread"
    ;;
*)
    echo "usage: $0 arm|x86|tools|check|probe" >&2; exit 2 ;;
esac

# --- Makefile --------------------------------------------------------------
MK=$OUT/Makefile
{
    echo "CXX=$CXX"
    echo "MOC=$MOC"
    echo "CXXFLAGS=$CXXFLAGS"
    echo "LDFLAGS=$LDFLAGS"
    echo "LIBS=$LIBS"
    echo "SRC=$HERE"
    echo

    if [ "$MODE" = tools ]; then
        objs=
        for s in $TOOLS_SRC; do
            o=$(basename "$s" .cpp).o; objs="$objs $o"
            echo "$o: \$(SRC)/$s"; printf '\t$(CXX) $(CXXFLAGS) -I. -c $< -o $@\n'
        done
        echo "OBJS=$objs"
        echo "all: pdf2printable ippposter ipp_probe qml_semantics"
        echo "pdf2printable: \$(SRC)/ppm2pwg/utils/pdf2printable_main.cpp \$(OBJS)"
        printf '\t$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ -lcurl -lglib-2.0 -lgobject-2.0 -ldl -lpthread\n'
        echo "ippposter: \$(SRC)/ppm2pwg/utils/ippposter.cpp \$(OBJS)"
        printf '\t$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ -lcurl -lglib-2.0 -lgobject-2.0 -ldl -lpthread\n'
        # The IPP probe needs the app's own IppMsg and the JSON stand-ins.
        echo "ippmsg.o: \$(SRC)/src/ippmsg.cpp"; printf '\t$(CXX) $(CXXFLAGS) -c $< -o $@\n'
        echo "qt4json.o: \$(SRC)/meego/compat/qt4json.cpp"; printf '\t$(CXX) $(CXXFLAGS) -c $< -o $@\n'
        echo "ipp_probe: \$(SRC)/meego/tests/ipp_probe.cpp \$(OBJS) ippmsg.o qt4json.o"
        printf '\t$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ -lQtCore -lcurl -lglib-2.0 -lgobject-2.0 -ldl -lpthread\n'
        # The QML semantics probe, against the device's own Qt 4.7.4 under
        # qemu-arm: Qt 4.8 answers some of its questions differently, because
        # its QtDeclarative runs JavaScript on V8 rather than on JSC.
        echo "qml_semantics.moc: \$(SRC)/meego/tests/qml_semantics.cpp"; printf '\t$(MOC) $< -o $@\n'
        echo "qml_semantics: \$(SRC)/meego/tests/qml_semantics.cpp qml_semantics.moc"
        printf '\t$(CXX) $(CXXFLAGS) -I. $(LDFLAGS) -o $@ $< -lQtDeclarative -lQtScript -lQtGui -lQtNetwork -lQtCore -lpthread\n'
    else
        objs=
        for s in $APP_SRC; do
            o=$(echo "$s" | sed 's|/|_|g; s|\.cpp$|.o|'); objs="$objs $o"
            echo "$o: \$(SRC)/$s"; printf '\t$(CXX) $(CXXFLAGS) -I. -c $< -o $@\n'
        done
        if [ "$MODE" = probe ]; then
            echo "discovery_probe.moc: \$(SRC)/meego/tests/discovery_probe.cpp"
            printf '\t$(MOC) $< -o $@\n'
            echo "meego_tests_discovery_probe.o: discovery_probe.moc"
        fi
        for h in $MOC_HEADERS; do
            n=$(basename "$h" .h); objs="$objs moc_$n.o"
            echo "moc_$n.cpp: \$(SRC)/$h"; printf '\t$(MOC) $< -o $@\n'
            echo "moc_$n.o: moc_$n.cpp"; printf '\t$(CXX) $(CXXFLAGS) -c $< -o $@\n'
        done
        echo "OBJS=$objs"
        echo "all: harbour-seaprint"
        echo "harbour-seaprint: \$(OBJS)"
        printf '\t$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)\n'
    fi
} > "$MK"

nice make -C "$OUT" -j"$JOBS" all

case "$MODE" in
arm|x86)
    # Qt 4.7's lrelease only knows TS version 2.0; the catalogues say 2.1.
    LRELEASE=$SIMQT/bin/lrelease
    mkdir -p "$OUT/translations"
    for ts in "$HERE"/translations/harbour-seaprint-*.ts; do
        name=$(basename "$ts" .ts)
        sed 's/<TS version="2\.1"/<TS version="2.0"/' "$ts" > "$OUT/$name.ts"
        "$LRELEASE" -silent "$OUT/$name.ts" -qm "$OUT/translations/$name.qm" || true
    done
    echo "== built $OUT/harbour-seaprint"
    ;;
tools)
    echo "== built $OUT/pdf2printable, $OUT/ippposter and $OUT/ipp_probe (run under qemu-arm -L \$SYSROOT)"
    ;;
guiprobe)
    # A small GUI test for the device: it needs X, which the N9 has and the
    # build machine has not, so it is only cross-compiled here.
    CXX=$XGCC/bin/arm-none-linux-gnueabi-g++
    MOC=$SIMQT/bin/moc
    QTINC=$SYSROOT/usr/include/qt4
    # No DEFINES here: this one is compiled straight, not through a Makefile,
    # and the version strings in them carry quotes the shell would eat.
    CXXFLAGS="--sysroot=$SYSROOT -std=gnu++17 -O2 -Wno-register -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="--sysroot=$SYSROOT -static-libstdc++ -static-libgcc -Wl,-O1 \
 -Wl,--exclude-libs,ALL -Wl,--dynamic-linker=/lib/ld-linux.so.3"
    LIBS="-lQtDeclarative -lQtGui -lQtCore -lpthread"
    mkdir -p "$OUT"
    $MOC "$HERE/meego/tests/model_probe.cpp" -o "$OUT/model_probe.moc"
    $CXX $CXXFLAGS -I"$OUT" $LDFLAGS -o "$OUT/model_probe" "$HERE/meego/tests/model_probe.cpp" $LIBS
    echo "== built $OUT/model_probe (run it on the device with DISPLAY=:0)"
    exit 0
    ;;
probe)
    # The discovery probe is the app without its user interface: same objects,
    # meego/tests/discovery_probe.cpp in place of main.cpp. It runs on the
    # device, where mDNS can actually be tried.
    CXX=$XGCC/bin/arm-none-linux-gnueabi-g++
    [ -x "$CXX" ] || { echo "cross compiler missing: $CXX" >&2; exit 1; }
    MOC=$SIMQT/bin/moc
    QTINC=$SYSROOT/usr/include/qt4
    GLIBINC="-I$SYSROOT/usr/include/glib-2.0 -I$SYSROOT/usr/lib/glib-2.0/include"
    CXXFLAGS="--sysroot=$SYSROOT $COMMON_FLAGS $GLIBINC -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="--sysroot=$SYSROOT -static-libstdc++ -static-libgcc -Wl,-O1 -Wl,--as-needed \
 -Wl,--exclude-libs,ALL -Wl,--dynamic-linker=/lib/ld-linux.so.3"
    LIBS="-lQtDeclarative -lQtDBus -lQtSvg -lQtGui -lQtNetwork -lQtCore -lcurl -lglib-2.0 -lgobject-2.0 -ldl -lpthread"
    APP_SRC=$(echo "$APP_SRC" | sed 's| meego/main.cpp | meego/tests/discovery_probe.cpp |')
    ;;
check)
    echo "== built $OUT/harbour-seaprint (the QML checker)"
    ;;
guiprobe)
    # A small GUI test for the device: it needs X, which the N9 has and the
    # build machine has not, so it is only cross-compiled here.
    CXX=$XGCC/bin/arm-none-linux-gnueabi-g++
    MOC=$SIMQT/bin/moc
    QTINC=$SYSROOT/usr/include/qt4
    # No DEFINES here: this one is compiled straight, not through a Makefile,
    # and the version strings in them carry quotes the shell would eat.
    CXXFLAGS="--sysroot=$SYSROOT -std=gnu++17 -O2 -Wno-register -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="--sysroot=$SYSROOT -static-libstdc++ -static-libgcc -Wl,-O1 \
 -Wl,--exclude-libs,ALL -Wl,--dynamic-linker=/lib/ld-linux.so.3"
    LIBS="-lQtDeclarative -lQtGui -lQtCore -lpthread"
    mkdir -p "$OUT"
    $MOC "$HERE/meego/tests/model_probe.cpp" -o "$OUT/model_probe.moc"
    $CXX $CXXFLAGS -I"$OUT" $LDFLAGS -o "$OUT/model_probe" "$HERE/meego/tests/model_probe.cpp" $LIBS
    echo "== built $OUT/model_probe (run it on the device with DISPLAY=:0)"
    exit 0
    ;;
probe)
    echo "== built $OUT/harbour-seaprint (the discovery probe, for the device)"
    ;;
esac
