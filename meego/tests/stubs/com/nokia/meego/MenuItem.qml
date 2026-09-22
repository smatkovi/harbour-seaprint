import QtQuick 1.1
Item {
    property string text
    property bool enabled: true
    property variant platformStyle
    signal clicked()
    width: parent ? parent.width : 0
    height: 60
}
