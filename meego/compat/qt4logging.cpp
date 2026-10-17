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
#include <unistd.h>

namespace {

bool g_debugEnabled = true;
bool g_stderrGoesToLog = false;

// Started from the launcher, an app on this device has no console to write to:
// its output goes nowhere anyone can read. So when the debug-log setting is on
// the same lines also go to a file, which is the only way to see what happened
// on a phone that is being used by hand. It is truncated at every start, so it
// cannot grow without end -- but the run before is kept as .log.1: an app that
// dies is started again, and the start is what wipes the evidence of the
// death. One spare run is enough to read what happened and still bounded.
QFile* logFile()
{
    static QFile* file = 0;
    static bool tried = false;
    if(!tried)
    {
        tried = true;
        const QString path = QDir::homePath() + "/.cache/harbour-seaprint.log";
        QDir().mkpath(QDir::homePath() + "/.cache");
        QFile::remove(path + ".1");
        QFile::rename(path, path + ".1");
        file = new QFile(path);
        if(!file->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            delete file;
            file = 0;
        }
        else
        {
            // curl's own output, poppler's warnings and whatever a dying
            // runtime says on its way out all go to stderr rather than
            // through Qt's handler, so they were simply lost. Point stderr at
            // the same file and they land in the log, in order, next to the
            // app's own lines.
            fflush(stderr);
            if(dup2(file->handle(), fileno(stderr)) != -1)
            {
                g_stderrGoesToLog = true;
                setvbuf(stderr, 0, _IONBF, 0);
            }
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
    QFile* file = g_debugEnabled ? logFile() : 0;
    if(file)
    {
        file->write(QDateTime::currentDateTime().toString("hh:mm:ss ").toUtf8());
        file->write(msg);
        file->write("\n");
        file->flush();
    }

    if(!g_stderrGoesToLog)
    { // No log file taking it over: on the desktop, and with the log switched
      // off, stderr is the only place these lines can go.
        fprintf(stderr, "%s\n", msg);
        fflush(stderr);
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
