// What a com.nokia.meego Menu does when it is built the way the Silica
// stand-in builds it.
//
// The settings pages say
//
//     menu: ContextMenu { MenuItem { text: "..." } ... }
//
// and meego/qml/silica/ContextMenu.qml turns that into a Menu with a
// MenuLayout inside, parented to the window because an object assigned to a
// property has no parent of its own. On the device that opened as a black
// sheet with nothing in it, so this builds the same thing three ways and
// shows which one has entries in it.
//
//   meego/build.sh guiprobe   ->  build/meego/guiprobe/menu_probe
//   DISPLAY=:0 ./menu_probe [1|2|3]
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeError>
#include <QDeclarativeItem>
#include <QDeclarativeView>
#include <QGraphicsScene>
#include <QTimer>
#include <QtDebug>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    const int variant = argc > 1 ? QString::fromLocal8Bit(argv[1]).toInt() : 1;

    // 1: the menu declared straight inside the page, as Harmattan's own
    //    examples do.
    // 2: the menu inside a holder item, opened through it -- the shape of the
    //    Silica stand-in, but left where it was declared.
    // 3: like 2, but with the menu reparented to the window, which is what
    //    the stand-in does so that a menu assigned to a property is visible.
    // 4: like 3, but the holder's default property points at MenuLayout's own
    //    default property (menuChildren) rather than at its children: the
    //    layout keeps its items in a Column inside itself, so anything put in
    //    its children list is simply never laid out.
    // 5: no default-property alias at all. The items stay ordinary children
    //    of the holder and are moved into a Column inside the menu as they
    //    appear -- which also catches the ones a Repeater makes later.
    // 6: the port's own ContextMenu.qml, as installed on the device, filled
    //    by a Repeater exactly the way ChoiceSetting does it. This is the
    //    colour-mode menu, without the rest of the app around it.
    QString body;
    if(variant == 6)
    {
        body =
            "    property variant choices: ['color', 'monochrome', 'auto']\n"
            "    ContextMenu {\n"
            "        id: holder\n"
            "        Repeater {\n"
            "            model: page.choices\n"
            "            MenuItem { text: modelData }\n"
            "        }\n"
            "    }\n";
    }
    else if(variant == 5)
    {
        body =
            "    Item {\n"
            "        id: holder\n"
            "        width: 0; height: 0\n"
            "        property bool moving: false\n"
            "        function open() { menu.open() }\n"
            "        function relayout() {\n"
            "            if (moving) return\n"
            "            moving = true\n"
            "            var kids = holder.children\n"
            "            for (var i = kids.length - 1; i >= 0; i--) {\n"
            "                if (kids[i] !== menu) kids[i].parent = column\n"
            "            }\n"
            "            moving = false\n"
            "        }\n"
            "        onChildrenChanged: relayout()\n"
            "        Component.onCompleted: relayout()\n"
            "        Menu {\n"
            "            id: menu\n"
            "            parent: page\n"
            "            Column {\n"
            "                id: column\n"
            "                anchors.left: parent.left\n"
            "                anchors.right: parent.right\n"
            "            }\n"
            "        }\n"
            "        MenuItem { text: 'one' }\n"
            "        MenuItem { text: 'two' }\n"
            "        MenuItem { text: 'three' }\n"
            "    }\n";
    }
    else if(variant == 1)
    {
        body =
            "    Menu {\n"
            "        id: menu\n"
            "        MenuLayout {\n"
            "            MenuItem { text: 'one' }\n"
            "            MenuItem { text: 'two' }\n"
            "            MenuItem { text: 'three' }\n"
            "        }\n"
            "    }\n";
    }
    else
    {
        body =
            "    Item {\n"
            "        id: holder\n"
            "        width: 0; height: 0\n"
            "        default property alias content: layout.children\n"
            "        function open() { menu.open() }\n"
            "        Menu {\n"
            "            id: menu\n";
        if(variant == 3 || variant == 4)
        {
            body += "            parent: page\n";
        }
        body +=
            "            MenuLayout { id: layout }\n"
            "        }\n"
            "        MenuItem { text: 'one' }\n"
            "        MenuItem { text: 'two' }\n"
            "        MenuItem { text: 'three' }\n"
            "    }\n";
        if(variant == 4)
        {
            body.replace("default property alias content: layout.children",
                         "default property alias content: layout.menuChildren");
        }
    }

    const QString qml =
        "import QtQuick 1.1\n"
        "import com.nokia.meego 1.0\n"
        + QString(variant == 6 ? "import \"silica\"\n" : "") +
        "Rectangle {\n"
        "    id: page\n"
        "    width: 854; height: 480\n"
        "    color: '#202020'\n"
        "    Text { anchors.centerIn: parent; color: 'white'; text: 'menu probe' }\n"
        + body +
        "    Component.onCompleted: openTimer.start()\n"
        "    Timer {\n"
        "        id: openTimer\n"
        "        interval: 1500\n"
        "        onTriggered: {\n"
        + QString(variant == 1 ? "            menu.open()\n" : "            holder.open(page)\n") +
        "            console.log('opened')\n"
        "        }\n"
        "    }\n"
        "}\n";

    qDebug().nospace() << "--- QML ---\n" << qPrintable(qml) << "--- end ---";

    QDeclarativeView view;

    // The stand-ins read their metrics from AppTheme and put their pop-ups in
    // appWindow, both of which meego/main.cpp supplies.
    QDeclarativeComponent themeComponent(view.engine(),
        QUrl::fromLocalFile("/opt/harbour-seaprint/qml/context/Theme.qml"));
    QObject* theme = themeComponent.create(view.rootContext());
    if(theme)
    {
        theme->setParent(view.engine());
        QDeclarativeEngine::setObjectOwnership(theme, QDeclarativeEngine::CppOwnership);
    }
    view.rootContext()->setContextProperty("AppTheme", theme);

    QDeclarativeComponent component(view.engine());
    // The base URL decides what a directory import means; the installed QML
    // is where the stand-ins live.
    component.setData(qml.toUtf8(),
                      variant == 6 ? QUrl::fromLocalFile("/opt/harbour-seaprint/qml/menu-probe.qml")
                                   : QUrl("qrc:/menu.qml"));
    QObject* object = component.create(view.rootContext());
    if(component.isError())
    {
        for(const QDeclarativeError& error : component.errors())
        {
            qWarning() << error.toString();
        }
        return 1;
    }
    view.setSceneRect(0, 0, 854, 480);
    if(QDeclarativeItem* item = qobject_cast<QDeclarativeItem*>(object))
    {
        view.scene()->addItem(item);
        // The stand-ins hang their pop-ups on this.
        view.rootContext()->setContextProperty("appWindow", item);
    }
    view.showFullScreen();
    view.raise();
    view.activateWindow();

    QTimer::singleShot(6000, &app, SLOT(quit()));
    return app.exec();
}
