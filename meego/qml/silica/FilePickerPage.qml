import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
import seaprint.tintedimage 1.0

// What Sailfish.Pickers' DocumentPickerPage and ImagePickerPage do, for
// Harmattan: walk the user's files and hand one back.
//
// A browser rather than a gallery query: on the N9 only pictures and music are
// reliably indexed by tracker, so looking for a PDF by asking the gallery
// would come up empty in exactly the case the app is for. FileBrowser (C++)
// does the listing.
Meego.Page {
    id: root

    property string title: qsTr("Choose file")
    // Suffixes to show, empty for everything.
    property variant suffixes: []
    property string folder: FileBrowser.home()
    property variant entries: []

    signal selected(string path)

    Component.onCompleted: reload()
    onFolderChanged: reload()

    function reload() {
        entries = FileBrowser.list(folder, suffixes)
    }

    Item {
        id: header
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: AppTheme.itemSizeLarge

        Text {
            anchors {
                left: up.right; right: parent.right
                leftMargin: AppTheme.paddingMedium
                rightMargin: AppTheme.horizontalPageMargin
                bottom: parent.bottom; bottomMargin: AppTheme.paddingMedium
            }
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideLeft
            text: FileBrowser.displayPath(root.folder)
            color: AppTheme.highlightColor
            font.pixelSize: AppTheme.fontSizeSmall
        }

        Item {
            id: up
            anchors { left: parent.left; bottom: parent.bottom }
            width: AppTheme.itemSizeSmall
            height: AppTheme.itemSizeSmall
            visible: root.folder !== FileBrowser.home()

            TintedImage {
                anchors.centerIn: parent
                width: AppTheme.iconSizeMedium
                height: AppTheme.iconSizeMedium
                source: "image://theme/icon-m-up"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.folder = FileBrowser.parentOf(root.folder)
            }
        }
    }

    ListView {
        id: view
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        // Without this the rows swallow the drag and the list never scrolls.
        pressDelay: 150
        model: root.entries

        delegate: Item {
            width: view.width
            height: AppTheme.itemSizeSmall

            Rectangle {
                anchors.fill: parent
                color: AppTheme.highlightBackgroundColor
                opacity: area.pressed ? AppTheme.highlightBackgroundOpacity : 0
            }

            TintedImage {
                id: icon
                anchors { left: parent.left; leftMargin: AppTheme.horizontalPageMargin
                          verticalCenter: parent.verticalCenter }
                width: AppTheme.iconSizeMedium
                height: AppTheme.iconSizeMedium
                source: modelData.isDir ? "image://theme/icon-m-file-archive-folder"
                                        : (modelData.suffix === "pdf" ? "image://theme/icon-m-file-pdf"
                                           : "image://theme/icon-m-file-document")
            }

            Text {
                anchors { left: icon.right; leftMargin: AppTheme.paddingMedium
                          right: size.left; rightMargin: AppTheme.paddingMedium
                          verticalCenter: parent.verticalCenter }
                text: modelData.name
                elide: Text.ElideRight
                color: AppTheme.primaryColor
                font.pixelSize: AppTheme.fontSizeSmall
            }

            Text {
                id: size
                anchors { right: parent.right; rightMargin: AppTheme.horizontalPageMargin
                          verticalCenter: parent.verticalCenter }
                text: modelData.size
                color: AppTheme.secondaryColor
                font.pixelSize: AppTheme.fontSizeExtraSmall
            }

            MouseArea {
                id: area
                anchors.fill: parent
                onClicked: {
                    if (modelData.isDir)
                        root.folder = modelData.path
                    else {
                        root.selected(modelData.path)
                        pageStack.pop()
                    }
                }
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: root.entries.length === 0
        text: qsTr("Nothing to print here")
        color: AppTheme.secondaryColor
        font.pixelSize: AppTheme.fontSizeSmall
    }
}
