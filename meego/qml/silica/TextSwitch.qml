import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
// Silica's TextSwitch: a label with a switch, the whole row tappable.
Item {
    id: root
    property string text: ""
    property string description: ""
    property bool checked: false
    property bool automaticCheck: true
    property bool enabled: true
    signal clicked()

    width: parent ? parent.width : 0
    height: Math.max(toggle.height, column.height) + 2 * AppTheme.paddingSmall
    opacity: enabled ? 1.0 : 0.4

    Column {
        id: column
        anchors {
            left: parent.left; leftMargin: AppTheme.horizontalPageMargin
            right: toggle.left; rightMargin: AppTheme.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            text: root.text
            color: root.checked ? AppTheme.primaryColor : AppTheme.secondaryColor
            font.pixelSize: AppTheme.fontSizeSmall
        }
        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            visible: root.description !== ""
            text: root.description
            color: AppTheme.secondaryColor
            font.pixelSize: AppTheme.fontSizeExtraSmall
        }
    }

    Meego.Switch {
        id: toggle
        anchors {
            right: parent.right; rightMargin: AppTheme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }
        checked: root.checked
        onCheckedChanged: if (checked !== root.checked) root.checked = checked
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        onClicked: {
            if (root.automaticCheck)
                root.checked = !root.checked
            root.clicked()
        }
    }
}
