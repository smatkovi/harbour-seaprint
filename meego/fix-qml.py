#!/usr/bin/env python3
"""Sailfish QML -> QtQuick 1.1 + com.nokia.meego.

Called by meego/port-qml.sh; every rule here is one thing QtQuick 1.1 or
Harmattan spells differently. Anything that needs judgement rather than a
substitution lives in meego/qml/hand/ instead.
"""
import os
import re
import shutil
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
    "components/RangeListInputDialog.qml": [
        # Object.keys() of an undefined value throws, and the value is
        # undefined until something has been typed. On Sailfish the throw is
        # swallowed with the binding; here it is one more line in the log
        # every keystroke, and the Accept button never became usable.
        ("            acceptableInput: Object.keys(value).length !== 0",
         "            acceptableInput: value !== undefined && value.length !== 0"),
    ],
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
    # A Silica page can refuse the back gesture; a MeeGo page has no such
    # property -- navigation is the toolbar's, and this port shows none, so
    # the busy page cannot be left by mistake either way.
    (r'^\s*backNavigation:[^\n]*$\n', ''),
    # QtQuick 1.1's PageStack resolves a relative URL against its own
    # directory, not against the file that asked: com.nokia.meego's
    # PageStack.js calls Qt.createComponent(page) itself. Silica resolves it
    # against the caller, so upstream can pass a bare name.
    (r'pageStack\.(push|replace)\("([^"]+\.qml)"', r'pageStack.\1(Qt.resolvedUrl("\2")'),
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

    # Qt 4.7's Qt.application knows only `active` and `layoutDirection`; the
    # version is a root context property from meego/main.cpp.
    (r'\bQt\.application\.version\b', 'appVersion'),

    # (the theme icon names are rewritten in convert(), from the table in
    #  meego/themeicons.cpp)

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


def theme_icons():
    """The Silica icon names and what Harmattan calls them.

    Read out of meego/themeicons.cpp, which the C++ side uses, so that a name
    cannot be mapped one way in QML and another way in the image item. The
    names are rewritten here rather than served by an image provider of our
    own: a provider registered for "theme" replaces the platform's, and the
    MeeGo components then lose their own graphics.
    """
    with open(os.path.join("meego", "themeicons.cpp")) as f:
        table = re.findall(r'\{"([a-z0-9-]+)",\s*"([a-z0-9-]+)"\}', f.read())
    if len(table) < 10:
        raise SystemExit("meego/fix-qml.py: the icon table in meego/themeicons.cpp "
                         "could not be read")
    return table


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

    for silica, blanco in theme_icons():
        text = text.replace("image://theme/" + silica + '"', "image://theme/" + blanco + '"')

    text = TOLERANT_CONNECTIONS[0].sub(TOLERANT_CONNECTIONS[1], text)
    return text



def fix_utils(text, src):
    """utils.js, which upstream feeds the singletons through a QML import.

    Qt 4.7 accepts `.import` only in a `.pragma library` file, and such a file
    has no QML context -- so neither the modules (which this port does not
    have; the singletons are context properties) nor a context property can be
    reached from it. What it actually takes from Mimer is a handful of constant
    strings and two one-line tests, so those are written into the file. They
    are read out of src/mimer.cpp rather than typed here, so that they cannot
    drift away from what the C++ answers.
    """
    mimer = os.path.join(os.path.dirname(src.rstrip("/")), "src", "mimer.cpp")
    if not os.path.exists(mimer):
        mimer = os.path.join("src", "mimer.cpp")
    with open(mimer) as f:
        cpp = f.read()

    constants = re.findall(r'const QString Mimer::(\w+)\s*=\s*"([^"]*)"', cpp)
    if len(constants) < 15:
        raise SystemExit("meego/fix-qml.py: could not read the MIME types out of "
                         "src/mimer.cpp; check what changed upstream")
    names = dict(constants)

    office = re.search(r'const QStringList Mimer::OfficeFormats = ([^;]+);', cpp)
    if not office:
        raise SystemExit("meego/fix-qml.py: OfficeFormats not found in src/mimer.cpp")
    office_names = re.findall(r'\b([A-Z][A-Z_]+)\b', office.group(1))
    raster = re.search(r'const QStringList Mimer::RasterFormats = ([^;]+);', cpp)
    raster_names = re.findall(r'\b([A-Z][A-Z_]+)\b', raster.group(1)) if raster else ["PWG", "URF"]

    lines = ['// Written by meego/fix-qml.py out of src/mimer.cpp: Qt 4.7 only allows',
             '// .import in a .pragma library file, and such a file cannot see the',
             '// singletons this used to import.',
             'var Mimer = {']
    for name, value in constants:
        lines.append('    %s: "%s",' % (name, value))
    lines.append('    OfficeFormats: [%s],' %
                 ", ".join('"%s"' % names[n] for n in office_names if n in names))
    lines.append('    RasterFormats: [%s],' %
                 ", ".join('"%s"' % names[n] for n in raster_names if n in names))
    lines.append('    isRaster: function(t) { return this.RasterFormats.indexOf(t) != -1 },')
    lines.append('    isImage: function(t) { return t.indexOf("image/") == 0 && !this.isRaster(t) },')
    lines.append('    isOffice: function(t) { return this.OfficeFormats.indexOf(t) != -1 }')
    lines.append('}')
    lines.append('')
    lines.append('// Harmattan has no calligraconverter and no way to install one.')
    lines.append('var ConvertChecker = { calligra: false }')
    lines.append('')

    text = re.sub(r'^\.import seaprint\.\w+ 1\.0 as \w+\s*$\n', '', text, flags=re.M)
    text = text.replace("Mimer.Mimer.", "Mimer.")
    text = text.replace("ConvertChecker.ConvertChecker.", "ConvertChecker.")

    # Qt 4.7 cannot import one JavaScript file from another at all -- not even
    # in a library, where it is the only kind of import that would be allowed.
    # strings.js (the printer vocabulary) is therefore written into this file;
    # nothing else imports it, and its three tables collide with nothing here.
    strings_path = os.path.join(os.path.dirname(os.path.join(src, "pages", "x")), "strings.js")
    with open(strings_path) as f:
        strings = f.read()
    strings = re.sub(r'^\.pragma library\s*$\n', '', strings, flags=re.M)
    text = re.sub(r'^\.import "strings\.js" as Strings\s*$\n', '', text, flags=re.M)
    text = text.replace("Strings.", "")

    lines.append("// strings.js, written in here: see above.")
    lines.append(strings)

    # ".pragma library" has to be the first thing in the file; everything
    # generated goes right after it, before the code that uses it.
    text = re.sub(r'^\.pragma library\s*$\n', '', text, flags=re.M)
    return ".pragma library\n\n" + "\n".join(lines) + "\n" + text


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

    # The pictures the pages carry (the placeholder printer icon) go along.
    for name in sorted(os.listdir(os.path.join(src, "pages"))):
        if name.endswith((".svg", ".png")):
            shutil.copy(os.path.join(src, "pages", name), os.path.join(dst, "pages", name))

    # utils.js needs one change of its own; see fix_utils().
    for name in sorted(os.listdir(os.path.join(src, "pages"))):
        if name.endswith(".js"):
            with open(os.path.join(src, "pages", name)) as f:
                text = f.read()
            if name == "utils.js":
                text = fix_utils(text, src)
            with open(os.path.join(dst, "pages", name), "w") as f:
                f.write(text)

    print("%d QML files converted" % len(files))


if __name__ == "__main__":
    main()
