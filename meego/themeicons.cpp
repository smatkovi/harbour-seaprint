#include "themeicons.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

namespace {

struct IconName
{
    const char* silica;
    const char* blanco;
};

// The "-inverse" variants are the white ones, which is what a dark page needs.
const IconName IconNames[] = {
    {"icon-m-file-pdf",            "icon-m-content-pdf-inverse"},
    {"icon-m-file-document",       "icon-m-content-document-inverse"},
    {"icon-m-file-image",          "icon-m-content-image-inverse"},
    {"icon-m-file-formatted",      "icon-m-content-word-inverse"},
    {"icon-m-file-presentation",   "icon-m-content-open-document-presentation-inverse"},
    {"icon-m-file-other",          "icon-m-content-file-unknown-inverse"},
    {"icon-m-file-archive-folder", "icon-m-common-directory"},
    {"icon-m-warning",             "icon-l-error"},
    {"icon-s-warning",             "icon-l-error"},
    {"icon-lock-warning",          "icon-l-error"},
    {"icon-m-right",               "icon-m-common-drilldown-arrow-inverse"},
    {"icon-m-down",                "icon-m-common-expand"},
    {"icon-m-up",                  "icon-m-common-collapse"},
    {"icon-s-edit",                "icon-m-toolbar-edit-white"},
    {"icon-m-edit",                "icon-m-toolbar-edit-white"},
    {"icon-m-printer",             "icon-m-common-printer"},
    {0, 0}
};

// The blanco icons on the device, and in the SDK when this runs on a desktop.
QString iconPath(const QString& name)
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

}

QString themeIconFile(const QString& source)
{
    if(source.isEmpty())
    {
        return QString();
    }

    QString name = source;
    if(name.startsWith("image://theme/"))
    {
        name = name.mid(QString("image://theme/").length());
    }
    else if(name.contains('/') || name.startsWith("file:"))
    {
        return name.startsWith("file://") ? name.mid(7) : name;
    }

    for(const IconName* icon = IconNames; icon->silica; icon++)
    {
        if(name == QLatin1String(icon->silica))
        {
            const QString path = iconPath(QLatin1String(icon->blanco));
            if(!path.isEmpty())
            {
                return path;
            }
        }
    }
    return iconPath(name);
}
