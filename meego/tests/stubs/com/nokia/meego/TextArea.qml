import QtQuick 1.1
// TextEdit-based, like the real one: the pages reach into its font and
// wrapMode, which a plain Item cannot offer.
TextEdit {
    property string placeholderText
    property variant platformStyle
    property bool platformSipAttributes
    width: 200; height: 120
}
