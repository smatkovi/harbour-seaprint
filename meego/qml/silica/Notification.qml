import QtQuick 1.1

// Nemo.Notifications' Notification, as the app uses it: a short line about
// what just happened.
//
// Not an InfoBanner: that one lives in com.nokia.extras, not in
// com.nokia.meego, and depends on a plugin this port would then have to
// require on the device. The banner is drawn here instead -- it says the same
// thing, in the same place, and nothing else has to be installed.
Item {
    id: root

    property int expireTimeout: 4000
    property string body: ""
    property string previewBody: ""
    property string summary: ""

    function publish() {
        label.text = body !== "" ? body : previewBody
        root.visible = true
        banner.y = 0
        timer.restart()
    }
    function close() {
        timer.stop()
        banner.y = -banner.height
        root.visible = false
    }

    // The banner belongs to the window, above whatever page is showing.
    // appWindow is the root object, put in the context by meego/main.cpp.
    parent: appWindow
    anchors.fill: parent ? parent : undefined
    visible: false
    z: 1000

    Rectangle {
        id: banner
        width: parent ? parent.width : 0
        height: Math.max(AppTheme.itemSizeSmall, label.height + 2 * AppTheme.paddingLarge)
        y: -height
        color: AppTheme.rgba(AppTheme.highlightBackgroundColor, 0.95)

        Behavior on y { NumberAnimation { duration: 200 } }

        Text {
            id: label
            anchors {
                left: parent.left; leftMargin: AppTheme.horizontalPageMargin
                right: parent.right; rightMargin: AppTheme.horizontalPageMargin
                verticalCenter: parent.verticalCenter
            }
            wrapMode: Text.WordWrap
            color: AppTheme.primaryColor
            font.pixelSize: AppTheme.fontSizeSmall
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.close()
        }
    }

    Timer {
        id: timer
        interval: root.expireTimeout
        onTriggered: root.close()
    }
}
