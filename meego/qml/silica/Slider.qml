import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
// Silica's Slider, which names its bounds differently from the MeeGo one and
// shows a label and the current value.
Item {
    id: root
    property real minimumValue: 0
    property real maximumValue: 100
    property real value: 0
    property real stepSize: 1
    property string label: ""
    property string valueText: ""
    property bool enabled: true

    width: parent ? parent.width : 0
    height: column.height + AppTheme.paddingMedium

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
            text: root.label + (root.valueText !== "" ? ": " + root.valueText : "")
            visible: text !== ""
            color: AppTheme.secondaryColor
            font.pixelSize: AppTheme.fontSizeExtraSmall
        }
        Meego.Slider {
            id: slider
            width: parent.width
            enabled: root.enabled
            minimum: root.minimumValue
            maximum: root.maximumValue
            stepSize: root.stepSize
            value: root.value
            onValueChanged: if (value !== root.value) root.value = value
        }
    }
}
