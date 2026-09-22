#include "tintedimage.h"

#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QtDebug>

#include "themeicons.h"

TintedImage::TintedImage(QDeclarativeItem* parent)
    : QQuickPaintedItem(parent), _color(Qt::white), _tinted(false)
{
}

void TintedImage::setSource(const QString& source)
{
    if(source == _source)
    {
        return;
    }
    _source = source;
    load();
    emit sourceChanged();
    update();
}

void TintedImage::setColor(const QColor& color)
{
    if(color == _color)
    {
        return;
    }
    _color = color;
    emit colorChanged();
    update();
}

void TintedImage::setTinted(bool tinted)
{
    if(tinted == _tinted)
    {
        return;
    }
    _tinted = tinted;
    emit tintedChanged();
    update();
}

void TintedImage::load()
{
    const QString path = themeIconFile(_source);
    _image = QImage();
    if(path.isEmpty())
    {
        qDebug() << "no icon for" << _source;
        return;
    }
    if(!_image.load(path))
    {
        qDebug() << "could not load" << path;
    }
}

void TintedImage::paint(QPainter* painter)
{
    if(_image.isNull())
    {
        return;
    }

    const QSize target = QSize(qRound(width()), qRound(height()));
    if(target.isEmpty())
    {
        return;
    }

    QImage image = _image.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if(_tinted)
    {
        // Keep the icon's own alpha, replace its colour: SourceIn paints the
        // colour only where the icon is opaque.
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        QPainter tint(&image);
        tint.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tint.fillRect(image.rect(), _color);
        tint.end();
    }

    // Centred, like an Image with fillMode PreserveAspectFit.
    const int x = qRound((width() - image.width()) / 2.0);
    const int y = qRound((height() - image.height()) / 2.0);
    painter->drawImage(x, y, image);
}
