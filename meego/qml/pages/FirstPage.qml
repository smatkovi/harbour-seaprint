import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import seaprint.ippprinter 1.0
import seaprint.imageitem 1.0
import "utils.js" as Utils

Page {
    id: page
    orientationLock: PageOrientation.Automatic
    Connections {
        target: wifi
        onConnectedChanged: {
            if(wifi.connected) {
                var favourites = db.getFavourites(wifi.ssid);
                IppDiscovery.favourites = favourites;
            }
            else {
                IppDiscovery.favourites = []
            }
        }

        property bool initialSSIDchange: true

        onSsidChanged: {
            if(!initialSSIDchange)
            {
                IppDiscovery.reset();
            }
            initialSSIDchange = false;
        }
    }

    Label {
        text: qsTr("Not on WiFi")
        color: AppTheme.highlightColor
        anchors.centerIn: parent
        visible: !wifi.connected
    }

    property bool initialized: false
    onVisibleChanged: {
        if (visible)
        {
            if(initialized)
                page.refreshed()
            initialized = true
        }
    }

    signal refreshed()
    signal noFileSelected()

    Component.onCompleted: {
        IppDiscovery.discover();
    }

    property bool nagged: false

    onStatusChanged: {
        if(status==PageStatus.Active && !nagged && nagScreenSetting.value != nagScreenSetting.expectedValue)
        {
            if(expectCalligra && !ConvertChecker.calligra)
            {
                nagged=true
                pageStack.push(Qt.resolvedUrl("NagScreen.qml"))
            }
        }
    }

    // To enable PullDownMenu, place our content in a Flickable
    Flickable {
        anchors.top: parent.top
        width: parent.width
        anchors.bottom: fileDock.top
        clip: true

        // PullDownMenu and PushUpMenu must be declared in Flickable, ListView or GridView
        PullDownMenu {
            MenuItem {
                text: qsTr("About SeaPrint")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Add by URL")
                enabled: wifi.connected
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("AddPrinterDialog.qml"));
                        dialog.accepted.connect(function() {
                            db.addFavourite(wifi.ssid, dialog.value);
                            IppDiscovery.favourites = db.getFavourites(dialog.ssid);
                    })
                }
            }
            MenuItem {
                text: qsTr("Refresh")
                onClicked: {
                    IppDiscovery.discover();
                    page.refreshed();
                }
            }
        }

        ListView {
            id: listView
            anchors.fill: parent
            model: IppDiscovery
            spacing: AppTheme.paddingSmall


            header: PageHeader {
                id: pageHeader
                title: "SeaPrint"
                description: qsTr("Available printers")
            }

            delegate: ListItem {
                id: delegate
                contentItem.height: visible ? Math.max(column.implicitHeight, AppTheme.itemSizeLarge+2*AppTheme.paddingMedium) : 0

                IppPrinter {
                    id: printer
                    url: model.display
                }

                visible: Object.keys(printer.attrs).length !== 0

                property string name: Utils.unknownForEmptyString(printer.attrs["printer-name"].value)
                property variant supported_formats: Utils.supported_formats(printer, considerAdditionalFormatsSetting.value)
                property bool canPrint: supported_formats.mimetypes.indexOf(appWin.selectedFileType) != -1

                Connections {
                    target: printer
                    onAttrsChanged: {
                        if(Object.keys(printer.attrs).length === 0) {
                            delegate.visible = false
                        }
                        else {
                            delegate.visible = true
                        }
                    }
                }


                Connections {
                    target: page
                    onRefreshed: {
                        printer.refresh()
                    }
                }

                Timer
                {
                    id: debugCountReset
                    interval: 666
                    repeat: false
                    onTriggered:
                    {
                        debugCount = 0;
                    }
                }

                property int debugCount: 0

                function maybeGetParams()
                {
                    if(printer.attrs.hasOwnProperty("printer-uuid"))
                    {
                        return JSON.parse(db.getJobSettings(printer.attrs["printer-uuid"].value, appWin.selectedFileType));
                    }
                    else
                    {
                        return new Object();
                    }
                }


                onClicked: {

                    if(++debugCount == 5)
                    {
                        pageStack.push(Qt.resolvedUrl("DebugPage.qml"), {printer: printer})
                        return;
                    }
                    debugCountReset.restart();
                    if(appWin.selectedFile == "")
                    {
                        noFileSelected();
                    }
                    else if(!canPrint)
                    {
                        return;
                    }
                    else
                    {
                        pageStack.push(Qt.resolvedUrl("PrinterPage.qml"), {printer: printer, selectedFile: appWin.selectedFile, jobParams: maybeGetParams()})
                    }
                }

                ImageItem {
                    id: icon
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.topMargin: AppTheme.paddingMedium
                    anchors.leftMargin: AppTheme.paddingMedium

                    height: AppTheme.itemSizeLarge
                    width: AppTheme.itemSizeLarge
                    image: printer.icon

                    Image {
                        id: placeholder
                        anchors.fill: parent
                        sourceSize.height: height
                        sourceSize.width: width

                        visible: !parent.valid
                        source: "icon-seaprint-nobg.svg"
                    }
                }

                Image {
                    id: warningIcon
                    source: "image://theme/icon-lock-warning"
                    anchors.bottom: icon.bottom
                    anchors.right: icon.right
                    visible: Utils.isWaringState(printer)
                }

                Column {
                    id: column
                    anchors.left: icon.right
                    anchors.leftMargin: AppTheme.paddingMedium

                    Label {
                        id: name_label
                        color: canPrint || appWin.selectedFile == "" ? AppTheme.primaryColor : AppTheme.secondaryColor
                        text: name
                    }

                    Label {
                        id: mm_label
                        color: canPrint || appWin.selectedFile == "" ? AppTheme.primaryColor : AppTheme.secondaryColor
                        font.pixelSize: AppTheme.fontSizeExtraSmall
                        text: Utils.unknownForEmptyString(printer.attrs["printer-make-and-model"].value)
                              + (Utils.existsAndNotEmpty("printer-location", printer)  ? "  •  "+printer.attrs["printer-location"].value : "")
                    }

                    Label {
                        id: uri_label
                        color: canPrint || appWin.selectedFile == "" ? AppTheme.highlightColor : AppTheme.secondaryColor
                        font.pixelSize: AppTheme.fontSizeTiny
                        text: printer.url
                    }

                    Row {
                        spacing: AppTheme.paddingMedium
                        Label {
                            id: format_unsupported_label
                            visible: !supported_formats.pdf && !supported_formats.postscript && !supported_formats.plaintext
                                     && !supported_formats.office && !supported_formats.images
                            color: "red"
                            font.pixelSize: AppTheme.fontSizeExtraSmall
                            text: qsTr("No compatible formats supported")
                        }

                        HighlightImage {
                            height: AppTheme.itemSizeExtraSmall/2
                            width: AppTheme.itemSizeExtraSmall/2
                            visible: supported_formats.pdf
                            highlightColor: "red"
                            highlighted: !(appWin.selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-pdf"
                        }
                        HighlightImage {
                            height: AppTheme.itemSizeExtraSmall/2
                            width: AppTheme.itemSizeExtraSmall/2
                            visible: supported_formats.postscript
                            highlightColor: "red"
                            highlighted: !(appWin.selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-other"

                        }
                        HighlightImage {
                            height: AppTheme.itemSizeExtraSmall/2
                            width: AppTheme.itemSizeExtraSmall/2
                            visible: supported_formats.plaintext
                            highlightColor: "red"
                            highlighted: !(appWin.selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-document"
                        }
                        HighlightImage {
                            height: AppTheme.itemSizeExtraSmall/2
                            width: AppTheme.itemSizeExtraSmall/2
                            visible: supported_formats.office
                            highlightColor: "red"
                            highlighted: !(appWin.selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-formatted"
                        }
                        HighlightImage {
                            height: AppTheme.itemSizeExtraSmall/2
                            width: AppTheme.itemSizeExtraSmall/2
                            visible: supported_formats.office
                            highlightColor: "red"
                            highlighted: !(appWin.selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-presentation"
                        }
                        HighlightImage {
                            height: AppTheme.itemSizeExtraSmall/2
                            width: AppTheme.itemSizeExtraSmall/2
                            visible: supported_formats.images
                            highlightColor: "red"
                            highlighted: !(appWin.selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-image"
                        }
                    }

                }

                RemorseItem {
                    id: removeRemorse
                }

                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("View jobs")
                        onClicked:  pageStack.push(Qt.resolvedUrl("JobsPage.qml"), {printer: printer})
                    }
                    MenuItem {
                        text: qsTr("Printer details")
                        onClicked:  pageStack.push(Qt.resolvedUrl("DetailsPage.qml"), {printer: printer})
                    }
                    MenuItem {
                        text: qsTr("Remove printer")
                        visible: db.isFavourite(wifi.ssid, model.display)
                        onClicked: {
                            removeRemorse.execute(delegate, qsTr("Removing printer"),
                                                  function() {db.removeFavourite(wifi.ssid, model.display);
                                                              IppDiscovery.favourites = db.getFavourites(wifi.ssid)})
                        }
                    }
                }

            }
        }
    }
    Rectangle {
        id: fileDock
        height: panelColumn.implicitHeight
        width: parent.width
        anchors.bottom: parent.bottom

        gradient: Gradient {
            GradientStop { position: 0.0; color: AppTheme.rgba(AppTheme.highlightBackgroundColor, 0.15) }
            GradientStop { position: 1.0; color: AppTheme.rgba(AppTheme.highlightBackgroundColor, 0.3) }
        }

        Column {
            id: panelColumn
            width: parent.width
            spacing: AppTheme.paddingLarge
            anchors.topMargin: (page.height > page.width) && !warningRow.visible ? 2*AppTheme.paddingLarge : AppTheme.paddingLarge
            anchors.bottomMargin: (page.height > page.width) ? 2*AppTheme.paddingLarge : AppTheme.paddingLarge

            Row {
                id: warningRow
                anchors.horizontalCenter: parent.horizontalCenter
                visible: Mimer.isOffice(appWin.selectedFileType)

                HighlightImage {
                    source: "image://theme/icon-s-warning"
                    highlighted: true
                    highlightColor: AppTheme.highlightColor
                }
                Label
                {
                    text: qsTr("This format may not render correctly")
                    font.pixelSize: AppTheme.fontSizeExtraSmall
                    color: AppTheme.highlightColor
                }
            }

            Label {
                id: fileLabel
                width: parent.width
                anchors.leftMargin: AppTheme.paddingLarge
                anchors.rightMargin: AppTheme.paddingLarge

                horizontalAlignment: contentWidth > width ? Text.AlignRight : Text.AlignHCenter
                elide: Text.ElideRight
                text: appWin.selectedFile != "" ? appWin.selectedFile : qsTr("No file selected")

                SequentialAnimation {
                    id: noFileSelectedAnimation
                    loops: 3

                    ColorAnimation {target: fileLabel; property: "color"; from: AppTheme.primaryColor; to: AppTheme.highlightColor; duration: 200 }
                    ColorAnimation {target: fileLabel; property: "color"; from: AppTheme.highlightColor; to: AppTheme.primaryColor;  duration: 200 }
                }

                Connections {
                    target: page
                    onNoFileSelected: {
                        noFileSelectedAnimation.start()
                    }
                }

            }


            Row {
                width: parent.width
                IconButton {
                    id: folderButton
                    source: "image://theme/icon-m-file-document"
                    width: parent.width/2
                    onClicked: pageStack.push(documentPickerPage)
                }
                IconButton {
                    id: imageButton
                    source: "image://theme/icon-m-file-image"
                    width: parent.width/2
                    onClicked: pageStack.push(imagePickerPage)
                }

            }

        }
        Component {
            id: documentPickerPage
            FilePickerPage {
                title: qsTr("Choose file")
                // The formats this build can print; there is no
                // calligraconverter on Harmattan, so no office documents.
                suffixes: ["pdf", "ps", "txt", "text", "log", "md", "csv"]
                onSelected: appWin.openFile(path)
            }
        }
        Component {
            id: imagePickerPage
            FilePickerPage {
                title: qsTr("Choose image")
                suffixes: ["jpg", "jpeg", "png", "gif", "bmp", "tif", "tiff", "svg"]
                onSelected: appWin.openFile(path)
            }
        }
    }
}
