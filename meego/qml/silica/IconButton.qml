import QtQuick 1.1
import seaprint.tintedimage 1.0
// Silica's IconButton.
Item {
    id: root
    property string icon: ""
    property alias source: image.source
    property bool enabled: true
    property bool down: area.pressed
    signal clicked()

    width: AppTheme.itemSizeSmall
    height: AppTheme.itemSizeSmall
    opacity: enabled ? (area.pressed ? 0.6 : 1.0) : 0.4

    TintedImage {
        id: image
        anchors.centerIn: parent
        width: AppTheme.iconSizeMedium
        height: AppTheme.iconSizeMedium
    }

    MouseArea {
        id: area
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
    }
}
