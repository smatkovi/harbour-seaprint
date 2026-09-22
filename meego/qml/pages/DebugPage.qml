import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "utils.js" as Utils

Page {

    id: aboutPage
    orientationLock: PageOrientation.Automatic
    property variant printer

    Flickable {
        id: flick
        anchors.fill : parent
        contentHeight: textArea.height + header.height
        contentWidth : parent.width - (2.0 * AppTheme.paddingLarge)

        PageHeader {
            id: header
            title: Utils.unknownForEmptyString(printer.attrs["printer-name"].value)
        }

        TextArea {
            id: textArea

            readOnly: true
            width: parent.width
            anchors.top: header.bottom
            anchors.bottom: flick.bottom
            font.family: "courier"
            font.pixelSize: AppTheme.fontSizeTiny
            wrapMode: TextEdit.Wrap
            text: JSON.stringify(printer.attrs, null, "  ")

            onPressAndHold: {
                selectAll()
                copy()
                deselect()
                notifier.notify(qsTr("Copied to clipboard"))
            }
        }
    }
}
