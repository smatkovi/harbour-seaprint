import QtQuick 1.1
import com.nokia.meego 1.0 as Meego

// Silica's ContextMenu: a list of choices opened from an item. Harmattan
// shows the same thing as a menu rising from the bottom of the screen.
//
// The entries are *not* taken through a default-property alias, which is the
// obvious way to write this and does not work here: QtQuick 1.1 puts every
// object declared in a component's own body into that same default property,
// so the menu itself would land inside its own layout, and an alias that
// points at another alias (MenuLayout's) resolves to nothing at all. Both
// mistakes look identical on the device -- the menu opens as an empty sheet.
//
// So the entries stay ordinary children of this item and are moved into the
// menu's column as they turn up, which also catches the ones a Repeater
// creates later.
Item {
    id: root

    // Takes no room where it is declared: the menu itself lives in the window.
    width: 0
    height: 0

    property bool _moving: false

    function open(item) {
        menu.open()
    }
    function close() {
        menu.close()
    }

    function _collect() {
        if (_moving)
            return
        _moving = true
        // Always take the first entry that is not the menu, so the entries
        // keep the order they were declared in.
        var moved = true
        while (moved) {
            moved = false
            var kids = root.children
            for (var i = 0; i < kids.length; i++) {
                if (kids[i] !== menu) {
                    kids[i].parent = column
                    moved = true
                    break
                }
            }
        }
        _moving = false
    }

    onChildrenChanged: _collect()
    Component.onCompleted: _collect()

    Meego.Menu {
        id: menu
        // A Menu anchors its pane to its parent item, and a ContextMenu
        // assigned to a property (as the settings do) has no parent at all --
        // the menu would open into nothing. appWindow is the root object, put
        // in the context by meego/main.cpp.
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
