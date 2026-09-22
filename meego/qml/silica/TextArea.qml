import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
// Silica's TextArea: the MeeGo one with a label above it. A FocusScope for
// the same reason as TextField.
FocusScope {
    id: root
    property alias text: field.text
    property alias readOnly: field.readOnly
    property alias placeholderText: field.placeholderText
    property alias wrapMode: field.wrapMode
    property alias font: field.font
    property string label: ""
    // Upstream copies the attribute dump to the clipboard on a long press.
    // Harmattan's own TextArea answers a long press with its cut/copy/paste
    // bubble, which does the same thing in the way the platform does it, so
    // these are here for the binding and left to the platform.
    signal pressAndHold()
    function selectAll() { field.selectAll() }
    function copy() { field.copy() }
    function deselect() { field.deselect() }

    width: parent ? parent.width : 0
    height: column.height

    Column {
        id: column
        width: parent.width
        spacing: 2

        Text {
            x: AppTheme.horizontalPageMargin
            width: parent.width - 2 * AppTheme.horizontalPageMargin
            text: root.label
            visible: text !== ""
            color: AppTheme.secondaryColor
            font.pixelSize: AppTheme.fontSizeExtraSmall
        }
        Meego.TextArea {
            id: field
            x: AppTheme.horizontalPageMargin
            width: parent.width - 2 * AppTheme.horizontalPageMargin
            focus: true
        }
    }
}
