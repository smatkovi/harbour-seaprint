// image://theme/... for the ported pages.
//
// The pages name Sailfish's icons, and several of them are plain Image
// elements rather than the Icon stand-in, so the mapping has to sit in the
// engine's own provider. Names the table does not know are looked up in the
// theme directory unchanged, which is what com.nokia.meego's own components
// ask for.
#ifndef THEMEIMAGEPROVIDER_H
#define THEMEIMAGEPROVIDER_H

#include <QDeclarativeImageProvider>

class ThemeImageProvider : public QDeclarativeImageProvider
{
public:
    ThemeImageProvider() : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize);
};

#endif // THEMEIMAGEPROVIDER_H
