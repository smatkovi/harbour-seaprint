import QtQuick 1.1
// Silica's BackgroundItem: a row that lights up while it is pressed.
Item {
    id: root
    property bool highlighted: area.pressed
    property bool down: area.pressed
    property color highlightedColor: AppTheme.rgba(AppTheme.highlightBackgroundColor,
                                                   AppTheme.highlightBackgroundOpacity)
    property real contentHeight: AppTheme.itemSizeSmall
    property bool enabled: true
    signal clicked()
    signal pressAndHold()

    width: parent ? parent.width : 0
    height: contentHeight
    opacity: enabled ? 1.0 : 0.4

    Rectangle {
        anchors.fill: parent
        color: root.highlightedColor
        visible: area.pressed
    }

    MouseArea {
        id: area
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
        onPressAndHold: root.pressAndHold()
    }
}
