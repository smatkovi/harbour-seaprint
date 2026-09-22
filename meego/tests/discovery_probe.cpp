// Runs the app's own printer discovery, without a user interface.
//
// mDNS is the one part of SeaPrint that cannot be tried out anywhere but on
// the device itself: it depends on the phone's network, and the answer comes
// back to a socket that the code never binds. This puts IppDiscovery on a
// plain event loop, prints what it finds, and leaves qDebug() on, so the
// whole conversation with the printer shows up.
//
// With an address it asks that printer for its attributes instead, which is
// the rest of the path: the worker thread, curl, the IPP parser and the JSON
// stand-ins, all on the device.
//
// Build it with "meego/build.sh probe" and copy it to the device:
//   ./discovery_probe [seconds] [ipp://host/path]
#include <QCoreApplication>
#include <QStringList>
#include <QTimer>
#include <QtDebug>

#include <iostream>

#include <src/ippdiscovery.h>
#include <src/ippprinter.h>

class Watcher : public QObject
{
    Q_OBJECT

public:
    Watcher(int seconds, const QString& url = QString()) : _left(seconds), _printer(0)
    {
        if(!url.isEmpty())
        {
            _printer = new IppPrinter();
            _printer->setUrl(url);
        }
        connect(&_timer, SIGNAL(timeout()), this, SLOT(tick()));
        _timer.start(1000);
    }

    Watcher(int seconds) : _left(seconds), _printer(0)
    {
        connect(&_timer, SIGNAL(timeout()), this, SLOT(tick()));
        _timer.start(1000);
    }

private slots:
    void tick()
    {
        if(_printer)
        {
            const QVariantMap attrs = _printer->attrs();
            std::cout << "[" << _left << "s] " << attrs.size() << " attribute(s)";
            if(!attrs.isEmpty())
            {
                std::cout << "\n    printer-name: "
                          << qPrintable(attrs.value("printer-name").toMap().value("value").toString())
                          << "\n    make-and-model: "
                          << qPrintable(attrs.value("printer-make-and-model").toMap().value("value").toString())
                          << "\n    formats: "
                          << qPrintable(attrs.value("document-format-supported").toMap()
                                             .value("value").toStringList().join(", "));
                std::cout << std::endl;
                QCoreApplication::quit();
                return;
            }
            std::cout << std::endl;
            if(--_left <= 0)
            {
                QCoreApplication::quit();
            }
            return;
        }

        const QStringList found = IppDiscovery::instance()->stringList();
        std::cout << "[" << _left << "s] " << found.size() << " printer(s)";
        for(const QString& printer : found)
        {
            std::cout << "\n    " << qPrintable(printer);
        }
        std::cout << std::endl;

        if(--_left <= 0)
        {
            QCoreApplication::quit();
        }
    }

private:
    QTimer _timer;
    int _left;
    IppPrinter* _printer;
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("net.attah");
    app.setApplicationName("harbour-seaprint");

    const int seconds = argc > 1 ? QString::fromLocal8Bit(argv[1]).toInt() : 10;
    const QString url = argc > 2 ? QString::fromLocal8Bit(argv[2]) : QString();

    Watcher watcher(seconds, url);
    if(url.isEmpty())
    {
        IppDiscovery::instance()->discover();
    }

    return app.exec();
}

#include "discovery_probe.moc"
