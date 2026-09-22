#ifndef MIMER_H
#define MIMER_H

#include <QString>
#include <QObject>
#include <QMutex>
#include <QMimeDatabase>

class Mimer : public  QObject
{
    Q_OBJECT
public:
    static Mimer* instance();
    Q_INVOKABLE QString get_type(QString filename);

    Q_PROPERTY(QString OctetStream READ getOctetStream CONSTANT)
    QString getOctetStream() const {return OctetStream;}

    Q_PROPERTY(QString PDF READ getPDF CONSTANT)
    QString getPDF() const {return PDF;}
    Q_PROPERTY(QString Postscript READ getPostscript CONSTANT)
    QString getPostscript() const {return Postscript;}
    Q_PROPERTY(QString PWG READ getPWG CONSTANT)
    QString getPWG() const {return PWG;}
    Q_PROPERTY(QString URF READ getURF CONSTANT)
    QString getURF() const {return URF;}

    Q_PROPERTY(QString PNG READ getPNG CONSTANT)
    QString getPNG() const {return PNG;}
    Q_PROPERTY(QString GIF READ getGIF CONSTANT)
    QString getGIF() const {return GIF;}
    Q_PROPERTY(QString JPEG READ getJPEG CONSTANT)
    QString getJPEG() const {return JPEG;}
    Q_PROPERTY(QString TIFF READ getTIFF CONSTANT)
    QString getTIFF() const {return TIFF;}
    Q_PROPERTY(QString SVG READ getSVG CONSTANT)
    QString getSVG() const {return SVG;}
    Q_PROPERTY(QString RBMP READ getRBMP CONSTANT)
    QString getRBMP() const {return RBMP;}

    Q_PROPERTY(QString DOC READ getDOC CONSTANT)
    QString getDOC() const {return DOC;}
    Q_PROPERTY(QString DOCX READ getDOCX CONSTANT)
    QString getDOCX() const {return DOCX;}
    Q_PROPERTY(QString RTF READ getRTF CONSTANT)
    QString getRTF() const {return RTF;}
    Q_PROPERTY(QString ODT READ getODT CONSTANT)
    QString getODT() const {return ODT;}
    Q_PROPERTY(QString PPT READ getPPT CONSTANT)
    QString getPPT() const {return PPT;}
    Q_PROPERTY(QString PPTX READ getPPTX CONSTANT)
    QString getPPTX() const {return PPTX;}
    Q_PROPERTY(QString ODP READ getODP CONSTANT)
    QString getODP() const {return ODP;}


    Q_PROPERTY(QString Plaintext READ getPlaintext CONSTANT)
    QString getPlaintext() const {return Plaintext;}

    Q_PROPERTY(QStringList OfficeFormats READ getOfficeFormats CONSTANT)
    QStringList getOfficeFormats() const {return OfficeFormats;}

    static const QString OctetStream;

    static const QString PDF;
    static const QString Postscript;
    static const QString PWG;
    static const QString URF;

    static const QString PNG;
    static const QString GIF;
    static const QString JPEG;
    static const QString TIFF;
    static const QString SVG;
    static const QString RBMP;

    static const QString DOC;
    static const QString DOCX;
    static const QString RTF;
    static const QString RTF_APP;
    static const QString ODT;
    static const QString PPT;
    static const QString PPTX;
    static const QString ODP;

    static const QString Plaintext;

    static const QStringList RasterFormats;
    static const QStringList OfficeFormats;

    Q_INVOKABLE static bool isImage(QString mimeType)
    {
        return mimeType.startsWith("image/") && ! isRaster(mimeType);
    }
    Q_INVOKABLE static bool isRaster(QString mimeType)
    {
        return RasterFormats.contains(mimeType);
    }
    Q_INVOKABLE static bool isOffice(QString mimeType)
    {
        return OfficeFormats.contains(mimeType);
    }

private:
    Mimer();
    static Mimer* m_Instance;
    QMimeDatabase _db;

};

#endif // MIMER_H
