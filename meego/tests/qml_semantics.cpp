// What QtQuick 1.1 does with the two things the ported QML depends on:
//
//   1. a C++ QVariantMap property read from JavaScript -- the printer
//      attributes arrive that way (upstream hands QML a QJsonObject), and the
//      pages do printer.attrs["media-supported"].value[0], so the conversion
//      has to be recursive;
//   2. a JS object kept in a `property variant` and then modified in place --
//      upstream's SettingsColumn builds the job attributes like that.
//
// Run on the build machine, no display needed:
//   meego/tests/run-qml-semantics.sh
#include <QCoreApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QtDebug>

class Fixture : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap attrs READ attrs CONSTANT)
    Q_PROPERTY(QVariantList list READ list CONSTANT)

public:
    QVariantMap attrs() const
    {
        // The shape IppMsg builds: a map of tagged values, one of them a list.
        QVariantMap sides;
        sides.insert("tag", 0x44);
        sides.insert("value", "one-sided");

        QVariantList supported;
        supported << "one-sided" << "two-sided-long-edge";
        QVariantMap sidesSupported;
        sidesSupported.insert("tag", 0x44);
        sidesSupported.insert("value", supported);

        QVariantMap out;
        out.insert("sides-default", sides);
        out.insert("sides-supported", sidesSupported);
        return out;
    }

    QVariantList list() const
    {
        QVariantList out;
        out << 1 << 2 << 3;
        return out;
    }
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QDeclarativeEngine engine;
    Fixture fixture;
    engine.rootContext()->setContextProperty("Fixture", &fixture);

    const char* qml =
        "import QtQuick 1.1\n"
        "QtObject {\n"
        "    property variant jobParams: ({})\n"
        "    property variant bag: []\n"
        "    Component.onCompleted: {\n"
        "        var a = Fixture.attrs\n"
        "        console.log('1 typeof attrs:', typeof a)\n"
        "        console.log('2 hasOwnProperty:', a.hasOwnProperty('sides-supported'))\n"
        "        console.log('3 nested map:', typeof a['sides-default'], a['sides-default'].value)\n"
        "        console.log('4 nested list:', typeof a['sides-supported'].value,\n"
        "                    a['sides-supported'].value.length, a['sides-supported'].value[1])\n"
        "        console.log('5 plain list:', typeof Fixture.list, Fixture.list.length, Fixture.list[2])\n"
        "        jobParams['copies'] = {tag: 33, value: 2}\n"
        "        console.log('6 variant kept the write:', jobParams.hasOwnProperty('copies'))\n"
        "        var local = jobParams\n"
        "        local['sides'] = {tag: 68, value: 'one-sided'}\n"
        "        console.log('7 write through a local:', jobParams.hasOwnProperty('sides'))\n"
        "        bag.push(7)\n"
        "        console.log('8 push into a variant array:', bag.length)\n"
        "        var copy = jobParams\n"
        "        copy = {}\n"
        "        copy['x'] = 1\n"
        "        jobParams = copy\n"
        "        console.log('9 assignment back:', jobParams.hasOwnProperty('x'))\n"
        // What the pages expect of the JavaScript engine. Qt 4.7's QML runs on
        // JavaScriptCore, Qt 4.8's on V8, so this has to be answered by the
        // device's own Qt -- build it for ARM and run it under qemu-arm.
        "        console.log('10 Object.keys:', typeof Object.keys)\n"
        "        console.log('11 JSON:', typeof JSON, typeof JSON.stringify, typeof JSON.parse)\n"
        "        console.log('12 Array.isArray:', typeof Array.isArray)\n"
        "        console.log('13 Array.prototype.filter:', typeof [].filter, typeof [].indexOf,\n"
        "                    typeof [].forEach, typeof [].map)\n"
        "        console.log('14 String.prototype.trim:', typeof ''.trim)\n"
        "        console.log('15 Function.prototype.bind:', typeof (function(){}).bind)\n"
        "        console.log('16 openDatabaseSync:', typeof openDatabaseSync)\n"
        "        console.log('17 round trip:', JSON.stringify({a: [1, {b: 2}]}))\n"
        // The favourites live in a QML offline-storage database; on the device
        // this answered 'Not an XMLHttpRequest object', which no desktop Qt
        // reproduces.
        "        try {\n"
        "            var db = openDatabaseSync('ProbeDB', '1.0', 'probe', 100000)\n"
        "            console.log('18 openDatabaseSync:', typeof db, db ? db.version : '-')\n"
        "            db.transaction(function (tx) {\n"
        "                tx.executeSql('CREATE TABLE IF NOT EXISTS T (a STRING)')\n"
        "                tx.executeSql('INSERT INTO T VALUES(?)', ['x'])\n"
        "                var res = tx.executeSql('SELECT * FROM T')\n"
        "                console.log('19 rows:', res.rows.length)\n"
        "            })\n"
        "        } catch (e) {\n"
        "            console.log('18 database failed:', e)\n"
        "        }\n"
        "    }\n"
        "}\n";

    QDeclarativeComponent component(&engine);
    component.setData(QByteArray(qml), QUrl("qrc:/probe.qml"));
    if(component.isError())
    {
        for(const QDeclarativeError& error : component.errors())
        {
            qDebug() << error.toString();
        }
        return 1;
    }
    QObject* object = component.create();
    if(!object)
    {
        qDebug() << "no object";
        return 1;
    }
    return 0;
}

#include "qml_semantics.moc"
