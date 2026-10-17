// Runs the ported IPP parser over a saved response, off the device.
//
// When the app logs "Tried to read past end" after a successful request, the
// bytes are either short of what the printer sent or the parser walks off the
// end of a complete message -- and the log cannot tell the two apart. This
// takes the raw IPP body (what curl wrote, or a capture taken with
// meego/tests/ipp-capture.py) and parses it through the same IppMsg and the
// same Qt 4 JSON stand-ins the device uses, so a parser fault shows up here,
// on the build machine, in a second.
//
//   meego/tests/run-parse-test.sh <datei.raw> ...
//
// On a failure it prints how far the parser got and the bytes around that
// position, which is enough to find the attribute it choked on.
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

#include <fstream>
#include <iostream>

#include "ippmsg.h"

static QString oneLine(const QJsonValue& value)
{
    if(value.isArray() || value.isObject())
    {
        const QJsonDocument doc = value.isArray() ? QJsonDocument(value.toArray())
                                                  : QJsonDocument(value.toObject());
        return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    }
    if(value.isBool())
    {
        return value.toBool() ? "true" : "false";
    }
    if(value.isDouble())
    {
        return QString::number(value.toInt());
    }
    return value.toString();
}

static void show(const QJsonObject& attrs, const QString& name)
{
    std::cout << "    " << qPrintable(name.leftJustified(32)) << ": "
              << (attrs.contains(name) ? qPrintable(oneLine(attrs[name].toObject()["value"]))
                                       : "(nicht geliefert)")
              << std::endl;
}

static bool parse(const QString& path)
{
    std::ifstream ifs(path.toStdString().c_str(), std::ios::in | std::ios::binary);
    if(!ifs.good())
    {
        std::cout << qPrintable(path) << ": nicht lesbar" << std::endl;
        return false;
    }

    Bytestream bts(ifs);
    std::cout << qPrintable(path) << ": " << bts.size() << " Byte" << std::endl;

    try
    {
        IppMsg msg(bts);
        const QJsonObject attrs = msg.getPrinterAttrs();
        std::cout << "  Status 0x" << std::hex << msg.getStatus() << std::dec
                  << ", " << msg.getOpAttrs().size() << " Betriebs- und "
                  << attrs.size() << " Druckerattribute" << std::endl;
        // The handful the pages read; an empty one of these is what the user
        // sees as a printer without a name.
        show(attrs, "printer-name");
        show(attrs, "printer-make-and-model");
        show(attrs, "document-format-supported");
        show(attrs, "printer-resolution-default");
        show(attrs, "printer-resolution-supported");
        show(attrs, "media-default");
        show(attrs, "ipp-versions-supported");
        return attrs.size() != 0;
    }
    catch(const std::exception& e)
    {
        std::cout << "  FEHLER: " << e.what() << " -- bei Byte " << bts.pos()
                  << " von " << bts.size() << std::endl;
        // Where it stopped, with a little run-up, so the attribute can be
        // found in the capture.
        size_t from = bts.pos() > 48 ? bts.pos() - 48 : 0;
        size_t len = std::min((size_t)96, bts.size() - from);
        Bytestream around(bts.raw() + from, len);
        std::cout << "  ab Byte " << from << ": " << around.hexdump(len) << std::endl;
        return false;
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    const QStringList args = QCoreApplication::arguments().mid(1);
    if(args.isEmpty())
    {
        std::cerr << "usage: ipp_parse_test <datei.raw> ..." << std::endl;
        return 2;
    }

    bool allGood = true;
    for(const QString& path : args)
    {
        allGood = parse(path) && allGood;
    }
    return allGood ? 0 : 1;
}
