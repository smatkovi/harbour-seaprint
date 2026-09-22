import QtQuick 1.1
// Silica's Label: a Text with the theme's colour and size, wrapping off.
Text {
    property real leftPadding: 0
    property real rightPadding: 0
    property bool highlighted: false
    // QtQuick 2's Text calls it contentWidth, QtQuick 1.1's paintedWidth.
    property real contentWidth: paintedWidth
    property real contentHeight: paintedHeight
    color: highlighted ? AppTheme.highlightColor : AppTheme.primaryColor
    font.pixelSize: AppTheme.fontSizeMedium
    // Silica fades a truncated line; a QtQuick 1.1 Text can only elide.
    property int truncationMode: 0
    x: leftPadding
}
