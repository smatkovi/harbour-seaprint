import QtQuick 1.1
import seaprint.tintedimage 1.0
// Silica's HighlightImage: the picture as it is, or painted in highlightColor
// while `highlighted` -- which is how the printer list marks a format the
// printer cannot take.
TintedImage {
    property bool highlighted: false
    property color highlightColor: AppTheme.highlightColor
    property variant sourceSize: undefined
    width: AppTheme.iconSizeMedium
    height: AppTheme.iconSizeMedium
    tinted: highlighted
    color: highlightColor
}
