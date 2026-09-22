import QtQuick 1.1

// One setting that only makes sense while another one has certain values.
//
// The only change from upstream: `overlap` is built in a local array and then
// assigned. QtQuick 1.1 has no `property var`, and a `property variant` hands
// out a copy of what it holds, so overlap.push(...) would write into a
// throwaway and the dependency would never see a match.
Item {
    property alias target: targetConnection.target
    property variant values
    property variant overlap: []

    Component.onCompleted:
    {
        var found = []
        for(var i in values)
        {
            if(target.actual_choices.indexOf(values[i]) != -1)
            {
                found.push(values[i])
            }
        }
        overlap = found
        parent.valid = parent.valid && overlap.length != 0
    }

    Connections {
        id: targetConnection
        onChoiceChanged:
        {
            if(parent.choice != undefined && overlap.indexOf(target.choice) == -1)
            {
                parent.reset()
                parent.highlight()
            }
        }
    }
    Connections {
        target: parent
        onChoiceChanged:
        {
            if(parent.choice != undefined && overlap.indexOf(targetConnection.target.choice) == -1)
            {
                targetConnection.target.choice = overlap[0]
                targetConnection.target.highlight()
            }
        }
    }
}
