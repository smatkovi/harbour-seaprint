import QtQuick 1.1
Item {
    property real minimum: 0
    property real maximum: 100
    property real value: 0
    property real stepSize: 1
    property bool enabled: true
    property string valueIndicatorText
    property bool valueIndicatorVisible: false
    property variant platformStyle
    width: 200; height: 50
}
