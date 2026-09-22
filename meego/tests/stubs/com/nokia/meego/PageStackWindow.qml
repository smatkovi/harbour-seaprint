import QtQuick 1.1
Item {
    property variant initialPage
    property bool showStatusBar: true
    property bool showToolBar: true
    property variant pageStack: stack
    property variant platformStyle
    width: 480; height: 854
    PageStack { id: stack }
}
