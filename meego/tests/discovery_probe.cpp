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
// stand-ins, all on the device. With a file as well it prints it, which is
// the only way to watch a job go out without driving the app by hand.
//
// Build it with "meego/build.sh probe" and copy it to the device:
//   ./discovery_probe [seconds] [ipp://host/path] [file]
#include <QCoreApplication>
#include <QStringList>
#include <QTimer>
#include <QtDebug>

#include <iostream>

#include <src/ippdiscovery.h>
#include <src/ippprinter.h>

// The worker thread is reached through queued connections, which refuse to
// carry a type Qt has not been told about -- without these the request is
// simply never made and the printer stays silent.
Q_DECLARE_METATYPE(CURLcode)
Q_DECLARE_METATYPE(Bytestream)
Q_DECLARE_METATYPE(PrintParameters)

class Watcher : public QObject
{
    Q_OBJECT

public:
    Watcher(int seconds, const QString& url = QString(), const QString& file = QString(),
            const QString& format = QString(), const QString& pages = QString())
        : _left(seconds), _printer(0), _file(file), _format(format), _pages(pages), _printing(false)
    {
        if(!url.isEmpty())
        {
            _printer = new IppPrinter();
            connect(_printer, SIGNAL(jobFinished(bool)), this, SLOT(jobFinished(bool)));
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

public slots:
    void jobFinished(bool status)
    {
        std::cout << "job finished, status " << (status ? "ok" : "failed") << std::endl;
        const QVariantMap jobAttrs = _printer->jobAttrs();
        for(const QString& key : jobAttrs.keys())
        {
            std::cout << "    " << qPrintable(key) << ": "
                      << qPrintable(jobAttrs.value(key).toMap().value("value").toString())
                      << std::endl;
        }
        QCoreApplication::quit();
    }

private slots:
    void tick()
    {
        if(_printer)
        {
            const QVariantMap attrs = _printer->attrs();
            std::cout << "[" << _left << "s] " << attrs.size() << " attribute(s)";
            if(!attrs.isEmpty() && !_file.isEmpty() && !_printing)
            {
                // The attributes are in; print the file with the printer's
                // own defaults, exactly as the busy page does.
                _printing = true;
                QVariantMap jobParams;
                if(!_format.isEmpty())
                {
                    // What the settings page calls the transfer format. Left
                    // out, SeaPrint tells the printer application/octet-stream
                    // and lets it work the format out for itself.
                    QVariantMap value;
                    value.insert("tag", int(IppMsg::MimeMediaType));
                    value.insert("value", _format);
                    jobParams.insert("document-format", value);
                }
                if(!_pages.isEmpty())
                {
                    // What the page-range setting builds: a list of
                    // {low, high}, tagged rangeOfInteger.
                    QVariantList ranges;
                    for(const QString& part : _pages.split(","))
                    {
                        const QStringList ends = part.split("-");
                        QVariantMap range;
                        range.insert("low", ends.first().toInt());
                        range.insert("high", ends.last().toInt());
                        ranges.append(range);
                    }
                    QVariantMap value;
                    value.insert("tag", int(IppMsg::IntegerRange));
                    value.insert("value", ranges);
                    jobParams.insert("page-ranges", value);
                }
                std::cout << "printing " << qPrintable(_file)
                          << (_format.isEmpty() ? " (format: auto)"
                                                : qPrintable(" (format: " + _format + ")"))
                          << (_pages.isEmpty() ? "" : qPrintable(", pages " + _pages))
                          << std::endl;
                _printer->print(jobParams, _file);
                return;
            }
            if(_printing)
            {
                std::cout << "[" << _left << "s] waiting for the job" << std::endl;
                if(--_left <= 0)
                {
                    QCoreApplication::quit();
                }
                return;
            }
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
    QString _file;
    QString _format;
    QString _pages;
    bool _printing;
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("net.attah");
    app.setApplicationName("harbour-seaprint");

    qRegisterMetaType<CURLcode>("CURLcode");
    qRegisterMetaType<Bytestream>("Bytestream");
    qRegisterMetaType<PrintParameters>("PrintParameters");
    qRegisterMetaType<IppMsg>("IppMsg");
    qRegisterMetaType<QMargins>("QMargins");

    const int seconds = argc > 1 ? QString::fromLocal8Bit(argv[1]).toInt() : 10;
    const QString url = argc > 2 ? QString::fromLocal8Bit(argv[2]) : QString();
    const QString file = argc > 3 ? QString::fromLocal8Bit(argv[3]) : QString();
    const QString format = argc > 4 ? QString::fromLocal8Bit(argv[4]) : QString();
    const QString pages = argc > 5 ? QString::fromLocal8Bit(argv[5]) : QString();

    Watcher watcher(seconds, url, file, format, pages);
    if(url.isEmpty())
    {
        IppDiscovery::instance()->discover();
    }

    return app.exec();
}

#include "discovery_probe.moc"
