import QtQuick 1.1

// Silica's Theme for Harmattan. main.cpp puts this object in the root context
// as "AppTheme" -- not as "Theme", because com.nokia.meego exports a Theme of
// its own that beats a context property of the same name, which is what left
// an earlier port of another app completely unstyled.
//
// The numbers are the Sailfish defaults at their 540 px reference width,
// scaled to the N9's 480, so the pages keep the proportions they were drawn
// with. The colours are a dark scheme, like Silica's, in SeaPrint's blue.
QtObject {
    property real screenWidth: 480
    property real screenHeight: 854
    property real ratio: screenWidth / 540

    property real paddingSmall: 6 * ratio
    property real paddingMedium: 12 * ratio
    property real paddingLarge: 24 * ratio
    property real horizontalPageMargin: 24 * ratio

    property real itemSizeExtraSmall: 70 * ratio
    property real itemSizeSmall: 80 * ratio
    property real itemSizeMedium: 100 * ratio
    property real itemSizeLarge: 110 * ratio
    property real itemSizeExtraLarge: 135 * ratio
    property real itemSizeHuge: 180 * ratio

    property real iconSizeSmall: 32 * ratio
    property real iconSizeMedium: 48 * ratio
    property real iconSizeLarge: 64 * ratio
    property real iconSizeExtraLarge: 86 * ratio

    property real fontSizeTiny: 16 * ratio
    property real fontSizeExtraSmall: 20 * ratio
    property real fontSizeSmall: 24 * ratio
    property real fontSizeMedium: 28 * ratio
    property real fontSizeLarge: 36 * ratio
    property real fontSizeExtraLarge: 44 * ratio
    property real fontSizeHuge: 60 * ratio

    property color primaryColor: "#ffffff"
    property color secondaryColor: "#99ffffff"
    property color highlightColor: "#6fc2e0"
    property color secondaryHighlightColor: "#4d8296"
    property color highlightBackgroundColor: "#6fc2e0"
    property real highlightBackgroundOpacity: 0.3
    property color errorColor: "#ff4d4d"
    property color backgroundColor: "#000000"

    // Silica has light and dark schemes; this port is dark throughout, so the
    // constant is here only so that the pages can name it.
    property int colorScheme: 0
    property int lightOnDark: 0
    property int darkOnLight: 1

    function rgba(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    // On Silica this picks a background tone to go with the given colour and
    // scheme; one dark scheme needs no choice.
    function highlightBackgroundFromColor(color, scheme) {
        return color
    }
}
