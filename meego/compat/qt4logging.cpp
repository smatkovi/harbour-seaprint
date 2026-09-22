// QLoggingCategory for Qt 4.7: one switch for qDebug(), which is all the app
// asks of it. Without the handler every qDebug() would go to the journal even
// with the debug-log setting off -- on this device that is both slow and, for
// a printing app, more than the user asked to have written down.
#include "QLoggingCategory"

#include <QByteArray>
#include <cstdio>

namespace {

bool g_debugEnabled = true;

void messageHandler(QtMsgType type, const char* msg)
{
    if(type == QtDebugMsg && !g_debugEnabled)
    {
        return;
    }
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
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
