#include "wifichecker.h"

#include <QNetworkInterface>
#include <QtDebug>

// Header order matters: <sys/socket.h> has to define struct sockaddr before
// the kernel headers describe the interface request structs with one inside,
// and <net/if.h> must not be pulled in next to <linux/if.h> -- they define the
// same structs.
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

namespace {

// The SSID of an associated wireless interface, empty if it is not associated.
QString ssidOf(const QString& name)
{
    const int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        return QString();
    }

    char essid[IW_ESSID_MAX_SIZE+1];
    memset(essid, 0, sizeof(essid));

    struct iwreq request;
    memset(&request, 0, sizeof(request));
    strncpy(request.ifr_name, name.toLatin1().constData(), IFNAMSIZ-1);
    request.u.essid.pointer = essid;
    request.u.essid.length = IW_ESSID_MAX_SIZE;
    request.u.essid.flags = 0;

    QString result;
    if(ioctl(sock, SIOCGIWESSID, &request) == 0)
    {
        result = QString::fromUtf8(essid, request.u.essid.length);
    }
    close(sock);
    return result;
}

}

WifiChecker::WifiChecker(QObject* parent) : QObject(parent), _connected(false)
{
    connect(&_timer, SIGNAL(timeout()), this, SLOT(poll()));
    _timer.start(5000);
    poll();
}

void WifiChecker::poll()
{
    bool connected = false;
    QString ssid;

    for(const QNetworkInterface& interface : QNetworkInterface::allInterfaces())
    {
        if(!(interface.flags() & QNetworkInterface::IsUp) ||
           !(interface.flags() & QNetworkInterface::IsRunning) ||
           (interface.flags() & QNetworkInterface::IsLoopBack))
        {
            continue;
        }

        bool hasAddress = false;
        for(const QNetworkAddressEntry& entry : interface.addressEntries())
        {
            if(!entry.ip().isNull())
            {
                hasAddress = true;
                break;
            }
        }
        if(!hasAddress)
        {
            continue;
        }

        const QString name = interface.name();
        const QString interfaceSsid = ssidOf(name);
        if(!interfaceSsid.isEmpty())
        {
            connected = true;
            ssid = interfaceSsid;
            break;
        }
    }

    const bool connectedChangedNow = connected != _connected;
    const bool ssidChangedNow = ssid != _ssid;

    if(connectedChangedNow || ssidChangedNow)
    {
        _connected = connected;
        _ssid = ssid;
        qDebug() << "wifi" << _connected << _ssid;
    }
    // The favourites are keyed by network name, so the pages want to know
    // about the two changes separately.
    if(connectedChangedNow)
    {
        emit connectedChanged();
    }
    if(ssidChangedNow)
    {
        emit ssidChanged();
    }
}
