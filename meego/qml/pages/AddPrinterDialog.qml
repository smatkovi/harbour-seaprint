import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import seaprint.ippprinter 1.0
import "utils.js" as Utils

Dialog {
    id: dialog
    orientationLock: PageOrientation.Automatic
    property string value
    property string ssid
    property variant printerName: false
    canAccept: Object.keys(printer.attrs).length != 0

    Connections {
        target: wifi
        onConnectedChanged: {
            if(!wifi.connected) {
                dialog.reject()
            }
        }
    }

    Component.onCompleted: {
        valueField.forceActiveFocus()
    }

    IppPrinter {
        id: printer
        url: valueField.text
        onAttrsChanged: {
            if(printer.attrs.hasOwnProperty("printer-name")) {
                printerName = Utils.unknownForEmptyString(printer.attrs["printer-name"].value)
            }
            else
            {
                printerName = qsTr("Unknown")
            }

        }
    }

    Column {
        id: col
        width: parent.width

        DialogHeader {
            title: qsTr("Add printer")
        }

        TextField {
            id: valueField
            width: parent.width
            placeholderText: "192.168.1.1/ipp/print"
            inputMethodHints: Qt.ImhUrlCharactersOnly | Qt.ImhNoAutoUppercase
            label: title
        }
        Row {
            x: AppTheme.paddingLarge
            spacing: AppTheme.paddingMedium

            Label {
                id: wifi_label
                text: qsTr("On WiFi:")
            }
            Label {
                id: ssid_label
                color: AppTheme.secondaryColor
                text: wifi.ssid
            }
        }
        Row {
            x: AppTheme.paddingLarge
            spacing: AppTheme.paddingMedium

            Label {
                id: found_label
                text: canAccept ? qsTr("Found:") : qsTr("No printer found")
            }
            Label {
                id: printer_label
                color: AppTheme.secondaryColor
                text: canAccept ? printerName : ""
            }
        }

        Item {
            width: 1
            height: 2*AppTheme.paddingLarge
        }

        Row {
            visible: valueField.text.indexOf(":9100") != -1
            x: AppTheme.paddingLarge

            width: parent.width-2*AppTheme.paddingLarge
            spacing: AppTheme.paddingMedium

            Icon {
                id: warningIcon
                source: "image://theme/icon-m-warning"
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                width: parent.width-warningIcon.width-AppTheme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter
                color: AppTheme.highlightColor
                wrapMode: Text.WordWrap
                text: qsTr("Port 9100 is not used for IPP.")
            }
        }

        Row {
            id: suffixWarning
            visible: canAccept && !printer.correctSuffix
            x: AppTheme.paddingLarge

            width: parent.width-2*AppTheme.paddingLarge
            spacing: AppTheme.paddingMedium

            Icon {
                id: warningIcon2
                source: "image://theme/icon-m-warning"
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                width: parent.width-warningIcon2.width-AppTheme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter
                color: AppTheme.highlightColor
                wrapMode: Text.WordWrap
                text: qsTr("The uri suffix is not in the printer's supported list.")+" "+
                      qsTr("It might not accept print jobs on this address.")+" "+
                      qsTr("Consider using a suffix like \"/ipp/print\".")
            }
        }

        Item {
            width: 1
            height: 2*AppTheme.paddingLarge
        }

        Label
        {
            x: AppTheme.paddingLarge
            visible: suffixWarning.visible && printer.suffixes.length != 0
            text: qsTr("The printer/server lists these suffixes:")
        }

        Repeater
        {
            model: suffixWarning.visible ? printer.suffixes : 0
            Label
            {
                x: AppTheme.paddingLarge
                text: printer.suffixes[index]
            }

        }

    }

    onDone: {
        if (result == DialogResult.Accepted) {
            value = valueField.text
            ssid = wifi.ssid
        }
    }

}
