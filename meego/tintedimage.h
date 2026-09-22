// An icon, optionally painted in one colour.
//
// Silica's Icon and HighlightImage tint a monochrome icon with a theme colour;
// QtQuick 1.1 has no shader effects and Qt 4.7 no QtGraphicalEffects, so the
// tint is done with QPainter here instead.
//
// It also resolves the icon names: the QML says image://theme/icon-m-warning,
// which is a Sailfish name. Harmattan's own set is in the blanco theme, under
// different names -- the table in tintedimage.cpp maps the ones SeaPrint uses,
// and anything else is looked up in the theme directory as it stands.
#ifndef TINTEDIMAGE_H
#define TINTEDIMAGE_H

#include <QColor>
#include <QImage>
#include <QQuickPaintedItem>
#include <QString>

class TintedImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool tinted READ tinted WRITE setTinted NOTIFY tintedChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY sourceChanged)

public:
    TintedImage(QDeclarativeItem* parent = 0);

    QString source() const { return _source; }
    void setSource(const QString& source);

    QColor color() const { return _color; }
    void setColor(const QColor& color);

    bool tinted() const { return _tinted; }
    void setTinted(bool tinted);

    bool valid() const { return !_image.isNull(); }

    void paint(QPainter* painter);

signals:
    void sourceChanged();
    void colorChanged();
    void tintedChanged();

private:
    void load();

    QString _source;
    QColor _color;
    bool _tinted;
    QImage _image;
};

#endif // TINTEDIMAGE_H
