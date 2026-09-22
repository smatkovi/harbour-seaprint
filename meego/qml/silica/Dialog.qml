import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
// Silica's Dialog: a page with Cancel and Accept, which reports back through
// accepted()/rejected() and the done() handler.
//
// Harmattan would use a Sheet, but the app pushes its dialogs on the page
// stack and keeps the returned object (dialog.accepted.connect(...)), so this
// stays a Page with the two buttons in a bar of its own at the top.
Meego.Page {
    id: root

    property bool canAccept: true
    property string dialogTitle: ""
    property string acceptText: qsTr("Accept")
    property string cancelText: qsTr("Cancel")
    // Where the content lives: everything declared inside the dialog is placed
    // below the button bar. `data` rather than `children`, because the dialogs
    // also declare Connections and other non-visual objects.
    default property alias content: container.data

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

    Rectangle {
        id: bar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: AppTheme.itemSizeLarge
        color: AppTheme.rgba(AppTheme.highlightBackgroundColor, 0.15)

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
