import QtQuick 1.1
import com.nokia.meego 1.0 as Meego

// Silica's Dialog: a page with Cancel and Accept, which reports back through
// accepted()/rejected() and the done() handler.
//
// Harmattan would use a Sheet, but the app pushes its dialogs on the page
// stack and keeps the returned object (dialog.accepted.connect(...)), so this
// stays a Page with the two buttons in a bar of its own.
//
// The content is collected rather than taken through a default-property
// alias: QtQuick 1.1 puts everything declared in a component's own body into
// its default property, so the bar and the content holder would end up inside
// the holder itself -- which showed on the device as a dialog with nothing in
// it. The dialogs also declare non-visual children (Connections), so `data`
// is what is walked, not `children`.
Meego.Page {
    id: root

    property bool canAccept: true
    property string dialogTitle: ""
    property string acceptText: qsTr("Accept")
    property string cancelText: qsTr("Cancel")

    property bool _moving: false

    signal accepted()
    signal rejected()
    signal done(int result)

    function accept() {
        if (!canAccept)
            return
        done(1)
        accepted()
        pageStack.pop()
    }
    function reject() {
        done(0)
        rejected()
        pageStack.pop()
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
                if (kids[i] !== bar && kids[i] !== container) {
                    kids[i].parent = container
                    moved = true
                    break
                }
            }
        }
        _moving = false
    }

    onChildrenChanged: _collect()
    Component.onCompleted: _collect()

    Rectangle {
        id: bar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: AppTheme.itemSizeLarge
        color: AppTheme.rgba(AppTheme.highlightBackgroundColor, 0.15)
        z: 10

        Text {
            anchors {
                left: cancel.right; right: accept.left
                leftMargin: AppTheme.paddingMedium; rightMargin: AppTheme.paddingMedium
                verticalCenter: parent.verticalCenter
            }
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: root.dialogTitle
            color: AppTheme.highlightColor
            font.pixelSize: AppTheme.fontSizeSmall
        }

        Meego.Button {
            id: cancel
            anchors { left: parent.left; leftMargin: AppTheme.paddingMedium
                      verticalCenter: parent.verticalCenter }
            width: AppTheme.itemSizeExtraLarge
            text: root.cancelText
            onClicked: root.reject()
        }
        Meego.Button {
            id: accept
            anchors { right: parent.right; rightMargin: AppTheme.paddingMedium
                      verticalCenter: parent.verticalCenter }
            width: AppTheme.itemSizeExtraLarge
            text: root.acceptText
            enabled: root.canAccept
            onClicked: root.accept()
        }
    }

    Item {
        id: container
        anchors { top: bar.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
    }
}
