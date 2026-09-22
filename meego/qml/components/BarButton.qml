import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
BackgroundItem {
    id: barButton

    property alias text: label.text
    property bool active: false
    highlightedColor: AppTheme.rgba(AppTheme.highlightBackgroundFromColor(AppTheme.highlightBackgroundColor, AppTheme.colorScheme), AppTheme.highlightBackgroundOpacity)

    onClicked:
    {
        active = !active
    }

    function _color(alpha) {
        alpha = alpha*0.25
        return AppTheme.rgba(AppTheme.highlightBackgroundColor, alpha)
    }

    Rectangle {
        x: -1
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: _color(1) }
            GradientStop { position: 0.2; color: _color(0.6) }
            GradientStop { position: 0.8; color: _color(0.4) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    Label {
        id: label
        anchors.leftMargin: AppTheme.paddingLarge*2
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        color: barButton.highlighted ? AppTheme.highlightColor : AppTheme.primaryColor
    }

    Image {
        id: image
        anchors.right: parent.right
        anchors.rightMargin: AppTheme.paddingMedium
        anchors.verticalCenter: parent.verticalCenter
        source: active ? "image://theme/icon-m-common-expand" : "image://theme/icon-m-common-drilldown-arrow-inverse"
    }

}
