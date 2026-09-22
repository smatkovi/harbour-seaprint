import QtQuick 1.1
// Silica's ListItem: a row of a list that lights up while it is pressed and
// can open a context menu when held.
Item {
    id: root
    property bool highlighted: area.pressed
    property bool down: area.pressed
    property real contentHeight: AppTheme.itemSizeSmall
    property bool enabled: true
    // Silica's ListItem takes a menu and opens it on a long press.
    property variant menu: null
    property bool showMenuOnPressAndHold: true

    signal clicked()
    signal pressAndHold()

    function openMenu() {
        if (menu)
            menu.open(root)
    }

    // The children go into contentItem, as in Silica: the pages size the row
    // by setting contentItem.height.
    default property alias _content: content.data
    property alias contentItem: content

    width: parent ? parent.width : 0
    height: content.height
    opacity: enabled ? 1.0 : 0.4

    Item {
        id: content
        width: parent.width
        height: root.contentHeight
    }

    Rectangle {
        anchors.fill: parent
        color: AppTheme.highlightBackgroundColor
        opacity: area.pressed ? AppTheme.highlightBackgroundOpacity : 0
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
