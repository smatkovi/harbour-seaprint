import QtQuick 1.1
// Silica's RemorseItem: the action is announced on the row it belongs to and
// carried out a few seconds later unless it is tapped away.
Item {
    id: root
    property int delay: 4000
    property variant _action: null

    function execute(item, text, action) {
        // Silica's first argument is the row to cover; here the banner covers
        // the row this item was declared in, which is the same one.
        root._action = action
        label.text = text
        root.visible = true
        timer.restart()
    }
    function cancel() {
        timer.stop()
        root.visible = false
        root._action = null
    }

    visible: false
    anchors.fill: parent
    z: 200

    Rectangle {
        anchors.fill: parent
        color: AppTheme.rgba(AppTheme.highlightBackgroundColor, 0.9)
    }
    Text {
        id: label
        anchors {
            left: parent.left; leftMargin: AppTheme.horizontalPageMargin
            right: undo.left; rightMargin: AppTheme.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        elide: Text.ElideRight
        color: AppTheme.primaryColor
        font.pixelSize: AppTheme.fontSizeSmall
    }
    Text {
        id: undo
        anchors { right: parent.right; rightMargin: AppTheme.horizontalPageMargin
                  verticalCenter: parent.verticalCenter }
        text: qsTr("Tap to undo")
        color: AppTheme.primaryColor
        font.pixelSize: AppTheme.fontSizeExtraSmall
    }
    MouseArea { anchors.fill: parent; onClicked: root.cancel() }

    Timer {
        id: timer
        interval: root.delay
        onTriggered: {
            root.visible = false
            if (root._action)
                root._action()
            root._action = null
        }
    }
}
