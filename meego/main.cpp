// SeaPrint for MeeGo Harmattan (Nokia N9 / N950).
//
// Upstream's src/harbour-seaprint.cpp builds a Sailfish app: SailfishApp for
// the view, qmlRegisterSingletonType for the five singletons. Qt 4.7 has
// neither, so this starts a QDeclarativeView and puts the singletons in the
// root context under the names the QML already uses -- a context property and
// an imported singleton are spelled exactly the same way at the point of use.
//
// Everything below the QML is the upstream code, unchanged apart from what
// meego/compat holds up.
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeError>
#include <QDeclarativeEngine>
#include <QDeclarativeView>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QTextCodec>
#include <QLocale>
#include <QTranslator>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <QtDebug>

#include <QDBusConnection>

#include "compat/QLoggingCategory"
#include "clipboard.h"
#include "compat/mgconfitem.h"
#include "filebrowser.h"
#include "ipptags.h"
#include "tintedimage.h"
#include "wifichecker.h"

#include <src/convertchecker.h>
#include <src/freedesktopdbusadaptor.h>
#include <src/imageitem.h>
#include <src/ippdiscovery.h>
#include <src/ippprinter.h>
#include <src/mimer.h>
#include <src/rangelistchecker.h>
#include <src/seaprintdbusadaptor.h>
#include <src/settings.h>

Q_DECLARE_METATYPE(CURLcode)
Q_DECLARE_METATYPE(Bytestream)
Q_DECLARE_METATYPE(PrintParameters)

// Where the QML and the translations are once the .deb is unpacked; the source
// tree is used instead when the binary is started from one (meego/run-x86.sh).
// A QML object from a file, used for the theme. It is parented to the engine
// rather than left on the stack: an object created by a QDeclarativeComponent
// lives in a context owned by that component, so letting the component die
// tears the context down and every property of the object reads as undefined
// afterwards -- a whole app in default fonts, with nothing in the log.
static QObject* instantiate(QDeclarativeEngine* engine, const QString& file)
{
    QDeclarativeComponent* component =
        new QDeclarativeComponent(engine, QUrl::fromLocalFile(file), engine);
    if(component->isError())
    {
        for(const QDeclarativeError& error : component->errors())
        {
            qWarning() << file << error.toString();
        }
        return 0;
    }
    QObject* object = component->create(engine->rootContext());
    if(object)
    {
        object->setParent(engine);
        QDeclarativeEngine::setObjectOwnership(object, QDeclarativeEngine::CppOwnership);
    }
    return object;
}

