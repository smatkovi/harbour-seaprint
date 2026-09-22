#include "tintedimage.h"

#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QtDebug>

namespace {

struct IconName
{
    const char* silica;
    const char* blanco;
};

// The icons SeaPrint asks for, and what Harmattan calls the same picture.
// The "-inverse" variants are the white ones, which is what a dark page needs.
const IconName IconNames[] = {
    {"icon-m-file-pdf",          "icon-m-content-pdf-inverse"},
    {"icon-m-file-document",     "icon-m-content-document-inverse"},
    {"icon-m-file-image",        "icon-m-content-image-inverse"},
    {"icon-m-file-formatted",    "icon-m-content-word-inverse"},
    {"icon-m-file-presentation", "icon-m-content-open-document-presentation-inverse"},
    {"icon-m-file-other",        "icon-m-content-file-unknown-inverse"},
    {"icon-m-file-archive-folder", "icon-m-common-directory"},
    {"icon-m-warning",           "icon-l-error"},
    {"icon-s-warning",           "icon-l-error"},
    {"icon-lock-warning",        "icon-l-error"},
    {"icon-m-right",             "icon-m-common-drilldown-arrow-inverse"},
    {"icon-m-down",              "icon-m-common-expand"},
    {"icon-m-up",                "icon-m-common-collapse"},
    {"icon-s-edit",              "icon-m-toolbar-edit-white"},
    {"icon-m-edit",              "icon-m-toolbar-edit-white"},
    {"icon-m-printer",           "icon-m-common-printer"},
    {0, 0}
};

// Where the blanco icons are: on the device, and in the SDK when the app runs
// on the desktop against the simulator's copy of the theme.
QString themeIconPath(const QString& name)
{
    QStringList dirs;
    dirs << "/usr/share/themes/blanco/meegotouch/icons"
         << "/usr/share/themes/base/meegotouch/icons"
         << QDir::homePath() + "/QtSDK/Desktop/Qt/474/gcc/harmattanthemes/blanco/meegotouch/icons";

    for(const QString& dir : dirs)
    {
        const QString path = dir + "/" + name + ".png";
        if(QFileInfo(path).exists())
        {
            return path;
        }
    }
    return QString();
}

QString resolve(const QString& source)
{
    if(source.isEmpty())
    {
        return QString();
    }
    if(!source.startsWith("image://theme/"))
    {
        QString path = source;
        if(path.startsWith("file://"))
        {
            path = path.mid(7);
        }
        return path;
    }

    const QString name = source.mid(QString("image://theme/").length());
    for(const IconName* icon = IconNames; icon->silica; icon++)
    {
        if(name == QLatin1String(icon->silica))
        {
            const QString path = themeIconPath(QLatin1String(icon->blanco));
            if(!path.isEmpty())
            {
                return path;
            }
        }
    }
    // Not in the table, or the mapped name is missing: try the name itself,
    // which is right for any icon that happens to be spelled the same way.
    return themeIconPath(name);
}

}

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
    const QString path = resolve(_source);
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
