#ifndef IPPPRINTER_H
#define IPPPRINTER_H

#include "ippmsg.h"
#include "printerworker.h"
#include "curlrequester.h"
#include <QImage>


class IppPrinter : public QObject
{
    Q_OBJECT

    friend class PrinterWorker;

    // Read through getters rather than MEMBER, which moc 4.7 does not know,
    // and as QVariantMap/QVariantList rather than as the JSON classes:
    // QtDeclarative turns those into plain JavaScript objects for the QML,
    // and Qt 5 does the same, so both builds see the same thing.
    Q_PROPERTY(QString url READ getUrl WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QVariantMap attrs READ attrs NOTIFY attrsChanged)
    Q_PROPERTY(QVariantMap jobAttrs READ jobAttrs NOTIFY jobAttrsChanged)
    Q_PROPERTY(QVariantList jobs READ jobs NOTIFY jobsChanged)
    Q_PROPERTY(QVariantMap strings READ strings NOTIFY stringsChanged)
    Q_PROPERTY(QImage icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QStringList additionalDocumentFormats READ additionalDocumentFormats NOTIFY additionalDocumentFormatsChanged)
    Q_PROPERTY(QString busyMessage READ busyMessage NOTIFY busyMessageChanged)
    Q_PROPERTY(QString progress READ progress NOTIFY progressChanged)

    Q_PROPERTY(bool correctSuffix READ correctSuffix NOTIFY attrsChanged)
    Q_PROPERTY(QStringList suffixes READ suffixes NOTIFY attrsChanged)

public:
    IppPrinter();
    ~IppPrinter();


    QString getUrl() {return _url.toString();}
    void setUrl(QString url);

    QVariantMap attrs() const {return _attrs.toVariantMap();}
    QVariantMap jobAttrs() const {return _jobAttrs.toVariantMap();}
    QVariantList jobs() const {return _jobs.toVariantList();}
    QVariantMap strings() const {return _strings.toVariantMap();}
    QImage icon() const {return _icon;}
    QStringList additionalDocumentFormats() const {return _additionalDocumentFormats;}
    QString busyMessage() const {return _busyMessage;}
    QString progress() const {return _progress;}

    Q_INVOKABLE void refresh();

    Q_INVOKABLE bool getJobs();
    Q_INVOKABLE bool cancelJob(qint32 jobId);
    Q_INVOKABLE bool identify();

    bool correctSuffix();
    QStringList suffixes();

signals:
    void urlChanged();
    void attrsChanged();
    void jobAttrsChanged();
    void jobsChanged();

    void stringsChanged();
    void iconChanged();

    void jobFinished(bool status);
    void cancelStatus(bool status);

    void doDoGetPrinterAttributes(Bytestream msg);
    void doGetJobs(Bytestream msg);
    void doCancelJob(Bytestream msg);
    void doIdentify(Bytestream msg);

    void doJustUpload(QString filename, Bytestream header);
    void doFixupPlaintext(QString filename, Bytestream header);
    void doPrintImageAsImage(QString filename, Bytestream header, QString targetFormat);

    void doConvertPdf(QString filename, Bytestream header, PrintParameters Params);

    void doConvertImage(QString filename,  Bytestream header, PrintParameters Params, QMargins margins);

    void doConvertOfficeDocument(QString filename, Bytestream header, PrintParameters Params);

    void doConvertPlaintext(QString filename, Bytestream header, PrintParameters Params);

    void doPrint(QString filename, QString mimeType, QString targetFormat, IppMsg job, PrintParameters Params, QMargins margins);
    void doPrint2(QString filename, QString mimeType, QString targetFormat, IppMsg createJob, IppMsg sendDocument, PrintParameters Params, QMargins margins);

    void doGetStrings(QUrl url);
    void doGetImage(QUrl url);

    void additionalDocumentFormatsChanged();
    void busyMessageChanged();
    void progressChanged();

public slots:
    // QML hands the job settings over as a JavaScript object. Qt 5 would turn
    // that into a QJsonObject on its own; QtDeclarative makes a QVariantMap of
    // it, which is the one spelling both engines convert to.
    void print(QVariantMap attrs, QString file);

    void onUrlChanged();
    void MaybeGetStrings();
    void MaybeGetIcon(bool retry=false);
    void UpdateAdditionalDocumentFormats();
    void getPrinterAttributesFinished(CURLcode res, Bytestream data);
    void printRequestFinished(CURLcode res, Bytestream data);
    void getJobsRequestFinished(CURLcode res, Bytestream data);
    void cancelJobFinished(CURLcode res, Bytestream data);
    void identifyFinished(CURLcode res, Bytestream data);

    void getStringsFinished(CURLcode res, Bytestream data);
    void getImageFinished(CURLcode res, Bytestream data);

    void convertFailed(QString message);

private:
    QUrl _url;
    QUrl httpUrl();
    void resolveUrl(QUrl& url);

    QJsonObject opAttrs();

    void adjustRasterSettings(QString filename, QString mimeType, QJsonObject& jobAttrs, PrintParameters& Params);

    void setBusyMessage(QString msg);
    void setProgress(qint64 sent, qint64 total);

    bool isAllowedAddress(QUrl addr);

    QJsonValue getAttrOrDefault(QJsonObject jobAttrs, QString name, QString subkey = "");

    IppMsg mk_msg(IppMsg::Operation operation, QJsonObject opAttrs, QJsonObject jobAttrs=QJsonObject());

    QJsonObject _attrs;
    QJsonObject _jobAttrs;
    QJsonArray _jobs;

    QJsonObject _strings;
    QImage _icon;

    QStringList _additionalDocumentFormats;

    QString _busyMessage;
    QString _progress;

    PrinterWorker* _worker;

    bool _iconRetried = false;
};

#endif // IPPPRINTER_H
