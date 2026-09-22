// The file list behind the picker pages.
//
// Sailfish hands SeaPrint a DocumentPickerPage and an ImagePickerPage out of
// Sailfish.Pickers; Harmattan has no such thing, and its QtQuick 1.1
// FolderListModel cannot even say which entries are directories. So the
// listing is done here, where QDir already knows, and meego/qml/silica/
// FilePickerPage.qml only draws it.
#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class FileBrowser : public QObject
{
    Q_OBJECT

public:
    explicit FileBrowser(QObject* parent = 0);

    // Directories first, then the files whose suffix is in `suffixes` (all
    // files if it is empty), each as {name, path, isDir, size, suffix}.
    Q_INVOKABLE QVariantList list(const QString& path, const QStringList& suffixes) const;

    // Where the user's own files are: MyDocs on the N9, the home directory
    // anywhere else.
    Q_INVOKABLE QString home() const;
    Q_INVOKABLE QString parentOf(const QString& path) const;
    Q_INVOKABLE QString displayPath(const QString& path) const;
    Q_INVOKABLE bool isDir(const QString& path) const;
    Q_INVOKABLE QString sizeString(qint64 bytes) const;
};

#endif // FILEBROWSER_H
