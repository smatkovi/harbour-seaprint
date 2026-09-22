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
    Q_PROPERTY(bool connected READ connected NOTIFY changed)
    Q_PROPERTY(QString ssid READ ssid NOTIFY changed)

public:
    explicit WifiChecker(QObject* parent = 0);

    bool connected() const { return _connected; }
    QString ssid() const { return _ssid; }

signals:
    void changed();

private slots:
    void poll();

private:
    bool _connected;
    QString _ssid;
    QTimer _timer;
};

#endif // WIFICHECKER_H