static QString dataDir()
{
    const QString installed = "/opt/harbour-seaprint";
    if(QFileInfo(installed + "/qml/harbour-seaprint.qml").exists())
    {
        return installed;
    }
    const QString here = QCoreApplication::applicationDirPath();
    for(QDir dir(here); !dir.isRoot(); dir.cdUp())
    {
        if(QFileInfo(dir.absolutePath() + "/meego/qml/harbour-seaprint.qml").exists())
        {
            return dir.absolutePath() + "/meego";
        }
    }
    return installed;
}

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

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Qt 4 pushes the source text of tr() through Latin-1 unless told
    // otherwise, which would mangle every accented string in the app.
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    app.setOrganizationName(QStringLiteral("net.attah"));
    app.setApplicationName(QStringLiteral("harbour-seaprint"));
    app.setApplicationVersion(QStringLiteral(SEAPRINT_VERSION));

    qRegisterMetaType<CURLcode>("CURLcode");
    qRegisterMetaType<Bytestream>("Bytestream");
    qRegisterMetaType<PrintParameters>("PrintParameters");
    qRegisterMetaType<IppMsg>("IppMsg");
    qRegisterMetaType<QMargins>("QMargins");

    // Turn qDebug() on or off according to the setting, as upstream does with
    // a logging category.
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, Settings::instance()->debugLog());

    const QString data = dataDir();

    QTranslator translator;
    // harbour-seaprint-de.qm and friends: QTranslator drops the country part
    // by itself if there is no catalogue for it.
    if(translator.load("harbour-seaprint-" + QLocale::system().name(), data + "/translations"))
    {
        app.installTranslator(&translator);
    }

    qmlRegisterType<IppPrinter>("seaprint.ippprinter", 1, 0, "IppPrinter");
    qmlRegisterType<ImageItem>("seaprint.imageitem", 1, 0, "ImageItem");
    qmlRegisterType<TintedImage>("seaprint.tintedimage", 1, 0, "TintedImage");
    qmlRegisterType<WifiChecker>("seaprint.wifichecker", 1, 0, "WifiCheckerItem");
    // "import Nemo.Configuration 1.0" keeps working: same name, same key
    // strings, a QSettings file underneath instead of GConf.
    qmlRegisterType<MGConfItem>("Nemo.Configuration", 1, 0, "ConfigurationValue");
    qmlRegisterUncreatableType<IppTags>("seaprint.ippmsg", 1, 0, "IppMsg", "Only used to supply an enum type");

    QDeclarativeView view;
    QDeclarativeContext* context = view.rootContext();

    // The singletons, under the names the Sailfish QML imports them by.
    context->setContextProperty("IppDiscovery", IppDiscovery::instance());
    context->setContextProperty("Mimer", Mimer::instance());
    context->setContextProperty("ConvertChecker", convertCheckerOrNull());
    context->setContextProperty("SeaPrintSettings", Settings::instance());
    context->setContextProperty("RangeListChecker", RangeListChecker::instance());
    context->setContextProperty("FileBrowser", new FileBrowser(&view));
    // Filled in below, once the root object exists; declared here so that the
    // bindings that read it evaluate to null instead of erroring on the way up.
    context->setContextProperty("appWindow", (QObject*)0);
    context->setContextProperty("Clipboard", new Clipboard(&view));
    context->setContextProperty("appVersion", QStringLiteral(SEAPRINT_VERSION));
    context->setContextProperty("appDataDir", data);

    // Silica's DialogResult, which the dialogs compare their result against.
    QVariantMap dialogResult;
    dialogResult.insert("Accepted", 1);
    dialogResult.insert("Rejected", 0);
    context->setContextProperty("DialogResult", dialogResult);

    // The theme object the ported pages read their metrics and colours from.
    // It is called AppTheme and not Theme on purpose: com.nokia.meego exports
    // a Theme of its own, which beats a context property of the same name and
    // would leave every page unstyled.
    context->setContextProperty("AppTheme",
                                instantiate(view.engine(), data + "/qml/context/Theme.qml"));

    view.engine()->addImportPath(data + "/qml");
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view.setSource(QUrl::fromLocalFile(data + "/qml/harbour-seaprint.qml"));

    // The window itself, for the stand-ins that have to put a pop-up
    // somewhere: a com.nokia.meego Menu anchors its pane to its parent item,
    // and a ContextMenu assigned to a property (as the settings do) has no
    // parent at all, so it would open into nothing.
    context->setContextProperty("appWindow", view.rootObject());

    FreedesktopDBusAdaptor freedesktopDbus(&view);
    SeaPrintDBusAdaptor seaprintDbus(&view);

    if(!QDBusConnection::sessionBus().registerObject("/net/attah/seaprint", &view))
    {
        qWarning() << "Could not register /net/attah/seaprint D-Bus object.";
    }
    if(!QDBusConnection::sessionBus().registerService("net.attah.seaprint"))
    {
        qWarning() << "Could not register net.attah.seaprint D-Bus service.";
    }

    // Harmattan hands a file over on the command line (the .desktop file asks
    // for %U), not through the Sailfish argument parser.
    QString fileName;
    for(int i = 1; i < argc; i++)
    {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if(arg.startsWith("-"))
        {
            continue;
        }
        fileName = arg.startsWith("file://") ? QUrl(arg).toLocalFile() : arg;
        break;
    }

    view.showFullScreen();

    if(!fileName.isEmpty())
    {
        qDebug() << "Opening" << fileName;
        QMetaObject::invokeMethod(view.rootObject(), "openFile", Q_ARG(QVariant, QVariant(fileName)));
    }

    return app.exec();
}
