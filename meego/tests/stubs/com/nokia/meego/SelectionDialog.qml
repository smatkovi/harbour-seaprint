import QtQuick 1.1
Item {
    property string titleText
    property variant model
    property int selectedIndex: -1
    property variant platformStyle
    signal accepted()
    signal rejected()
    function open() {}
    function close() {}
}
