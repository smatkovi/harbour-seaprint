import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
// Silica's BusyLabel: a spinner with a line of text under it, centred.
Item {
    id: root
    property string text: ""
    property bool running: false

    anchors.fill: parent
    visible: running

    Column {
        anchors.centerIn: parent
        spacing: AppTheme.paddingLarge
        width: parent.width - 2 * AppTheme.horizontalPageMargin

        Meego.BusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            running: root.running
            platformStyle: Meego.BusyIndicatorStyle { size: "large" }
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: root.text
            color: AppTheme.highlightColor
            font.pixelSize: AppTheme.fontSizeSmall
        }
    }
}
