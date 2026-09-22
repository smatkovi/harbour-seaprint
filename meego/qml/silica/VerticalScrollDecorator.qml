import QtQuick 1.1
// Silica's VerticalScrollDecorator: the thin bar that shows where in a long
// page the view is. It finds the flickable it was declared in.
Item {
    id: root
    property variant flickable: parent

    anchors.fill: parent

    Rectangle {
        width: 4
        radius: 2
        color: AppTheme.highlightColor
        opacity: flickable && flickable.moving ? 0.6 : 0
        visible: flickable !== undefined && flickable !== null
                 && flickable.contentHeight > flickable.height
        x: root.width - width
        y: flickable ? flickable.visibleArea.yPosition * root.height : 0
        height: flickable ? Math.max(30, flickable.visibleArea.heightRatio * root.height) : 0

        Behavior on opacity { NumberAnimation { duration: 200 } }
    }
}
