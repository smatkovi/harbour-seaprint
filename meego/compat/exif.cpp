#include "exif.h"

#include <QFile>
#include <QTransform>

namespace {

quint16 readU16(const QByteArray& data, int pos, bool bigEndian)
{
    const uchar hi = (uchar)data.at(pos);
    const uchar lo = (uchar)data.at(pos+1);
    return bigEndian ? (hi << 8 | lo) : (lo << 8 | hi);
}

quint32 readU32(const QByteArray& data, int pos, bool bigEndian)
{
    const quint32 a = (uchar)data.at(pos);
    const quint32 b = (uchar)data.at(pos+1);
    const quint32 c = (uchar)data.at(pos+2);
    const quint32 d = (uchar)data.at(pos+3);
    return bigEndian ? (a << 24 | b << 16 | c << 8 | d) : (d << 24 | c << 16 | b << 8 | a);
}

}

int exifOrientation(const QString& fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        return 0;
    }
    // The orientation lives in the first IFD of the APP1 segment, which is at
    // the very start of the file; 128 kB is more than enough to hold it.
    const QByteArray data = file.read(128*1024);
    file.close();

    if(data.size() < 4 || (uchar)data.at(0) != 0xff || (uchar)data.at(1) != 0xd8)
    {
        return 0; // not a JPEG
    }

    int pos = 2;
    while(pos + 4 <= data.size())
    {
        if((uchar)data.at(pos) != 0xff)
        {
            return 0;
        }
        const uchar marker = (uchar)data.at(pos+1);
        const int length = readU16(data, pos+2, true);
        if(length < 2)
        {
            return 0;
        }
        if(marker == 0xe1 && pos+4+6 <= data.size() && data.mid(pos+4, 6) == QByteArray("Exif\0\0", 6))
        {
            const int tiff = pos + 10;
            if(tiff + 8 > data.size())
            {
                return 0;
            }
            const bool bigEndian = data.at(tiff) == 'M';
            const quint32 ifdOffset = readU32(data, tiff+4, bigEndian);
            const int ifd = tiff + int(ifdOffset);
            if(ifd + 2 > data.size())
            {
                return 0;
            }
            const int entries = readU16(data, ifd, bigEndian);
            for(int i = 0; i < entries; i++)
            {
                const int entry = ifd + 2 + 12*i;
                if(entry + 12 > data.size())
                {
                    return 0;
                }
                if(readU16(data, entry, bigEndian) == 0x0112)
                {
                    const int value = readU16(data, entry+8, bigEndian);
                    return (value >= 1 && value <= 8) ? value : 0;
                }
            }
            return 0;
        }
        if(marker == 0xda || marker == 0xd9)
        {
            return 0; // image data begins; there was no APP1
        }
        pos += 2 + length;
    }
    return 0;
}

QImage applyExifTransform(const QImage& image, const QString& fileName)
{
    if(image.isNull())
    {
        return image;
    }

    QTransform transform;
    switch(exifOrientation(fileName))
    {
    case 2: transform.scale(-1, 1); break;
    case 3: transform.rotate(180); break;
    case 4: transform.scale(1, -1); break;
    case 5: transform.rotate(90); transform.scale(1, -1); break;
    case 6: transform.rotate(90); break;
    case 7: transform.rotate(270); transform.scale(1, -1); break;
    case 8: transform.rotate(270); break;
    default: return image;
    }
    return image.transformed(transform, Qt::SmoothTransformation);
}
