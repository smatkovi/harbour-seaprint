#!/usr/bin/env python3
"""Sailfish QML -> QtQuick 1.1 + com.nokia.meego.

Called by meego/port-qml.sh; every rule here is one thing QtQuick 1.1 or
Harmattan spells differently. Anything that needs judgement rather than a
substitution lives in meego/qml/hand/ instead.
"""
import os
import re
import sys

# Modules that do not exist on Harmattan. The singletons among them
# (Mimer, IppDiscovery, ConvertChecker, SeaPrintSettings, RangeListChecker)
# are root context properties in meego/main.cpp, which the QML reads by the
# same names, so dropping the import is all that is needed.
DROP_IMPORTS = [
    r'^import Sailfish\.Pickers 1\.0\s*$',
    r'^import Nemo\.Notifications 1\.0\s*$',
    r'^import Nemo\.DBus 2\.0\s*$',
    r'^import QtQuick\.LocalStorage 2\.0\s*$',   # QML 1 has openDatabaseSync built in
    r'^import QtDocGallery 5\.0\s*$',
    r'^import seaprint\.mimer 1\.0\s*$',
    r'^import seaprint\.settings 1\.0\s*$',
    r'^import seaprint\.ippdiscovery 1\.0\s*$',
    r'^import seaprint\.convertchecker 1\.0\s*$',
    r'^import seaprint\.rangelistchecker 1\.0\s*$',
]


# Whole blocks that have no mechanical answer. Each is matched against the
# upstream text exactly, so that a change upstream is noticed here rather than
# silently dropped.
FILE_PATCHES = {
    "pages/FirstPage.qml": [
        # Sailfish.Pickers' two picker pages. Harmattan has no such module, and
        # asking the gallery instead would find no PDFs: on the N9 tracker
        # indexes pictures and music, not documents. meego/filebrowser.cpp
        # walks the files, silica/FilePickerPage.qml shows them.
        ("""        Component {
            id: documentPickerPage
            DocumentPickerPage {
                allowedOrientations: Orientation.All

                Component.onCompleted: {
                    var thingy = Qt.createComponent(ConvertChecker.calligra ? "../components/DocumentFilterOffice.notqml" : "../components/DocumentFilter.notqml");
                    if (thingy.status == Component.Ready) {
                        _contentModel.contentFilter = thingy.createObject(this);
                    }
                }

                title: qsTr("Choose file")

                onSelectedContentPropertiesChanged: {
                    appWin.openFile(selectedContentProperties.filePath);
                }
            }
        }
        Component {
            id: imagePickerPage
            ImagePickerPage {
                allowedOrientations: Orientation.All

                onSelectedContentPropertiesChanged: {
                    appWin.openFile(selectedContentProperties.filePath);
                }
            }
        }""",
         """        Component {
            id: documentPickerPage
            FilePickerPage {
                title: qsTr("Choose file")
                // The formats this build can print; there is no
                // calligraconverter on Harmattan, so no office documents.
                suffixes: ["pdf", "ps", "txt", "text", "log", "md", "csv"]
                onSelected: appWin.openFile(path)
            }
        }
        Component {
            id: imagePickerPage
            FilePickerPage {
                title: qsTr("Choose image")
                suffixes: ["jpg", "jpeg", "png", "gif", "bmp", "tif", "tiff", "svg"]
                onSelected: appWin.openFile(path)
            }
        }"""),
    ],
}

