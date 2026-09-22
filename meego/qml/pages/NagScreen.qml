import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "../components"

Page {
    orientationLock: PageOrientation.Automatic
    backNavigation: false

    Flickable {
        anchors.fill: parent
        pressDelay: 150
        contentHeight: col.implicitHeight

    Column {
        id:col
        y: AppTheme.paddingLarge
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2*AppTheme.paddingLarge
        spacing: AppTheme.paddingLarge

        Label {
            width: parent.width
            wrapMode: Text.WordWrap
            font.bold: true

            text: qsTr("Optional dependencies are not installed!")
        }

        Label {
            visible: appWin.expectCalligra
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("In order to print \"office\" documents like odt and doc(x) you need "+
                       "\"calligraconverter\" from the \"calligra\" package (version 3.2.1+, Sailfish OS 4.2).")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("To install, enable developer mode, and open a terminal.")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("Become root:")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap
            font.family: "monospace"

            text: "devel-su"
            Clipper {}

        }

        Label {
            visible: appWin.expectCalligra
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("Install Calligra:") + " " + (ConvertChecker.calligra ? qsTr("(done)") : "")
        }

        Label {
            visible: appWin.expectCalligra
            width: parent.width
            wrapMode: Text.WordWrap
            font.family: "monospace"

            text: "pkcon install calligra"
            Clipper {}
        }

        Label {
            visible: appWin.expectCalligra
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("If you are still seeing this message despite installing Calligra, that can be because of SailJail "+
                       "in Sailfish OS 4.x before the compatibility permission in 4.4. " +
                       "You can either upgrade to 4.4 or edit out the SailJail section in /usr/share/applications/harbour-seaprint.desktop")
        }

        Item{
            width: parent.width
            height: AppTheme.itemSizeMedium
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Dismiss")
            onClicked: pageStack.pop()
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Don't show again")
            onClicked: {
                nagScreenSetting.value = nagScreenSetting.expectedValue
                pageStack.pop()
            }
        }

        Item{
            width: parent.width
            height: AppTheme.itemSizeMedium
        }
    }
    }

}
