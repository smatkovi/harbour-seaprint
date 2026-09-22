import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "../components"

    Row
    {
        id: item
        property variant color: "white"
        property int level: 100
        property int high_level: 100
        property int low_level: 10
        property string name: ""
        property string type: ""
        property real value_fraction: level/(high_level == 0 ? 100 : high_level)

        spacing: AppTheme.paddingMedium
        anchors.bottomMargin: AppTheme.paddingMedium

        CylinderGraph
        {
            anchors.verticalCenter: parent.verticalCenter
            color: parent.color
            value: value_fraction
        }
        Label
        {
            anchors.verticalCenter: parent.verticalCenter
            width: AppTheme.itemSizeExtraSmall
            text: ""+Math.round(100*value_fraction)+"%"
            color: level <= low_level ? "red" : AppTheme.highlightColor
            font.pixelSize: AppTheme.fontSizeExtraSmall
        }
        Column
        {
            anchors.verticalCenter: parent.verticalCenter
            Label
            {
                text: name != "" ? name : qsTr("Unnamed supply")
                font.pixelSize: AppTheme.fontSizeExtraSmall
            }
            Label
            {
                text: type != "" ? type :  qsTr("Unknown type")
                font.pixelSize: AppTheme.fontSizeExtraSmall
                color: AppTheme.secondaryColor
            }
        }
    }


