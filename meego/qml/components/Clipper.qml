import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
MouseArea {
    anchors.fill: parent
    onPressAndHold: {
        Clipboard.text = parent.text
        notifier.notify(qsTr("Copied to clipboard"))
    }
}
