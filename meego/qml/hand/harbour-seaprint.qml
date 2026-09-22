import QtQuick 1.1
import com.nokia.meego 1.0
import Nemo.Configuration 1.0
import "silica"
import "pages"
import "components"

// The MeeGo edition of SeaPrint's root file.
//
// Upstream is a Silica ApplicationWindow with a cover page, a Nemo
// notification and a QtQuick.LocalStorage database. Here it is a
// PageStackWindow; the cover is gone (Harmattan has no cover pages), the
// notification is an InfoBanner (meego/qml/silica/Notification.qml), and the
// database is the same SQL through QML 1's own openDatabaseSync().
PageStackWindow {
    id: appWin

    // The pages are drawn for a dark scheme, like Silica's.
    Component.onCompleted: theme.inverted = true

    showStatusBar: true
    showToolBar: false

    property variant _mainPage

    // Harmattan has no calligraconverter and no package to install one, so the
    // office formats are simply not offered and the nag screen never shows.
    property bool expectCalligra: false

    property string selectedFile: ""
    property string selectedFileType: ""

    property string busyMessage: ""
    property string progress: ""

    function openFile(file) {
        selectedFile = file
        selectedFileType = Mimer.get_type(selectedFile);
        if(selectedFileType == "")
        {
            selectedFile = ""
            notifier.notify(qsTr("Unsupported document format"))
        }
        if(pageStack.currentPage && pageStack.currentPage.busyPage)
        {
            notifier.notify(qsTr("New file selected"))
        }
        else if(appWin._mainPage)
        {
            pageStack.pop(appWin._mainPage, true)
        }
    }

    // The D-Bus adaptors call this when another app hands SeaPrint a file
    // while it is already running.
    function activate() {
    }

    initialPage: Component {
        FirstPage {
            Component.onCompleted: appWin._mainPage = this
        }
    }

    WifiChecker {
        id: wifi
    }

    Item {
        id: db

        // The database handle is not kept in a property: a `property variant`
        // copies what is put into it (QtQuick 1.1 has no `property var`), and
        // a copied SQL handle is of no use. openDatabaseSync() hands out the
        // same database every time, so it is called where it is needed.
        function conn() {
            return openDatabaseSync("SeaprintDB", "1.0", "Seaprint storage", 100000)
        }

        Component.onCompleted: {
            conn().transaction(function (tx) {
                tx.executeSql('CREATE TABLE IF NOT EXISTS Favourites (ssid STRING, url STRING)');
                tx.executeSql('CREATE TABLE IF NOT EXISTS JobSettings (uuid STRING, type STRING, data STRING)');
            });
        }

        function addFavourite(ssid, url) {
            if(isFavourite(ssid, url))
                return;
            conn().transaction(function (tx) {
                tx.executeSql('INSERT INTO Favourites VALUES(?, ?)', [ssid, url] );
            });
        }

        function getFavourites(ssid) {
            var favs = [];
            conn().transaction(function (tx) {
                var res = tx.executeSql('SELECT * FROM Favourites WHERE ssid=?', [ssid]);
                for (var i = 0; i < res.rows.length; i++) {
                    favs.push(res.rows.item(i).url);
                }
            });
            return favs
        }

        function isFavourite(ssid, url) {
            var isfav = false;
            conn().transaction(function (tx) {
                var res = tx.executeSql('SELECT * FROM Favourites WHERE ssid=? AND url=?', [ssid, url]);
                if (res.rows.length > 0) {
                    isfav = true;
                }
            });
            return isfav
        }

        function removeFavourite(ssid, url) {
            conn().transaction(function (tx) {
                tx.executeSql('DELETE FROM Favourites WHERE ssid=? AND url=?', [ssid, url] );
            });
        }

        function simplifyType(mimetype) {
            if(mimetype == Mimer.SVG)
            {
                return {simple: "svg", translatable: qsTr("SVGs")};
            }
            else if(Mimer.isImage(mimetype))
            {
                return {simple: "image", translatable: qsTr("images")};
            }
            else if(mimetype == Mimer.Plaintext)
            {
                return {simple: "plaintext", translatable: qsTr("plaintext")};
            }
            else
            {
                return {simple: "document", translatable: qsTr("documents")};
            }
        }

        function setJobSettings(uuid, mimetype, settings) {
            var type = simplifyType(mimetype).simple;
            conn().transaction(function (tx) {
                tx.executeSql('DELETE FROM JobSettings WHERE uuid=? AND type=?', [uuid, type] );
                tx.executeSql('INSERT INTO JobSettings VALUES(?, ?, ?)', [uuid, type, settings] );
            });
        }

        function getJobSettings(uuid, mimetype) {
            var type = simplifyType(mimetype).simple;
            var settings = "{}";
            conn().transaction(function (tx) {
                var res = tx.executeSql('SELECT * FROM JobSettings WHERE uuid=? AND type=?', [uuid, type]);
                if (res.rows.length)
                {
                    settings = res.rows.item(0).data
                }
            });
            return settings
        }

        function removeJobSettings(uuid, mimetype) {
            var type = simplifyType(mimetype).simple;
            conn().transaction(function (tx) {
                tx.executeSql('DELETE FROM JobSettings WHERE uuid=? AND type=?', [uuid, type] );
            });
        }
    }

    Notification {
        id: notifier

        expireTimeout: 4000

        function notify(data) {
            body = data
            previewBody = data
            publish()
        }
    }

    ConfigurationValue
    {
        id: nagScreenSetting
        key: "/apps/harbour-seaprint/settings/nag-screen"
        defaultValue: 0
        property int expectedValue: expectCalligra ? 2 : 1
    }

    ConfigurationValue
    {
        id: considerAdditionalFormatsSetting
        key: "/apps/harbour-seaprint/settings/consider-additional-formats"
        defaultValue: true
    }

    ConfigurationValue
    {
        id: ignoreSslErrorsSetting
        key: SeaPrintSettings.ignoreSslErrorsPath
        defaultValue: SeaPrintSettings.ignoreSslErrorsDefault
    }

    ConfigurationValue
    {
        id: debugLogSetting
        key: SeaPrintSettings.debugLogPath
        defaultValue: SeaPrintSettings.debugLogDefault
    }

    ConfigurationValue
    {
        id: allowExternalConnectionsSetting
        key: SeaPrintSettings.allowExternalConnectionsPath
        defaultValue: SeaPrintSettings.allowExternalConnectionsDefault
    }
}
