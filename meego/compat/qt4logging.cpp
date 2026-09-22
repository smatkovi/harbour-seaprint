// QLoggingCategory for Qt 4.7: one switch for qDebug(), which is all the app
// asks of it. Without the handler every qDebug() would go to the journal even
// with the debug-log setting off -- on this device that is both slow and, for
// a printing app, more than the user asked to have written down.
#include "QLoggingCategory"

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <cstdio>

namespace {

bool g_debugEnabled = true;

// Started from the launcher, an app on this device has no console to write to:
// its output goes nowhere anyone can read. So when the debug-log setting is on
// the same lines also go to a file, which is the only way to see what happened
// on a phone that is being used by hand. It is truncated at every start, so it
// cannot grow without end.
QFile* logFile()
{
    static QFile* file = 0;
    static bool tried = false;
    if(!tried)
    {
        tried = true;
        const QString path = QDir::homePath() + "/.cache/harbour-seaprint.log";
        QDir().mkpath(QDir::homePath() + "/.cache");
        file = new QFile(path);
        if(!file->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            delete file;
            file = 0;
        }
    }
    return file;
}

void messageHandler(QtMsgType type, const char* msg)
{
    if(type == QtDebugMsg && !g_debugEnabled)
    {
        return;
    }
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);

    if(g_debugEnabled)
    {
        if(QFile* file = logFile())
        {
            file->write(QDateTime::currentDateTime().toString("hh:mm:ss ").toUtf8());
            file->write(msg);
            file->write("\n");
            file->flush();
        }
    }

    if(type == QtFatalMsg)
    {
        abort();
    }
}

}

QLoggingCategory* QLoggingCategory::defaultCategory()
{
    static QLoggingCategory category;
    static bool installed = false;
    if(!installed)
    {
        qInstallMsgHandler(messageHandler);
        installed = true;
    }
    return &category;
}

void QLoggingCategory::setEnabled(QtMsgType type, bool enabled)
{
    if(type == QtDebugMsg)
    {
        g_debugEnabled = enabled;
    }
}

bool QLoggingCategory::isEnabled(QtMsgType type)
{
    return type == QtDebugMsg ? g_debugEnabled : true;
}
