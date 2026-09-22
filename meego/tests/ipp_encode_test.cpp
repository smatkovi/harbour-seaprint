// What goes on the wire for a print job, byte for byte.
//
// A job that the printer accepts and then never prints looks the same from
// the outside whether the document never arrived or an attribute was lost on
// the way -- and neither shows in the app's log, because curl's own output
// does not go through Qt's message handler. So this builds the two messages
// of the IPP 2.0 path exactly as PrinterWorker does and prints them, with the
// attributes decoded back out of the bytes.
//
// It needs no printer and no device: the encoder and the Qt 4 JSON stand-ins
// are the same everywhere.
//
//   meego/build.sh tools && qemu-arm -L $SYSROOT build/meego/tools/ipp_encode_test
#include <QCoreApplication>
#include <QStringList>

#include <iostream>

#include "ippmsg.h"

static QString tagName(quint8 tag)
{
    switch(tag)
    {
    case IppMsg::OpAttrs: return "(operation attributes)";
    case IppMsg::JobAttrs: return "(job attributes)";
    case IppMsg::EndAttrs: return "(end)";
    case IppMsg::Integer: return "integer";
    case IppMsg::Boolean: return "boolean";
    case IppMsg::Enum: return "enum";
    case IppMsg::Keyword: return "keyword";
    case IppMsg::Uri: return "uri";
    case IppMsg::Charset: return "charset";
    case IppMsg::NaturalLanguage: return "naturalLanguage";
    case IppMsg::MimeMediaType: return "mimeMediaType";
    case IppMsg::NameWithoutLanguage: return "name";
    case IppMsg::TextWithoutLanguage: return "text";
    default: return QString("tag 0x%1").arg(tag, 2, 16, QChar('0'));
    }
}

// Walk the encoded message and print what a printer would read out of it.
static void dump(Bytestream msg)
{
    const quint8* raw = msg.raw();
    const size_t size = msg.size();

    if(size < 8)
    {
        std::cout << "    (too short)" << std::endl;
        return;
    }
    std::cout << "    version " << int(raw[0]) << "." << int(raw[1])
              << ", operation 0x" << std::hex << ((raw[2] << 8) | raw[3]) << std::dec
              << ", " << size << " bytes" << std::endl;

    size_t i = 8;
    while(i < size)
    {
        const quint8 tag = raw[i++];
        if(tag <= 0x05)
        {
            std::cout << "    " << qPrintable(tagName(tag)) << std::endl;
            if(tag == IppMsg::EndAttrs)
            {
                break;
            }
            continue;
        }
        if(i + 2 > size) break;
        const quint16 nameLen = (raw[i] << 8) | raw[i+1];
        i += 2;
        const QString name = QString::fromUtf8((const char*)raw + i, nameLen);
        i += nameLen;
        if(i + 2 > size) break;
        const quint16 valueLen = (raw[i] << 8) | raw[i+1];
        i += 2;

        QString value;
        if(tag == IppMsg::Integer || tag == IppMsg::Enum)
        {
            qint32 v = 0;
            for(int b = 0; b < valueLen; b++)
            {
                v = (v << 8) | raw[i+b];
            }
            value = QString::number(v);
        }
        else if(tag == IppMsg::Boolean)
        {
            value = raw[i] ? "true" : "false";
        }
        else
        {
            value = QString::fromUtf8((const char*)raw + i, valueLen);
        }
        i += valueLen;

        std::cout << "        " << qPrintable(name.isEmpty() ? QString("  (another value)") : name)
                  << " [" << qPrintable(tagName(tag)) << "] = " << qPrintable(value) << std::endl;
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    const QString url = "ipp://192.168.1.5/ipp/printer";

    // Create-Job, as IppPrinter::print() builds it for a printer that speaks
    // IPP 2.0.
    QJsonObject opAttrs;
    opAttrs.insert("attributes-charset",
                   QJsonObject {{"tag", IppMsg::Charset}, {"value", "utf-8"}});
    opAttrs.insert("attributes-natural-language",
                   QJsonObject {{"tag", IppMsg::NaturalLanguage}, {"value", "en-us"}});
    opAttrs.insert("printer-uri",
                   QJsonObject {{"tag", IppMsg::Uri}, {"value", url}});
    opAttrs.insert("requesting-user-name",
                   QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", "user"}});
    opAttrs.insert("job-name",
                   QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", "test.pdf"}});

    IppMsg createJob(IppMsg::CreateJob, opAttrs, QJsonObject(), 2, 0);
    std::cout << "Create-Job:" << std::endl;
    dump(createJob.encode());

    // Send-Document, as PrinterWorker::print2() completes it once the printer
    // has answered with a job id. This is the message in question: without
    // last-document the printer keeps the job open and waits for more.
    QJsonObject sendOpAttrs;
    sendOpAttrs.insert("attributes-charset",
                       QJsonObject {{"tag", IppMsg::Charset}, {"value", "utf-8"}});
    sendOpAttrs.insert("attributes-natural-language",
                       QJsonObject {{"tag", IppMsg::NaturalLanguage}, {"value", "en-us"}});
    sendOpAttrs.insert("printer-uri",
                       QJsonObject {{"tag", IppMsg::Uri}, {"value", url}});
    sendOpAttrs.insert("requesting-user-name",
                       QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", "user"}});
    sendOpAttrs.insert("document-format",
                       QJsonObject {{"tag", IppMsg::MimeMediaType}, {"value", "application/pdf"}});

    IppMsg sendDocument(IppMsg::SendDocument, sendOpAttrs, QJsonObject(), 2, 0);
    sendDocument.setOpAttr("job-id", IppMsg::Integer, 27);
    sendDocument.setOpAttr("last-document", IppMsg::Boolean, true);

    std::cout << std::endl << "Send-Document:" << std::endl;
    dump(sendDocument.encode());

    return 0;
}
