// Silica's Clipboard singleton, which the pages use to copy a line of text.
// Qt 4 has the same clipboard behind QApplication; this only gives QML a
// property to assign to.
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QString>

class Clipboard : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit Clipboard(QObject* parent = 0) : QObject(parent) {}

    QString text() const;
    void setText(const QString& text);

signals:
    void textChanged();
};

#endif // CLIPBOARD_H
