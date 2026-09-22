import QtQuick 1.1
import "../silica"

// A number, entered in a dialog.
//
// Upstream puts a Slider and an edit button inside a MenuItem of the context
// menu. A Harmattan Menu is a list of buttons and cannot hold a slider, so the
// setting opens the number dialog that upstream offers behind its edit button
// -- one tap instead of two, and the keyboard is the better way to reach 250
// copies anyway.
Setting {
    property int minimum_high: 0
    property int low: _valid ? parent.printer.attrs[name+"-supported"].value.low : (minimum_high != 0) ? 1 : 0
    property int high: _valid ? ensure_minimum(parent.printer.attrs[name+"-supported"].value.high) : minimum_high

    function ensure_minimum(orig)
    {
        if(orig < minimum_high)
        {
            return minimum_high;
        }
        else
        {
            return orig;
        }
    }

    displayValue: choice != undefined ? choice : default_choice

    hasMenu: false

    onClicked: {
        var dialog = pageStack.push(Qt.resolvedUrl("IntegerInputDialog.qml"),
                                    {title: prettyName, min: low, max: high,
                                     value: choice != undefined ? choice : low});
        dialog.accepted.connect(function() {
            choice = dialog.value;
        })
    }
}
