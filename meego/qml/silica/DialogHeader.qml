import QtQuick 1.1
// Silica's DialogHeader carries the title and the accept/cancel buttons. Here
// the buttons are in the dialog's own bar, so this only hands the title over
// and takes no room.
Item {
    id: root
    property string title: ""

    width: parent ? parent.width : 0
    height: 0

    onTitleChanged: publish()
    Component.onCompleted: publish()

    function publish() {
        var dialog = root.parent
        while (dialog && dialog.canAccept === undefined)
            dialog = dialog.parent
        if (dialog && title !== "")
            dialog.dialogTitle = title
    }
}
