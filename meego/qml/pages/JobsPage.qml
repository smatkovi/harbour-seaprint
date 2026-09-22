import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "utils.js" as Utils

Page {
    id: page
    orientationLock: PageOrientation.Automatic
    property variant printer

    Component.onCompleted: {
        printer.getJobs()
    }

    Connections {
        target: printer
        onCancelStatus: {
            if(status != true) {
                notifier.notify(qsTr("Cancelling job failed"))
            }
        }
    }

    // To enable PullDownMenu, place our content in a Flickable
    Flickable {
        anchors.fill: parent
        pressDelay: 150

        // PullDownMenu and PushUpMenu must be declared in Flickable, ListView or GridView
        PullDownMenu {
//            MenuItem {
//                text: qsTr("Remove all")
//                onClicked: {
//                    console.log("todo")
//                }
//            }
            MenuItem {
                text: qsTr("Refresh")
                onClicked: {
                    printer.getJobs();
                }
            }
        }

        ListView {
            anchors.fill: parent
            pressDelay: 150
            id: listView

            model: printer.jobs.length

            header: PageHeader {
                id: pageHeader
                title: Utils.unknownForEmptyString(printer.attrs["printer-name"].value)
                description: printer.jobs.length==1 ?  printer.jobs.length+" "+qsTr("job") : printer.jobs.length+" "+qsTr("jobs")
            }

            delegate: ListItem {
                id: jobDelegate

                Label {
                    id: idLabel
                    anchors.leftMargin: AppTheme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    color: AppTheme.highlightColor
                    text: printer.jobs[index]["job-id"].value
                }

                Column {
                    anchors.left: idLabel.right
                    anchors.leftMargin: AppTheme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    Label {
                        text: printer.jobs[index]["job-name"] ? printer.jobs[index]["job-name"].value : qsTr("Untitled job")
                    }
                    Label {
                        font.pixelSize: AppTheme.fontSizeTiny
                        color: AppTheme.secondaryColor
                        property variant msg: printer.jobs[index]["job-printer-state-message"] && printer.jobs[index]["job-printer-state-message"].value != ""
                                         ? " ("+printer.jobs[index]["job-printer-state-message"].value+")" : ""
                        text: Utils.ippName("job-state", printer.jobs[index]["job-state"].value)+msg
                    }
                }

                RemorseItem {
                    id: cancelRemorse
                }

                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Cancel job")
                        onClicked: {
                            cancelRemorse.execute(jobDelegate, qsTr("Cancelling job"),
                                                  function() {printer.cancelJob(printer.jobs[index]["job-id"].value) })
                        }
                    }
                }
            }

        }

    }
}
