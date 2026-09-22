// Loads every ported QML file and prints what the engine says about it.
//
// The N9 is the only place the real com.nokia.meego can be instantiated, so
// this puts stand-ins for those components on the import path
// (meego/tests/stubs) and runs the app's own QML against them. That does not
// prove the pages look right -- it proves they parse, that no property is
// assigned that does not exist, and that the C++ types, context properties and
// Silica stand-ins line up with what the pages expect. Those are the mistakes
// a mechanical port makes by the dozen.
//
//   meego/tests/check-qml.sh
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeError>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QtDebug>

#include <cstdio>

#include "compat/mgconfitem.h"
#include "filebrowser.h"
#include "ipptags.h"
#include "tintedimage.h"
#include "wifichecker.h"

#include <src/convertchecker.h>
#include <src/imageitem.h>
#include <src/ippdiscovery.h>
#include <src/ippprinter.h>
#include <src/mimer.h>
#include <src/rangelistchecker.h>
#include <src/settings.h>

// ConvertChecker dlopen()s poppler in its constructor and throws if it is not
// there. That is fatal on a device without libpoppler-glib, and it is simply
// the normal case on the build machine, where the QML checker runs -- so the
// failure is caught and the app carries on without the page count it would
// have used for the page-range setting.
static ConvertChecker* convertCheckerOrNull()
{
    try
    {
        return ConvertChecker::instance();
    }
    catch(const std::exception& e)
    {
        qWarning() << "no PDF support:" << e.what();
        return 0;
    }
}

static QObject* instantiate(QDeclarativeEngine* engine, const QString& file)
{
    QDeclarativeComponent* component =
        new QDeclarativeComponent(engine, QUrl::fromLocalFile(file), engine);
    QObject* object = component->create(engine->rootContext());
    if(object)
    {
        object->setParent(engine);
        QDeclarativeEngine::setObjectOwnership(object, QDeclarativeEngine::CppOwnership);
    }
    return object;
}

int main(int argc, char* argv[])
{
    // No GUI: the checker never shows a window, and the build machine has no
    // display. QDeclarativeItem is a QGraphicsObject, which needs none.
    QApplication app(argc, argv, false);

    const QString qmlDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QString("meego/qml");
    const QString stubs = argc > 2 ? QString::fromLocal8Bit(argv[2]) : QString("meego/tests/stubs");

    qmlRegisterType<IppPrinter>("seaprint.ippprinter", 1, 0, "IppPrinter");
    qmlRegisterType<ImageItem>("seaprint.imageitem", 1, 0, "ImageItem");
    qmlRegisterType<TintedImage>("seaprint.tintedimage", 1, 0, "TintedImage");
    qmlRegisterType<WifiChecker>("seaprint.wifichecker", 1, 0, "WifiCheckerItem");
    qmlRegisterType<MGConfItem>("Nemo.Configuration", 1, 0, "ConfigurationValue");
    qmlRegisterUncreatableType<IppTags>("seaprint.ippmsg", 1, 0, "IppMsg", "enum only");

    QDeclarativeEngine engine;
    engine.addImportPath(stubs);
    engine.addImportPath(qmlDir);

    QDeclarativeContext* context = engine.rootContext();
    context->setContextProperty("IppDiscovery", IppDiscovery::instance());
    context->setContextProperty("Mimer", Mimer::instance());
    context->setContextProperty("ConvertChecker", convertCheckerOrNull());
    context->setContextProperty("SeaPrintSettings", Settings::instance());
    context->setContextProperty("RangeListChecker", RangeListChecker::instance());
    context->setContextProperty("FileBrowser", new FileBrowser(&engine));
    context->setContextProperty("appVersion", QString("check"));
    context->setContextProperty("appDataDir", qmlDir);

    QVariantMap dialogResult;
    dialogResult.insert("Accepted", 1);
    dialogResult.insert("Rejected", 0);
    context->setContextProperty("DialogResult", dialogResult);

    QVariantMap pageOrientation;
    pageOrientation.insert("Automatic", 0);
    pageOrientation.insert("LockPortrait", 1);
    pageOrientation.insert("LockLandscape", 2);
    context->setContextProperty("PageOrientation", pageOrientation);

    QVariantMap pageStatus;
    pageStatus.insert("Inactive", 0);
    pageStatus.insert("Activating", 1);
    pageStatus.insert("Active", 2);
    pageStatus.insert("Deactivating", 3);
    context->setContextProperty("PageStatus", pageStatus);

    QVariantMap theme;
    theme.insert("inverted", true);
    context->setContextProperty("theme", theme);

    context->setContextProperty("AppTheme", instantiate(&engine, qmlDir + "/context/Theme.qml"));

    // Every file, so that a page nobody reaches from the root is checked too.
    QStringList files;
    files << qmlDir + "/harbour-seaprint.qml";
    const char* subdirs[] = {"silica", "pages", "components", 0};
    for(const char** sub = subdirs; *sub; sub++)
    {
        QDir dir(qmlDir + "/" + *sub);
        for(const QString& name : dir.entryList(QStringList() << "*.qml", QDir::Files, QDir::Name))
        {
            files << dir.absoluteFilePath(name);
        }
    }

    int failed = 0;
    for(const QString& file : files)
    {
        QDeclarativeComponent component(&engine, QUrl::fromLocalFile(file));
        if(component.isError())
        {
            failed++;
            printf("FAIL %s\n", qPrintable(QFileInfo(file).fileName()));
            for(const QDeclarativeError& error : component.errors())
            {
                printf("     %s\n", qPrintable(error.toString()));
            }
            continue;
        }
        printf("ok   %s\n", qPrintable(QFileInfo(file).fileName()));
    }

    printf("\n%d of %d files failed to load\n", failed, files.size());
    return failed == 0 ? 0 : 1;
}
