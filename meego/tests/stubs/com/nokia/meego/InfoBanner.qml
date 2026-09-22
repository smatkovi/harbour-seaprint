import QtQuick 1.1
Item {
    property string text
    property string iconSource
    property int timerShowTime: 3000
    property bool timerEnabled: true
    property variant platformStyle
    function show() {}
    function hide() {}
    width: 480; height: 80
}
