import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
// Silica's ContextMenu: a list of MenuItems opened from an item. Harmattan
// shows the same thing as a Menu popping up from the bottom, which is where a
// MeeGo user expects a context menu.
//
// The items are declared as children of this (a Repeater among them, in the
// settings), so they are routed into the menu's layout.
Item {
    id: root
    visible: false

    property alias entries: layout.children
    default property alias content: layout.children

    function open(item) {
        menu.open()
    }
    function close() {
        menu.close()
    }

    Meego.Menu {
        id: menu
        Meego.MenuLayout { id: layout }
    }
}
