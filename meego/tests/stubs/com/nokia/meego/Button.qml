import QtQuick 1.1
Item {
    property string text
    property string iconSource
    property bool enabled: true
    property bool checked: false
    property bool checkable: false
    property variant platformStyle
    signal clicked()
    width: 160; height: 60
}
