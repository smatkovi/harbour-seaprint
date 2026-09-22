#include "filebrowser.h"

#include <QDir>
#include <QFileInfo>
#include <QVariantMap>

FileBrowser::FileBrowser(QObject* parent) : QObject(parent)
{
}

QString FileBrowser::home() const
{
    // On the N9 everything the user can see through USB lives under MyDocs;
    // the rest of the home directory is settings and caches.
    const QString myDocs = QDir::homePath() + "/MyDocs";
    if(QFileInfo(myDocs).isDir())
    {
        return myDocs;
    }
    return QDir::homePath();
}

QString FileBrowser::parentOf(const QString& path) const
{
    QDir dir(path);
    if(!dir.cdUp())
    {
        return path;
    }
    return dir.absolutePath();
}

QString FileBrowser::displayPath(const QString& path) const
{
    const QString base = home();
    if(path == base)
    {
        return "/";
    }
    if(path.startsWith(base + "/"))
    {
        return path.mid(base.length());
    }
    return path;
}

bool FileBrowser::isDir(const QString& path) const
{
    return QFileInfo(path).isDir();
}

QString FileBrowser::sizeString(qint64 bytes) const
{
    if(bytes >= 1024*1024)
    {
        return QString::number(bytes/(1024.0*1024.0), 'f', 1) + " MB";
    }
    if(bytes >= 1024)
    {
        return QString::number(bytes/1024) + " kB";
    }
    return QString::number(bytes) + " B";
}

QVariantList FileBrowser::list(const QString& path, const QStringList& suffixes) const
{
    QVariantList out;

    QDir dir(path.isEmpty() ? home() : path);
    if(!dir.exists())
    {
        return out;
    }

    QStringList wanted;
    for(const QString& suffix : suffixes)
    {
        wanted.append(suffix.toLower());
    }

    dir.setSorting(QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);
    const QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot |
                                                   QDir::Readable);

    for(const QFileInfo& info : entries)
    {
        if(info.fileName().startsWith("."))
        {
            continue;
        }
        if(!info.isDir() && !wanted.isEmpty() && !wanted.contains(info.suffix().toLower()))
        {
            continue;
        }

        QVariantMap entry;
        entry.insert("name", info.fileName());
        entry.insert("path", info.absoluteFilePath());
        entry.insert("isDir", info.isDir());
        entry.insert("size", info.isDir() ? QString() : sizeString(info.size()));
        entry.insert("suffix", info.suffix().toLower());
        out.append(entry);
    }

    return out;
}
