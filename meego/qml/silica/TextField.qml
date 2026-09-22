import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
// Silica's TextField: the MeeGo one with a label above it.
Item {
    id: root
    property alias text: field.text
    property alias placeholderText: field.placeholderText
    property alias inputMethodHints: field.inputMethodHints
    property alias validator: field.validator
    // Not an alias: RangeListInputDialog binds its own validity to this, and
    // an alias to the MeeGo field's read-only property cannot be written.
    property bool acceptableInput: field.acceptableInput
    property bool labelVisible: true
    property alias readOnly: field.readOnly
    property alias errorHighlight: field.errorHighlight
    property string label: ""
    signal accepted()

    width: parent ? parent.width : 0
    height: column.height

    function forceActiveFocus() { field.forceActiveFocus() }
    function selectAll() { field.selectAll() }

    Column {
        id: column
        width: parent.width
        spacing: 2

        Text {
            x: AppTheme.horizontalPageMargin
            width: parent.width - 2 * AppTheme.horizontalPageMargin
            text: root.label
            visible: text !== ""
            elide: Text.ElideRight
            color: AppTheme.secondaryColor
            font.pixelSize: AppTheme.fontSizeExtraSmall
        }
        Meego.TextField {
            id: field
            x: AppTheme.horizontalPageMargin
            width: parent.width - 2 * AppTheme.horizontalPageMargin
            Keys.onReturnPressed: root.accepted()
            Keys.onEnterPressed: root.accepted()
        }
    }
}
