#include "themeimageprovider.h"

#include "themeicons.h"

#include <QPixmap>
#include <QtDebug>

QPixmap ThemeImageProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize)
{
    const QString path = themeIconFile(id);
    QPixmap pixmap;

    if(path.isEmpty() || !pixmap.load(path))
    {
        qDebug() << "no theme icon for" << id;
        if(size)
        {
            *size = QSize();
        }
        return QPixmap();
    }

    if(size)
    {
        *size = pixmap.size();
    }
    if(requestedSize.isValid() && requestedSize != pixmap.size())
    {
        return pixmap.scaled(requestedSize.width() > 0 ? requestedSize.width() : pixmap.width(),
                             requestedSize.height() > 0 ? requestedSize.height() : pixmap.height(),
                             Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return pixmap;
}
