import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
// Silica's PullDownMenu. Harmattan has no pull-down gesture, so the same
// entries are reached through a button at the top right of the page, where a
// MeeGo user looks for a menu anyway -- the same arrangement as in the other
// ports of Sailfish apps in this family.
Item {
    id: root
    default property alias entries: layout.children

    // The button belongs to the page, not to the flickable the menu was
    // declared in, so it does not scroll away with the content.
    parent: null
    Component.onCompleted: {
        var page = root
        while (page && page.pageStack === undefined)
            page = page.parent
        if (page)
            root.parent = page
    }

    anchors { top: parent ? parent.top : undefined; right: parent ? parent.right : undefined }
    width: AppTheme.itemSizeSmall
    height: AppTheme.itemSizeSmall
    z: 100

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

    Meego.Menu {
        id: menu
        // The pane belongs to the window, not to this little button.
        parent: appWindow
        Meego.MenuLayout { id: layout }
    }
}
