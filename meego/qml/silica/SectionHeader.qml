import QtQuick 1.1
// Silica's SectionHeader: a right-aligned caption with a rule under it.
Item {
    property alias text: label.text
    width: parent ? parent.width : 0
    height: label.height + 2 * AppTheme.paddingMedium
    Text {
        id: label
        anchors {
            right: parent.right; rightMargin: AppTheme.horizontalPageMargin
            left: parent.left; leftMargin: AppTheme.horizontalPageMargin
            top: parent.top; topMargin: AppTheme.paddingMedium
        }
        horizontalAlignment: Text.AlignRight
        elide: Text.ElideRight
        color: AppTheme.highlightColor
        font.pixelSize: AppTheme.fontSizeSmall
    }
    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom
                  leftMargin: AppTheme.horizontalPageMargin
                  rightMargin: AppTheme.horizontalPageMargin }
        height: 1
        color: AppTheme.secondaryColor
        opacity: 0.4
    }
}
