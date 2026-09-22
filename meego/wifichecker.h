// What network the phone is on, for the QML that keeps a favourite printer
// list per WLAN.
//
// Sailfish asks connman over D-Bus; Harmattan's connection manager is ICd2 and
// speaks a different language, so this asks the kernel directly: the wireless
// extensions give the SSID of the associated network, and the interface flags
// say whether it is actually up. No new library on the device, and it works
// the same whether the connection was brought up by the UI or by hand.
#ifndef WIFICHECKER_H
#define WIFICHECKER_H

#include <QObject>
#include <QString>
#include <QTimer>

class WifiChecker : public QObject
{
    Q_OBJECT
    // One signal per property, not a shared changed(): the pages listen with
    // Connections { target: wifi; onConnectedChanged: ...; onSsidChanged: ... },
    // and those handlers are looked up as signals of those exact names.
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString ssid READ ssid NOTIFY ssidChanged)

public:
    explicit WifiChecker(QObject* parent = 0);

    bool connected() const { return _connected; }
    QString ssid() const { return _ssid; }

signals:
    void connectedChanged();
    void ssidChanged();

private slots:
    void poll();

private:
    bool _connected;
    QString _ssid;
    QTimer _timer;
};

#endif // WIFICHECKER_H
