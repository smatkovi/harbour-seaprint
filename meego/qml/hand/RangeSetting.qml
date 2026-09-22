import QtQuick 1.1
import "../silica"

// Which pages to print.
//
// Upstream has two sliders in the context menu for the first and the last
// page, and an "Advanced" entry for a list like 1,4-7. A Harmattan Menu cannot
// hold a slider, so the setting opens that advanced dialog straight away: it
// can say everything the two sliders can, and the page range of a document
// with more than fifty pages was out of the sliders' reach anyway.
Setting {
    property int high
    property bool acceptRangeList: false

    displayValue: prettify(choice)

    function prettify(choice)
    {
        if(choice == undefined)
        {
            return qsTr("all");
        }
        else if(choice.low != undefined)
        {
            return (""+choice.low+" - "+choice.high)
        }
        else
        {
            var ret = "";
            for(var i = 0; i <  choice.length; i++)
            {
                if(i!=0)
                {
                    ret = ret+","
                }
                if(choice[i].low == choice[i].high)
                {
                    ret=ret+choice[i].low
                }
                else
                {
                    ret=ret+choice[i].low+"-"+choice[i].high
                }
            }
            return ret
        }
    }

    hasMenu: false

    onClicked: {
        var dialog = pageStack.push(Qt.resolvedUrl("RangeListInputDialog.qml"),
                                    {title: prettyName});
        dialog.accepted.connect(function() {
            choice = dialog.value;
        })
    }
}
