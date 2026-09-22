#include "QMimeDatabase"

#include <QFile>
#include <QFileInfo>

namespace {

struct SuffixType
{
    const char* suffix;
    const char* type;
};

// Only the types SeaPrint can print, plus the ones it must recognise in order
// to say "no" for the right reason.
const SuffixType SuffixTypes[] = {
    {"pdf",  "application/pdf"},
    {"ps",   "application/postscript"},
    {"jpg",  "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"jpe",  "image/jpeg"},
    {"png",  "image/png"},
    {"gif",  "image/gif"},
    {"tif",  "image/tiff"},
    {"tiff", "image/tiff"},
    {"bmp",  "image/bmp"},
    {"svg",  "image/svg+xml"},
    {"txt",  "text/plain"},
    {"text", "text/plain"},
    {"log",  "text/plain"},
    {"md",   "text/plain"},
    {"csv",  "text/csv"},
    {"doc",  "application/msword"},
    {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {"rtf",  "text/rtf"},
    {"odt",  "application/vnd.oasis.opendocument.text"},
    {"ppt",  "application/vnd.ms-powerpoint"},
    {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {"odp",  "application/vnd.oasis.opendocument.presentation"},
    {0, 0}
};

bool startsWith(const QByteArray& data, const char* magic, int len)
{
    return data.size() >= len && memcmp(data.constData(), magic, len) == 0;
}

// Files handed over by another app often have no suffix worth trusting.
QString sniff(const QString& fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        return QString();
    }
    const QByteArray head = file.read(512);
    file.close();

    if(startsWith(head, "%PDF", 4))
    {
        return "application/pdf";
    }
    if(startsWith(head, "%!PS", 4))
    {
        return "application/postscript";
    }
    if(startsWith(head, "\xff\xd8\xff", 3))
    {
        return "image/jpeg";
    }
    if(startsWith(head, "\x89PNG\r\n\x1a\n", 8))
    {
        return "image/png";
    }
    if(startsWith(head, "GIF87a", 6) || startsWith(head, "GIF89a", 6))
    {
        return "image/gif";
    }
    if(startsWith(head, "II*\0", 4) || startsWith(head, "MM\0*", 4))
    {
        return "image/tiff";
    }
    if(startsWith(head, "BM", 2))
    {
        return "image/bmp";
    }
    if(startsWith(head, "{\\rtf", 5))
    {
        return "text/rtf";
    }
    if(startsWith(head, "\xd0\xcf\x11\xe0", 4))
    {   // An OLE2 container: .doc and .ppt both live in one.
        return "application/msword";
    }
    if(head.contains("<svg"))
    {
        return "image/svg+xml";
    }
    if(head.isEmpty())
    {
        return QString();
    }
    // Text if it reads as UTF-8 without control characters.
    for(int i = 0; i < head.size(); i++)
    {
        const unsigned char c = (unsigned char)head.at(i);
        if(c < 0x09 || (c > 0x0d && c < 0x20))
        {
            return QString();
        }
    }
    return "text/plain";
}

}

QMimeType QMimeDatabase::mimeTypeForFile(const QString& fileName, MatchMode mode) const
{
    const QString suffix = QFileInfo(fileName).suffix().toLower();

    if(mode != MatchContent && !suffix.isEmpty())
    {
        for(const SuffixType* st = SuffixTypes; st->suffix; st++)
        {
            if(suffix == QLatin1String(st->suffix))
            {
                return QMimeType(QLatin1String(st->type));
            }
        }
    }

    if(mode != MatchExtension)
    {
        const QString sniffed = sniff(fileName);
        if(!sniffed.isEmpty())
        {
            return QMimeType(sniffed);
        }
    }

    return QMimeType("application/octet-stream");
}
