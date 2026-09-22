import QtQuick 1.1
// Silica's VerticalScrollDecorator: the thin bar that shows where in a long
// page the view is. It finds the flickable it was declared in.
Item {
    id: root
    // Silica's decorator finds the flickable it was declared in. Not every
    // parent is one -- a page or a column has no visibleArea, and reading it
    // only fills the log.
    property variant flickable: parent
    property bool _usable: flickable !== undefined && flickable !== null
                           && flickable.visibleArea !== undefined

    anchors.fill: parent

    Rectangle {
        width: 4
        radius: 2
        color: AppTheme.highlightColor
        opacity: _usable && flickable.moving ? 0.6 : 0
        visible: _usable && flickable.contentHeight > flickable.height
        x: root.width - width
        y: _usable ? flickable.visibleArea.yPosition * root.height : 0
        height: _usable ? Math.max(30, flickable.visibleArea.heightRatio * root.height) : 0

        Behavior on opacity { NumberAnimation { duration: 200 } }
    }
}
