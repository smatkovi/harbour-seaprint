#ifndef IPPDISCOVERY_H
#define IPPDISCOVERY_H
#include <QStringListModel>
#include <QUdpSocket>
#include <QMutex>
#include <QTimer>
#include "bytestream.h"

class IppDiscovery : public  QStringListModel
{
    Q_OBJECT
public:
    static IppDiscovery* instance();

    Q_PROPERTY(QStringList favourites READ favourites WRITE setFavourites NOTIFY favouritesChanged)

    QStringList favourites() const {return _favourites;}
    void setFavourites(const QStringList& favourites)
    {
        if(favourites != _favourites)
        {
            _favourites = favourites;
            emit favouritesChanged();
        }
    }

    Q_INVOKABLE void discover();
    Q_INVOKABLE void reset();

    bool resolve(QUrl& url);

signals:
    void favouritesChanged();
    void ignoreSslErrorsChanged();

public slots:
    void readPendingDatagrams();
    void cleanUpdate();
    void update();
    void askAgain();

protected:
private:
    static IppDiscovery* m_Instance;

    quint16 _transactionid;

    IppDiscovery();
    ~IppDiscovery();
    IppDiscovery(const IppDiscovery &);
    IppDiscovery& operator=(const IppDiscovery &);

    void sendQuery(quint16 qtype, QStringList addr);

    void sendQuery(quint16 qtype, QStringList prefixes, QStringList suffixes);

    void updateAndQueryPtrs(QStringList& ptrs, QStringList new_ptrs);

    void sendServiceQuery();
    bool queryWhatIsMissing();

    QStringList _ipp;
    QStringList _ipps;
    QMap<QString,QString> _rps;
    QMap<QString,quint16> _ports;
    QMap<QString,QString> _targets;

    QMultiMap<QString,QString> _AAs;
    QMultiMap<QString,QString> _AAAAs;

    QMap<QPair<quint16, QString>, QTime> _outstandingQueries;

    QStringList _favourites;
    QUdpSocket* socket;

    // One query and one answer is one UDP packet each, and multicast over
    // wifi loses some: on the N950 about one question in ten went unanswered,
    // and a printer only appears in the list once four exchanges in a row
    // (PTR, TXT, SRV, A) have come back. So the queries are repeated, with
    // the backoff of RFC 6762 section 5.2, until nothing is missing.
    QTimer _askAgainTimer;
    int _askAgainLeft;
    int _askAgainInterval;
};

#endif // IPPDISCOVERY_H
