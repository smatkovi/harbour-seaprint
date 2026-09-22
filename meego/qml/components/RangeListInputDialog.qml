import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
Dialog {
    id: dialog
    orientationLock: PageOrientation.Automatic
    property string title
    property variant value
    canAccept: valueField.acceptableInput

    Column {
        width: parent.width

        DialogHeader { }

        TextField {
            id: valueField
            acceptableInput: value !== undefined && value.length !== 0
            width: parent.width
            placeholderText: "1,2,17-42"
            label: title
            focus: true
            labelVisible: true
            onTextChanged: { dialog.value = RangeListChecker.parse(text) }
        }
    }

}
