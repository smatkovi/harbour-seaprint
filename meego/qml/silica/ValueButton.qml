import QtQuick 1.1
// Silica's ValueButton: a label on the left, the chosen value on the right,
// the whole row tappable. This is what every print setting looks like.
Item {
    id: root
    property string label: ""
    property string value: ""
    property color valueColor: AppTheme.highlightColor
    property bool enabled: true
    signal clicked()
    signal pressAndHold()

    width: parent ? parent.width : 0
    height: Math.max(AppTheme.itemSizeSmall, column.height + 2 * AppTheme.paddingSmall)
    visible: enabled
    opacity: enabled ? 1.0 : 0.4

    Rectangle {
        anchors.fill: parent
        color: AppTheme.highlightBackgroundColor
        opacity: area.pressed ? AppTheme.highlightBackgroundOpacity : 0
    }

    Column {
        id: column
        anchors {
            left: parent.left; leftMargin: AppTheme.horizontalPageMargin
            right: parent.right; rightMargin: AppTheme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }
        spacing: 2

        Text {
            width: parent.width
            text: root.label
            color: AppTheme.secondaryColor
            font.pixelSize: AppTheme.fontSizeExtraSmall
            elide: Text.ElideRight
        }
        Text {
            width: parent.width
            text: root.value
            color: root.valueColor
            font.pixelSize: AppTheme.fontSizeSmall
            elide: Text.ElideRight
        }
    }

    MouseArea {
        id: area
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
        onPressAndHold: root.pressAndHold()
    }
}
