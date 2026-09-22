import QtQuick 1.1
// Silica's MenuLabel: a caption inside a menu, not tappable.
Item {
    property alias text: label.text
    width: parent ? parent.width : 0
    height: visible ? label.height + 2 * AppTheme.paddingMedium : 0
    Text {
        id: label
        anchors {
            left: parent.left; leftMargin: AppTheme.horizontalPageMargin
            right: parent.right; rightMargin: AppTheme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        color: AppTheme.secondaryColor
        font.pixelSize: AppTheme.fontSizeExtraSmall
    }
}
