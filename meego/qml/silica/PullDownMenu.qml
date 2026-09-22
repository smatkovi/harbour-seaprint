import QtQuick 1.1
import com.nokia.meego 1.0 as Meego

// Silica's PullDownMenu. Harmattan has no pull-down gesture, so the entries
// are reached through a button at the top right of the page, where a MeeGo
// user looks for a menu anyway.
//
// The entries are collected rather than aliased, for the reason spelled out
// in ContextMenu.qml.
Item {
    id: root

    property bool _moving: false

    // The button belongs to the page, not to the flickable the menu was
    // declared in, so it does not scroll away with the content.
    parent: null
    Component.onCompleted: {
        var page = root
        while (page && page.pageStack === undefined)
            page = page.parent
        if (page)
            root.parent = page
        _collect()
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
                if (kids[i] !== menu && kids[i] !== button) {
                    kids[i].parent = column
                    moved = true
                    break
                }
            }
        }
        _moving = false
    }

    onChildrenChanged: _collect()

    anchors { top: parent ? parent.top : undefined; right: parent ? parent.right : undefined }
    width: AppTheme.itemSizeSmall
    height: AppTheme.itemSizeSmall
    z: 100

    Item {
        id: button
        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            color: AppTheme.highlightColor
            opacity: area.pressed ? 0.3 : 0
            radius: 4
        }
        Text {
            anchors.centerIn: parent
            text: "⋮"
            color: AppTheme.highlightColor
            font.pixelSize: AppTheme.fontSizeLarge
        }
        MouseArea {
            id: area
            anchors.fill: parent
            onClicked: menu.open()
        }
    }

    Meego.Menu {
        id: menu
        // The pane belongs to the window, not to this little button.
        parent: appWindow

        Column {
            id: column
            anchors { left: parent.left; right: parent.right }

            // Harmattan's MenuItem calls parent.closeLayout() on itself when
            // it is tapped -- that is how a menu closes after a choice.
            // MenuLayout provides it; this column has to as well, or the call
            // throws and the entry's own handler never runs: the setting
            // could be opened but not changed.
            function closeLayout() {
                menu.close()
            }
        }
    }
}
