// How a QStringListModel reaches a QtQuick 1.1 delegate.
//
// The printer list is a QStringListModel (IppDiscovery), and the delegate has
// to get the address out of it. Two questions, both of which decided whether
// the printer showed up on the N950 at all:
//
//   1. what the delegate calls the string -- model.display, display, or
//      modelData (which Qt 5 also offers);
//   2. whether the delegate notices when the string arrives *after* the row.
//      IppDiscovery::update() inserts the row first and sets its text in the
//      next call, and if QtQuick 1.1 does not follow that, every printer is
//      built with an empty address.
//
// Needs a display, so it runs on the device:
//   meego/build.sh guiprobe   ->  build/meego/guiprobe/model_probe
//   DISPLAY=:0 ./model_probe
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeError>
#include <QDeclarativeItem>
#include <QDeclarativeView>
#include <QGraphicsScene>
#include <QStringListModel>
#include <QTimer>
#include <QtDebug>

class Adder : public QObject
{
    Q_OBJECT

public:
    Adder(QStringListModel* model) : _model(model) {}

public slots:
    void addRow()
    {
        // The sequence IppDiscovery::update() uses.
        _model->insertRow(_model->rowCount());
        _model->setData(_model->index(_model->rowCount()-1, 0), "ipp://192.168.1.5/ipp/printer");
        qDebug() << "row inserted, then set";
    }

private:
    QStringListModel* _model;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QStringListModel model;

    QDeclarativeView view;
    view.rootContext()->setContextProperty("TheModel", &model);

    const char* qml =
        "import QtQuick 1.1\n"
        "ListView {\n"
        "    width: 100; height: 100\n"
        "    model: TheModel\n"
        "    delegate: Item {\n"
        "        property string seen: model.display\n"
        "        Component.onCompleted: console.log('delegate created with:', \"'\" + seen + \"'\",\n"
        "                                          '| display:', typeof display !== 'undefined' ? display : '<undefined>',\n"
        "                                          '| modelData:', typeof modelData !== 'undefined' ? modelData : '<undefined>')\n"
        "        onSeenChanged: console.log('delegate updated to:', \"'\" + seen + \"'\")\n"
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

    Adder adder(&model);
    QTimer::singleShot(1000, &adder, SLOT(addRow()));
    QTimer::singleShot(3000, &app, SLOT(quit()));

    return app.exec();
}

#include "model_probe.moc"
