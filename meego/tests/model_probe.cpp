// How a QStringListModel reaches a QtQuick 1.1 delegate.
//
// The printer list is a QStringListModel (IppDiscovery), and the delegate has
// to get the address out of it. Upstream says model.display; Qt 5 also offers
// modelData. Run on the N950, this answers: model.display and display both
// carry the address, modelData does not exist at all. So upstream's spelling
// is right here too -- which is worth having written down, because the empty
// printer list looked for a while as though it were not.
//
//   meego/build.sh guiprobe   ->  build/meego/guiprobe/model_probe
//   DISPLAY=:0 ./model_probe
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeError>
#include <QDeclarativeItem>
#include <QDeclarativeView>
#include <QGraphicsScene>
#include <QtDebug>
#include <QStringListModel>
#include <QTimer>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QStringListModel model;
    model.setStringList(QStringList() << "ipp://192.168.1.5/ipp/printer");

    QDeclarativeView view;
    view.rootContext()->setContextProperty("TheModel", &model);

    const char* qml =
        "import QtQuick 1.1\n"
        "ListView {\n"
        "    width: 100; height: 100\n"
        "    model: TheModel\n"
        "    delegate: Item {\n"
        "        Component.onCompleted: {\n"
        "            console.log('modelData:', typeof modelData !== 'undefined' ? modelData : '<undefined>')\n"
        "            console.log('model.display:', typeof model !== 'undefined' && model.display !== undefined\n"
        "                        ? model.display : '<undefined>')\n"
        "            console.log('display:', typeof display !== 'undefined' ? display : '<undefined>')\n"
        "            console.log('model.modelData:', typeof model !== 'undefined' && model.modelData !== undefined\n"
        "                        ? model.modelData : '<undefined>')\n"
        "        }\n"
        "    }\n"
        "}\n";

    QDeclarativeComponent component(view.engine());
    component.setData(QByteArray(qml), QUrl("qrc:/probe.qml"));
    QObject* object = component.create(view.rootContext());
    if(component.isError())
    {
        for(const QDeclarativeError& error : component.errors())
        {
            qWarning() << error.toString();
        }
        return 1;
    }
    view.setSceneRect(0, 0, 100, 100);
    if(QDeclarativeItem* item = qobject_cast<QDeclarativeItem*>(object))
    {
        view.scene()->addItem(item);
    }
    view.show();

    QTimer::singleShot(2000, &app, SLOT(quit()));
    return app.exec();
}
