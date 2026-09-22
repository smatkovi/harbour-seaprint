import QtQuick 1.1
Item {
    property string acceptButtonText
    property string rejectButtonText
    property variant content
    property variant title
    property variant platformStyle
    signal accepted()
    signal rejected()
    function open() {}
    function close() {}
    width: 480; height: 854
}
