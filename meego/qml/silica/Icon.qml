import QtQuick 1.1
import seaprint.tintedimage 1.0
// Silica's Icon: a theme icon, optionally recoloured. The name is a Sailfish
// one; TintedImage maps it to Harmattan's icon set.
TintedImage {
    property variant sourceSize: undefined
    property bool highlighted: false
    property color highlightColor: AppTheme.highlightColor
    width: AppTheme.iconSizeMedium
    height: AppTheme.iconSizeMedium
    tinted: highlighted
    color: highlightColor
}
