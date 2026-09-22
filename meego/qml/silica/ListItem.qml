import QtQuick 1.1

// Silica's ListItem: a row of a list that lights up while it is pressed and
// can open a context menu when held.
//
// The row's contents are collected into contentItem rather than taken through
// a default-property alias: QtQuick 1.1 would put this component's own
// background and mouse area in there too (see ContextMenu.qml for what that
// does to a menu). The pages size the row by setting contentItem.height, so
// the name has to stay.
Item {
    id: root
    property bool highlighted: area.pressed
    property bool down: area.pressed
    property real contentHeight: AppTheme.itemSizeSmall
    property bool enabled: true
    // Silica's ListItem takes a menu and opens it on a long press.
    property variant menu: null
    property bool showMenuOnPressAndHold: true

    property alias contentItem: content
    property bool _moving: false

    signal clicked()
    signal pressAndHold()

    function openMenu() {
        if (menu)
            menu.open(root)
    }

    function _collect() {
        if (_moving)
            return
        _moving = true
        var moved = true
        while (moved) {
            moved = false
            var kids = root.children
            for (var i = 0; i < kids.length; i++) {
                if (kids[i] !== content && kids[i] !== background && kids[i] !== area) {
                    kids[i].parent = content
                    moved = true
                    break
                }
            }
        }
        _moving = false
    }

    onChildrenChanged: _collect()
    Component.onCompleted: _collect()

    width: parent ? parent.width : 0
    height: content.height
    opacity: enabled ? 1.0 : 0.4

    Rectangle {
        id: background
        anchors.fill: parent
        color: AppTheme.highlightBackgroundColor
        opacity: area.pressed ? AppTheme.highlightBackgroundOpacity : 0
    }

    Item {
        id: content
        width: parent.width
        height: root.contentHeight
    }

    MouseArea {
        id: area
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
        onPressAndHold: {
            root.pressAndHold()
            if (root.showMenuOnPressAndHold)
                root.openMenu()
        }
    }
}
