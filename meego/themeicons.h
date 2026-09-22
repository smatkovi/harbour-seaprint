// Where a Silica icon name lands in Harmattan's icon set.
//
// The QML asks for image://theme/icon-m-file-pdf and the like. Harmattan has
// the same pictures under other names, in the blanco theme; this maps the ones
// SeaPrint uses and otherwise looks the name up as it stands, which is right
// for the components' own icons.
#ifndef THEMEICONS_H
#define THEMEICONS_H

#include <QString>

// The file for a source of the form "image://theme/<name>", a bare theme name,
// or a plain path. Empty if there is no such icon.
QString themeIconFile(const QString& source);

#endif // THEMEICONS_H
