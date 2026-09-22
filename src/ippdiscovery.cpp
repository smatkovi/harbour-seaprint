#include "ippdiscovery.h"
#include "ippprinter.h"
#include "settings.h"

#define A 1
#define PTR 12
#define TXT 16
#define AAAA 28
#define SRV 33

#define ALL 255 //for querying

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
static bool caseInsensitiveLessThan(const QString& a, const QString& b)
{
    return a.compare(b, Qt::CaseInsensitive) < 0;
}
#endif

QStringList get_addr(Bytestream& bts)
{
    QStringList addr;
    while(true)
    {
        if(bts.nextU8(0))
        {
            break;
        }
        else if ((bts.peekU8()&0xc0)==0xc0)
        {
            quint16 ref = bts.getU16() & 0x0fff;
            Bytestream tmp = bts;
            tmp.setPos(ref);
            addr += get_addr(tmp);
            break;
        }
        else
        {
            std::string elem;
            bts/bts.getU8() >> elem;
            addr.append(QString(elem.c_str()));
        }
    }
    return addr;
}

IppDiscovery::IppDiscovery() : QStringListModel()
{
    socket = new QUdpSocket(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    // Bind before sending anything. Qt 5 arms the read notifier as soon as a
    // datagram goes out, Qt 4.7 only when the socket is bound -- so on
    // Harmattan the printers' answers arrived at the port and nothing ever
    // read them, and discovery came up empty on a network where a plain
    // socket sees the printer at once. Any address, any port: the query goes
    // out from that port and the answers come back to it, which is how a
    // one-shot mDNS query works (RFC 6762 section 5.1).
    if(!socket->bind())
    {
        qDebug() << "could not bind the discovery socket:" << socket->errorString();
    }
#endif
    // Qt 4.7 has no pointer-to-member connect.
    connect(socket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    connect(this, SIGNAL(favouritesChanged()), this, SLOT(cleanUpdate()));
    _transactionid = 0;
}

IppDiscovery::~IppDiscovery() {
    delete socket;
}

IppDiscovery* IppDiscovery::m_Instance = nullptr;

IppDiscovery* IppDiscovery::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new IppDiscovery;

        mutex.unlock();
    }

    return m_Instance;
}

void IppDiscovery::discover() {
    // Not a braced list: Qt 4.7's QList has no initializer-list constructor.
    QStringList services;
    services << "_ipp._tcp.local";
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    services << "_ipps._tcp.local";
#else
    // Harmattan is left out of the encrypted half on purpose. A printer that
    // advertises both is listed as ipps:// by update(), and the N9's curl
    // (7.21 against OpenSSL 0.9.8) cannot negotiate with a printer made this
    // decade -- so the entry would be there and simply never work, with no way
    // for the user to reach the ipp:// twin. Plain IPP on port 631 is what
    // this device can do; an ipps address can still be typed in by hand.
#endif
    sendQuery(PTR, services);
}

void IppDiscovery::reset() {
    _ipp.clear();
    _rps.clear();
    _ports.clear();
    _targets.clear();

    _AAs.clear();
    _AAAAs.clear();

    _outstandingQueries.clear();

    setStringList({});
    discover();
}


void IppDiscovery::sendQuery(quint16 qtype, QStringList addrs) {
    addrs.removeDuplicates();

    QTime now = QTime::currentTime();
    QTime aWhileAgo = now.addSecs(-1);

    for(QPair<quint16, QString> oq : _outstandingQueries.keys())
    {
        if(_outstandingQueries[oq] < aWhileAgo)
        { // Housekeeping for _outstandingQueries
            _outstandingQueries.remove(oq);
        }
        else if(oq.first == qtype && addrs.contains(oq.second))
        { // we recently asked about this, remove it
            addrs.removeOne(oq.second);
        }
    }

    if(addrs.empty())
    {
        qDebug() << "nothing to do";
        return;
    }

    qDebug() << "discovering" << qtype << addrs;

    Bytestream query;
    QMap<QString, quint16> suffixPositions;

    quint16 flags = 0;
    quint16 questions = addrs.length();

    query << _transactionid++ << flags << questions << (quint16)0 << (quint16)0 << (quint16)0;

    for(QString addr : addrs)
    {
        _outstandingQueries.insert({qtype, addr}, now);

        QStringList addrParts = addr.split(".");
        QString addrPart, restAddr;
        while(!addrParts.isEmpty())
        {
            restAddr = addrParts.join(".");
            if(suffixPositions.contains(restAddr))
            {
                query << (quint16)(0xc000 | (0x0fff & suffixPositions[restAddr]));
                break;
            }
            else
            {
                // We are putting in at least one part of the address, remember where that was
                suffixPositions.insert(restAddr, query.size());
                addrPart = addrParts.takeFirst();
                query << (quint8)addrPart.size() << addrPart.toStdString();
            }
        }
        if(addrParts.isEmpty())
        {
            // Whole addr was put in without c-pointers, 0-terminate it
            query << (quint8)0;
        }

        query << qtype << (quint16)0x0001;

    }

    socket->writeDatagram((char*)(query.raw()), query.size(), QHostAddress("224.0.0.251"), 5353);
}

