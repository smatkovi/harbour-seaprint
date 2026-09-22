import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "../pages/utils.js" as Utils

Dialog {
    id: dialog
    orientationLock: PageOrientation.Automatic
    property string name
    property string choice
    property string new_choice: choice
    property variant choices
    property variant preferred_choices
    property string preferred_choice_suffix
    property variant strings
    canAccept: false

    ListView
    {
        anchors.fill: parent

        header: DialogHeader {}

        model: choices
        delegate: BackgroundItem {
            onClicked: {
                new_choice=choices[index]
                dialog.canAccept = true
            }
            Label {
                x: AppTheme.paddingLarge
                anchors.verticalCenter: parent.verticalCenter
                highlighted: choices[index]==new_choice
                text: Utils.ippName(name, choices[index], strings)
                      + (Utils.has(preferred_choices, choices[index]) ? " "+preferred_choice_suffix : "")
            }
        }
    }

    onDone: {
        if (result == DialogResult.Accepted) {
            choice = new_choice
        }
    }
}
