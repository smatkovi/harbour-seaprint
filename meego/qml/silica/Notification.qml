import QtQuick 1.1
import com.nokia.meego 1.0 as Meego
// Nemo.Notifications' Notification, as the app uses it: a short message. On
// Harmattan that is an InfoBanner rather than a system notification -- the app
// only ever says things about what the user is doing right now.
Meego.InfoBanner {
    id: root
    property int expireTimeout: 4000
    property string body: ""
    property string previewBody: ""
    property string summary: ""

    timerShowTime: expireTimeout
    timerEnabled: true
    text: body !== "" ? body : previewBody

    function publish() {
        text = body !== "" ? body : previewBody
        show()
    }
    function close() {
        hide()
    }
}
