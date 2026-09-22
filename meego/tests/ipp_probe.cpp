// Asks a real printer what it can do, with the ported code, as an ARM binary.
//
// This is the part of the port that nothing else can check on a desktop: the
// IPP message is built and parsed through the Qt 4 stand-ins for Qt 5's JSON
// classes (meego/compat/qt4json.h), and posted with the N9's curl 7.21. Run it
// under qemu-arm against CUPS's own test printer:
//
//   meego/tests/run-ipp-probe.sh
//
// It prints the attributes SeaPrint's pages actually read, so a shim that
// silently drops a value shows up here rather than on the device.
#include <QCoreApplication>
#include <QStringList>
#include <QtDebug>

#include <iostream>

#include <QJsonDocument>

#include "curlrequester.h"
#include "ippmsg.h"

static void show(const QJsonObject& attrs, const QString& name)
{
    if(!attrs.contains(name))
    {
        std::cout << qPrintable(name) << ": (not offered)" << std::endl;
        return;
    }
    const QJsonValue value = attrs[name].toObject()["value"];
    // Objects are printed as JSON, which also puts the shim's own serialiser
    // through its paces: a resolution and a media collection are objects.
    QString text;
    if(value.isArray())
    {
        QStringList items;
        for(const QJsonValue& item : value.toArray())
        {
            items.append(item.isObject()
                         ? QString::fromUtf8(QJsonDocument(item.toObject()).toJson(QJsonDocument::Compact))
                         : item.toString(QString::number(item.toInt())));
        }
        text = items.join(", ");
    }
    else if(value.isObject())
    {
        text = QString::fromUtf8(QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact));
    }
    else
    {
        text = value.toString(QString::number(value.toInt()));
    }
    std::cout << qPrintable(name) << ": " << qPrintable(text) << std::endl;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if(argc < 2)
    {
        std::cerr << "usage: ipp_probe http://host:port/ipp/print" << std::endl;
        return 2;
    }
    const QString url = QString::fromLocal8Bit(argv[1]);

    QJsonObject opAttrs;
    opAttrs.insert("attributes-charset",
                   QJsonObject {{"tag", IppMsg::Charset}, {"value", "utf-8"}});
    opAttrs.insert("attributes-natural-language",
                   QJsonObject {{"tag", IppMsg::NaturalLanguage}, {"value", "en-us"}});
    opAttrs.insert("printer-uri",
                   QJsonObject {{"tag", IppMsg::Uri}, {"value", url}});
    opAttrs.insert("requesting-user-name",
                   QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", "seaprint"}});
    opAttrs.insert("requested-attributes",
                   QJsonObject {{"tag", IppMsg::Keyword}, {"value", "all"}});

    IppMsg request(IppMsg::GetPrinterAttrs, opAttrs);
    Bytestream encoded = request.encode();

    CurlIppPoster poster(url.toStdString(), encoded, false, true);
    Bytestream response;
    const CURLcode result = poster.await(&response);

    if(result != CURLE_OK)
    {
        std::cerr << "curl failed: " << curl_easy_strerror(result) << std::endl;
        return 1;
    }

    try
    {
        IppMsg reply(response);
        const QJsonObject attrs = reply.getPrinterAttrs();

        std::cout << "status: 0x" << std::hex << reply.getStatus() << std::dec << std::endl;
        std::cout << "attributes: " << attrs.keys().size() << std::endl;
        show(attrs, "printer-name");
        show(attrs, "printer-make-and-model");
        show(attrs, "document-format-supported");
        show(attrs, "media-default");
        show(attrs, "sides-supported");
        show(attrs, "print-color-mode-supported");
        show(attrs, "printer-resolution-default");
        show(attrs, "media-col-default");

        if(attrs.keys().isEmpty())
        {
            std::cerr << "no attributes decoded" << std::endl;
            return 1;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "decode failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
