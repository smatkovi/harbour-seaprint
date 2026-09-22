import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
Item {
    height: button.height + (menu != undefined ? menu.height : 0) + resetMenu.height
    width: parent.width

    property string name
    property string prettyName
    property int tag
    property string subkey: ""
    property bool _valid: parent.isValid(name)
    property bool valid: _valid

    property variant choice
    property variant default_choice: parent.getDefaultChoice(name)

    property variant strings: parent.printer.strings

    id: setting

    Component.onCompleted: parent.setInitialChoice(setting)

    onChoiceChanged: parent.choiceMade(setting)

    signal clicked()
    onClicked: {
        if (hasMenu)
        {
            menu.open(setting)
        }
    }

    signal pressAndHold()
    onPressAndHold: {
        resetMenu.open(setting)
    }

    property alias displayValue: button.value

    ValueButton {
        id: button
        enabled: valid
        label: prettyName
        onClicked: parent.clicked()
        onPressAndHold: parent.pressAndHold()
        valueColor: choice != undefined ? AppTheme.highlightColor : AppTheme.secondaryHighlightColor
    }

    property variant menu
    property bool hasMenu: true

    function reset()
    {
        choice = undefined
    }

    ContextMenu {
        id: resetMenu

        MenuItem {
            text: qsTr("Reset")
            onClicked: reset()
        }
    }

    function highlight()
    {
        highlightAnimation.start()
    }

    NumberAnimation
    {
        id: highlightAnimation
        target: fillRectangle
        property: "opacity"
        from: 0.5
        to: 0
        duration: 1000
    }

    Rectangle {
        id: fillRectangle
        anchors.fill: parent
        color: AppTheme.highlightBackgroundColor
        opacity: 0
    }

}
