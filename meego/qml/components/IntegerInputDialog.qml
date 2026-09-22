import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
Dialog {
    id: dialog
    orientationLock: PageOrientation.Automatic
    property int value
    property int min;
    property int max;
    property string title
    canAccept: valueField.acceptableInput

    Column {
        width: parent.width

        DialogHeader { }

        TextField {
            id: valueField
            validator: IntValidator{bottom: min; top: max;}
            width: parent.width
            placeholderText: ""+min+"-"+(max == 65535 ? "..." : max)
            label: title
            focus: true
            inputMethodHints: Qt.ImhDigitsOnly

        }
    }

    onDone: {
        if (result == DialogResult.Accepted) {
            value = valueField.text
        }
    }

}
