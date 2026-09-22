// Force-included (-include) into every translation unit of the MeeGo build.
//
// The things Qt 4.7 has no spelling for at all, so that the shared sources in
// src/ and ppm2pwg/ keep the form they have upstream. Everything else the port
// needs is a header next to this one (QJsonObject, QMimeDatabase, QPdfWriter,
// ...), found because meego/compat comes first on the include path.
#pragma once
#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <initializer_list>

#ifndef QStringLiteral
#define QStringLiteral(str) QString::fromUtf8(str)
#endif
#ifndef QByteArrayLiteral
#define QByteArrayLiteral(str) QByteArray(str)
#endif

// Q_ENUM is Qt 5; Q_ENUMS does the same registration here. moc 4.7 swallows
// the declaration that follows Q_ENUMS, so whatever comes after it in a class
// must be something moc does not need to see.
#ifndef Q_ENUM
#define Q_ENUM(x) Q_ENUMS(x)
#endif

// Qt 4.7's QList has no initializer-list constructor (Qt 4.8 got one). The few
// places that build a fixed list of strings say QSL{...} instead.
namespace qt4compat
{
inline QStringList stringList(std::initializer_list<const char*> items)
{
    QStringList list;
    for(const char* item : items)
    {
        list.append(QString::fromUtf8(item));
    }
    return list;
}
}
#define QSL qt4compat::stringList

// Harmattan carries poppler 0.16 as libpoppler-glib.so.6; Sailfish has .so.8.
// The name is dlopen()ed, so it is a string and not a link-time dependency.
#ifndef POPPLER_GLIB_SO
#define POPPLER_GLIB_SO "libpoppler-glib.so.6"
#endif

#endif // QT_VERSION