QString make_addr(QString proto, int defaultPort, quint16 port, QString ip, QString rp)
{
    QString maybePort = port != defaultPort ? ":"+QString::number(port) : "";
    QString addr = proto+"://"+ip+maybePort+"/"+rp;
    return addr;
}

QString make_ipp_addr(quint16 port, QString ip, QString rp)
{
    return make_addr("ipp", 631, port, ip, rp);
}

QString make_ipps_addr(quint16 port, QString ip, QString rp)
{
    return make_addr("ipps", 443, port, ip, rp);
}

void IppDiscovery::cleanUpdate()
{
    setStringList(_favourites);
    update();
}

void IppDiscovery::update()
{
    QStringList found;
    QList<QPair<QString,QString>> ippsIpRps;
    QString target, rp;

    for(QString it : _ipps)
    {
        if(!_targets.contains(it) || !_ports.contains(it) || !_rps.contains(it))
            continue;

        quint16 port = _ports.value(it);
        target = _targets.value(it);
        rp = _rps.value(it);


        for(QMultiMap<QString,QString>::Iterator ait = _AAs.begin(); ait != _AAs.end(); ait++)
        {
            if(ait.key() == target)
            {
                QString ip = ait.value();
                QString addr = make_ipps_addr(port, ip, rp);
                if(!found.contains(addr))
                {
                    ippsIpRps.append({ip, rp});
                    found.append(addr);
                }
            }
        }
    }

    for(QString it : _ipp)
    {
        if(!_targets.contains(it) || !_ports.contains(it) || !_rps.contains(it))
            continue;

        quint16 port = _ports.value(it);
        target = _targets.value(it);
        rp = _rps.value(it);

        for(QMultiMap<QString,QString>::Iterator ait = _AAs.begin(); ait != _AAs.end(); ait++)
        {
            if(ait.key() == target)
            {
                QString ip = ait.value();
                QString addr = make_ipp_addr(port, ip, rp);

                                         // IP+RP assumed unique, don't add ipp version of the printer if ipps already added
                if(!found.contains(addr) && !ippsIpRps.contains({ip, rp}))
                {
                    found.append(addr);
                }
            }
        }
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    found.sort(Qt::CaseInsensitive);
#else
    // QStringList::sort() takes no case sensitivity before Qt 5.
    qSort(found.begin(), found.end(), caseInsensitiveLessThan);
#endif

    // Counting on that _ipp duplicates doesn't resolve fully any erlier than their _ipps counterpart

    // TODO?: replace this with some logic that can bpoth add and remove
    // and it can consider _favourites, so we can drop cleanUpdate
    for(QString f : found)
    {
        if(!this->stringList().contains(f))
        {
            if(this->insertRow(this->rowCount()))
            {
                QModelIndex index = this->index(this->rowCount() - 1, 0);
                this->setData(index, f);
            }
        }
    }
}

void IppDiscovery::updateAndQueryPtrs(QStringList& ptrs, QStringList new_ptrs)
{
    new_ptrs.removeDuplicates();
    for(QString ptr : new_ptrs)
    {
        if(!ptrs.contains(ptr))
        {
            ptrs.append(ptr);
        }
        // If pointer does not resolve to a target or is missing information, query about it
        if(!_rps.contains(ptr))
        {
            // Avahi *really* hates sending TXT to anything else than a TXT query
            qDebug() << "querying txt " << ptr;
            sendQuery(TXT, QStringList(ptr));
        }
        if(!_targets.contains(ptr) || !_ports.contains(ptr))
        {
            qDebug() << "querying srv " << ptr;
            sendQuery(SRV, QStringList(ptr));
        }
    }
}

void IppDiscovery::readPendingDatagrams()
{
    while (socket->hasPendingDatagrams()) {

        size_t size = socket->pendingDatagramSize();
        Bytestream resp(size);
        QHostAddress sender;
        quint16 senderPort;

        QStringList new_ipp_ptrs;
        QStringList new_ipps_ptrs;
        QStringList new_targets;

        QString qaddr, aaddr, tmpname, target;

        socket->readDatagram((char*)(resp.raw()), size, &sender, &senderPort);
        sender = QHostAddress(sender.toIPv4Address());

        quint16 transactionid, flags, questions, answerRRs, authRRs, addRRs;

        try {

            resp >> transactionid >> flags >> questions >> answerRRs >> authRRs >> addRRs;

            for(quint16 i = 0; i < questions; i++)
            {
                quint16 qtype, qflags;
                qaddr = get_addr(resp).join(".");
                resp >> qtype >> qflags;
            }

            for(quint16 i = 0; i < answerRRs; i++)
            {
                quint16 atype, aflags, len;
                quint32 ttl;

                aaddr = get_addr(resp).join(".");
                resp >> atype >> aflags >> ttl >> len;

                quint16 pos_before = resp.pos();
                if (atype == PTR)
                {
                    tmpname = get_addr(resp).join(".");
                    if(aaddr.endsWith("_ipp._tcp.local"))
                    {
                        new_ipp_ptrs.append(tmpname);
                    }
                    else if(aaddr.endsWith("_ipps._tcp.local"))
                    {
                        new_ipps_ptrs.append(tmpname);
                    }
                }
                else if(atype == TXT)
                {
                    Bytestream tmp;
                    while(resp.pos() < pos_before+len)
                    {
                        resp/resp.getU8() >> tmp;
                        if(tmp >>= "rp=")
                        {
                            std::string tmprp;
                            tmp/tmp.remaining() >> tmprp;
                            qDebug() << "tmprp" << aaddr << tmprp.c_str();
                            _rps[aaddr] = tmprp.c_str();
                        }
                    }
                }
                else if (atype == SRV)
                {
                    quint16 prio, w, port;
                    resp >> prio >> w >> port;
                    target = get_addr(resp).join(".");
                    _ports[aaddr] = port;
                    _targets[aaddr] = target;
                    if(!new_targets.contains(target))
                    {
                        new_targets.append(target);
                    }
                }
                else if(atype == A)
                {
                    quint32 addr;
                    resp >> addr;
                    QHostAddress haddr(addr);
                    if(!_AAs.contains(aaddr, haddr.toString()))
                    {
                        _AAs.insert(aaddr, haddr.toString());
                    }
                }
                else
                {
                    resp += len;
                }
                Q_ASSERT(resp.pos() == pos_before+len);

            }
        }
        catch(const std::exception& e)
        {
            qDebug() << e.what();
            return;
        }
        qDebug() << "new ipp ptrs" << new_ipp_ptrs;
        qDebug() << "new ipps ptrs" << new_ipps_ptrs;
        qDebug() << "ipp ptrs" << _ipp;
        qDebug() << "ipps ptrs" << _ipps;
        qDebug() << "rps" << _rps;
        qDebug() << "ports" << _ports;
        qDebug() << "new targets" << new_targets;
        qDebug() << "targets" << _targets;
        qDebug() << "AAs" << _AAs;
        qDebug() << "AAAAs" << _AAAAs;

        // These will send one query per unique new ptr.
        // some responders doesn't give TXT records for more than one thing at at time :(
        updateAndQueryPtrs(_ipps, new_ipps_ptrs);
        updateAndQueryPtrs(_ipp, new_ipp_ptrs);

        QStringList unresolvedAddrs;

        for(QString t : new_targets)
        {
            // If target does not resolve to an address, query about it
            if(!_AAs.contains(t))
            {
                unresolvedAddrs.append(t);
            }
        }

        if(!unresolvedAddrs.empty())
        {
            sendQuery(A, unresolvedAddrs);
        }

    }
    this->update();

}

bool IppDiscovery::resolve(QUrl& url)
{
    QString host = url.host();

    if(host.endsWith("."))
    {
        host.chop(1);
    }

    // TODO IPv6
    if(_AAs.contains(host))
    {   // TODO: retry potential other IPs
        url.setHost(_AAs.value(host));
        return true;
    }
    return false;
}
