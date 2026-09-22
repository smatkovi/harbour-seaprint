#include "clipboard.h"

#include <QApplication>
#include <QClipboard>

QString Clipboard::text() const
{
    return QApplication::clipboard()->text();
}

void Clipboard::setText(const QString& text)
{
    QApplication::clipboard()->setText(text);
    emit textChanged();
}
