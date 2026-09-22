import QtQuick 1.1
import seaprint.wifichecker 1.0

// Upstream asks connman over D-Bus what wireless network the phone is on.
// Harmattan's connection manager is ICd2 and speaks a different language, so
// the same two answers come from the kernel instead -- see meego/wifichecker.cpp.
WifiCheckerItem {
}
