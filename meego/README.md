# SeaPrint for MeeGo Harmattan (Nokia N9 / N950)

Upstream SeaPrint is a Sailfish OS app: Qt 5, QtQuick 2, Silica, mlite5,
Nemo. Harmattan has Qt 4.7.4, QtQuick 1.1 and `com.nokia.meego`. Everything
in this directory is what closes that gap; `src/`, `ppm2pwg/` and `qml/` stay
upstream's, with only the changes noted under "Changes outside meego/".

    meego/remote-build.sh          # sync to the build machine, build, package
    meego/remote-build.sh build    # only the ARM binary
    meego/remote-build.sh tools    # pdf2printable, ippposter, ipp_probe
    meego/tests/check-qml.sh       # load every QML file and report errors
    meego/tests/run-ipp-probe.sh   # talk IPP to a real printer, as ARM code
    meego/tests/run-qml-semantics.sh

The package is `harbour-seaprint_<version>_armel.deb`; copies land in
`~/ps/rpms/seaprint/`.

## How it is built

GCC 14.2 cross (`meego/toolchain.sh`, shared with the other Harmattan ports
in `~/ps`) against the MADDE sysroot: ppm2pwg is C++17 and MADDE's GCC 4.4
cannot compile it. libstdc++ and libgcc are linked statically and hidden with
`--exclude-libs,ALL`, so Qt, poppler and cairo on the device keep using their
own GCC 4.4 runtime. Harmattan is hard-float but kept the old loader name, so
the link says `--dynamic-linker=/lib/ld-linux.so.3`.

`meego/build.sh` writes its own Makefile instead of using qmake: qmake would
insist on MADDE's compiler.

## What the port had to answer

**Qt 5 classes Qt 4.7 has not got** — `meego/compat/` holds stand-ins found
first on the include path, so the shared sources keep their spelling:
`QJsonObject`/`Array`/`Value`/`Document` (backed by `QVariantMap`, with a
hand-written parser in `qt4json.cpp`), `QMimeDatabase` (suffix table plus
content sniffing), `QPdfWriter` and `QPageSize` (over `QPrinter`),
`QRegularExpression` (over `QRegExp`), `QStandardPaths`, `QLoggingCategory`,
`QQuickPaintedItem` and `QQuickView` (over `QDeclarativeItem`/`View`),
`MGConfItem` (over `QSettings`, keeping the GConf key strings so the QML can
name them).

**The JSON classes reach the QML.** `IppPrinter` exposed its attributes as
`QJsonObject`; QtDeclarative cannot look inside a custom type, so the
properties are read as `QVariantMap`/`QVariantList`, which both engines turn
into plain JavaScript objects. `moc` 4.7 also does not know `MEMBER`, so every
property got a getter.

**poppler, cairo and libjpeg are the device's own.** ppm2pwg `dlopen()`s them
(`MADNESS=1`), so nothing is linked and only the headers were needed:
`meego/compat/cairo` is cairo 1.10, the version the N9 carries, and
`meego/compat/poppler` declares the two opaque types rather than dragging in
poppler's generated glib headers. Three differences to the Sailfish build:
the library is `libpoppler-glib.so.6` (`POPPLER_GLIB_SO`),
`cairo_pdf_surface_set_metadata` does not exist before cairo 1.16, and
**glib 2.24 needs `g_type_init()`** before the first poppler call -- without
it the process dies in the type system with no message.

**QtQuick 1.1 has no `property var`.** A `property variant` converts what is
put into it and hands out a copy, so `jobParams[name] = ...` wrote into a
throwaway. `meego/qml/hand/SettingsColumn.qml` takes a copy, changes it and
assigns it back; `DependentOn.qml` builds its array before assigning.
`meego/tests/qml_semantics.cpp` is the probe that established this (and that
a `QVariantMap` does convert recursively, which is why the reading side of
the pages is untouched).

**Silica.** `meego/port-qml.sh` + `fix-qml.py` convert `qml/` mechanically
(imports, `Theme.` to `AppTheme.`, orientations, `SilicaFlickable`,
`pressDelay`, paddings); the Silica types with no MeeGo counterpart are
written out in `meego/qml/silica/`, imported as a directory *after*
`com.nokia.meego` so they win where both define a name. Files that need
judgement rather than substitution live in `meego/qml/hand/`. The theme
object is called `AppTheme`, not `Theme`: `com.nokia.meego` exports a `Theme`
of its own that beats a context property of that name.

**Things Harmattan does differently.**

| Sailfish | here |
| --- | --- |
| `Sailfish.Pickers` | own file browser (`FilePickerPage.qml`, `filebrowser.cpp`) -- the N9 indexes pictures and music, not documents |
| `Nemo.DBus` + connman for the WLAN name | `wifichecker.cpp`, wireless extensions on the kernel |
| `Nemo.Notifications` | `InfoBanner` |
| `Nemo.Configuration` | the same QML type name, over `QSettings` |
| `PullDownMenu` | a menu button at the top right, as in the other ports here |
| Silica icon names | `tintedimage.cpp` maps them to blanco's and tints them with `QPainter` (QtQuick 1.1 has no shader effects) |
| a slider inside a `MenuItem` | the number dialog upstream already has |
| cover page | nothing; Harmattan has none |
| calligraconverter | nothing; no office formats on this device |
| `QImageReader::setAutoTransform` | `compat/exif.cpp`, so photos are not printed sideways |

## What is checked, and what is not

Verified on the build machine: the QML (all 49 files load, against stand-ins
for `com.nokia.meego`), PDF conversion to PWG, URF, PDF and PostScript with
the device's own poppler and cairo under `qemu-arm`, and a full IPP
Get-Printer-Attributes against CUPS's `ippeveprinter` as an ARM binary --
106 attributes, nested collections and all, through the JSON stand-ins.

Not verified: the app running on a device. The N9 and N950 were unreachable
while this was written. What that leaves open is the look of the pages
against the real `com.nokia.meego` components and anything that needs a
printer on the same network.

IPPS (TLS) will likely not work: the N9's curl is 7.21 against OpenSSL 0.9.8,
so a modern printer's TLS will be refused. Plain IPP on port 631 is what to
use.

## Changes outside meego/

Small and Qt-neutral, so the Sailfish build keeps working: getters instead of
`MEMBER` properties, `SIGNAL`/`SLOT` connects, `QStringList() << ...` instead
of braced lists, `QProcess::start(program, args)`, the `g_type_init()` and
cairo-version guards, `POPPLER_GLIB_SO`, and the EXIF helper behind a
`QT_VERSION` test.