SUBST = [
    # --- language -----------------------------------------------------------
    (r'^import QtQuick 2\.[0-9]+\s*$', 'import QtQuick 1.1'),
    (r'\breadonly property ', 'property '),
    (r'\bproperty var\b', 'property variant'),

    # --- the theme ----------------------------------------------------------
    # com.nokia.meego exports a Theme of its own, which beats a context
    # property of the same name; the port's theme object is called AppTheme.
    (r'\bTheme\.', 'AppTheme.'),
    (r'\bScreen\.width\b', 'AppTheme.screenWidth'),
    (r'\bScreen\.height\b', 'AppTheme.screenHeight'),

    # --- Silica types with a plain counterpart ------------------------------
    (r'\bSilicaFlickable\b', 'Flickable'),
    (r'\bSilicaListView\b', 'ListView'),
    (r'\bSilicaGridView\b', 'GridView'),

    # --- pages --------------------------------------------------------------
    # A Silica page lists the orientations it allows; a MeeGo page locks to one
    # or follows the device.
    (r'^\s*allowedOrientations:\s*Orientation\.All\s*$', '    orientationLock: PageOrientation.Automatic'),
    (r'^\s*allowedOrientations:\s*Orientation\.Portrait[^\n]*$', '    orientationLock: PageOrientation.LockPortrait'),
    (r'^\s*allowedOrientations:\s*Orientation\.Landscape[^\n]*$', '    orientationLock: PageOrientation.LockLandscape'),
    (r'^\s*allowedOrientations:\s*defaultAllowedOrientations\s*$\n', ''),
    (r'^\s*defaultAllowedOrientations:[^\n]*$\n', ''),
    # PageStack takes a bool for "do not animate".
    (r'\bPageStackAction\.Immediate\b', 'true'),
    (r'\bPageStackAction\.Animated\b', 'false'),
    # Silica pages know their orientation; here the shape of the page says it.
    (r'\bpage\.isPortrait\b', '(page.height > page.width)'),
    (r'\bisPortrait\b(?!:)', '(height > width)'),

    # --- text ---------------------------------------------------------------
    # Silica fades a truncated line, a QtQuick 1.1 Text elides.
    (r'^(\s*)truncationMode:\s*TruncationMode\.\w+\s*$', r'\1elide: Text.ElideRight'),
    # Padding on a Text or a Column is QtQuick 2.
    (r'^(\s*)leftPadding:\s*([^\n]+)$', r'\1anchors.leftMargin: \2'),
    (r'^(\s*)rightPadding:\s*([^\n]+)$', r'\1anchors.rightMargin: \2'),
    (r'^(\s*)topPadding:\s*([^\n]+)$', r'\1anchors.topMargin: \2'),
    (r'^(\s*)bottomPadding:\s*([^\n]+)$', r'\1anchors.bottomMargin: \2'),

    # --- what a QtQuick 1.1 Flickable needs to scroll at all ----------------
    # A child MouseArea takes the press at once, so the Flickable never gets to
    # flick; pressDelay lets it steal the press back, which is what
    # SilicaFlickable does for itself.
    (r'^(\s*)(Flickable|ListView|GridView) \{\s*$\n(\s*)(anchors\.fill: parent)',
     r'\1\2 {\n\3\4\n\3pressDelay: 150'),

    # --- icons --------------------------------------------------------------
    # IconButton takes the source directly here; icon.source is a Silica
    # grouped property.
    (r'\bicon\.source:', 'source:'),
]

# The BusyPage and others call this on Qt.application, which has no state in
# Qt 4.7.
TOLERANT_CONNECTIONS = (
    re.compile(r'^(\s*)(target:\s*Qt\.application\s*)$', re.M),
    r'\1\2\n\1ignoreUnknownSignals: true',
)


def silica_import(relative_depth):
    """The directory import that carries the Silica stand-ins."""
    prefix = "../" * relative_depth
    return 'import com.nokia.meego 1.0\nimport "%ssilica"' % prefix


def convert(text, depth, name=""):
    for old, new in FILE_PATCHES.get(name, []):
        if old not in text:
            raise SystemExit("meego/fix-qml.py: the block it patches in %s is not "
                             "there any more; check what changed upstream" % name)
        text = text.replace(old, new)

    for pattern in DROP_IMPORTS:
        text = re.sub(pattern + r'\n', '', text, flags=re.M)

    # Silica's import becomes two: the MeeGo components, then the stand-ins,
    # which have to come last so that they win where both define a name.
    text = re.sub(r'^import Sailfish\.Silica 1\.0\s*$', silica_import(depth), text, flags=re.M)

    for pattern, replacement in SUBST:
        text = re.sub(pattern, replacement, text, flags=re.M)

    text = TOLERANT_CONNECTIONS[0].sub(TOLERANT_CONNECTIONS[1], text)
    return text


def main():
    src, dst = sys.argv[1], sys.argv[2]

    files = [("harbour-seaprint.qml", 0)]
    for name in sorted(os.listdir(os.path.join(src, "pages"))):
        if name.endswith(".qml"):
            files.append(("pages/" + name, 1))
    for name in sorted(os.listdir(os.path.join(src, "components"))):
        if name.endswith(".qml"):
            files.append(("components/" + name, 1))

    for name, depth in files:
        with open(os.path.join(src, name)) as f:
            text = f.read()
        out = os.path.join(dst, name)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        with open(out, "w") as f:
            f.write(convert(text, depth, name))

    # The JavaScript is shared as it stands; QtScript in Qt 4.7 is ES5.
    for name in sorted(os.listdir(os.path.join(src, "pages"))):
        if name.endswith(".js"):
            with open(os.path.join(src, "pages", name)) as f:
                text = f.read()
            with open(os.path.join(dst, "pages", name), "w") as f:
                f.write(text)

    print("%d QML files converted" % len(files))


if __name__ == "__main__":
    main()
