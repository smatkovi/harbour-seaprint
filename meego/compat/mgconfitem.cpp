#include "mgconfitem.h"

#include <QCoreApplication>
#include <QDir>
#include <QList>
#include <QSettings>

namespace {

// One file for the whole app, named after the app rather than after the
// organisation, so the path is the obvious one on the device:
// ~/.config/harbour-seaprint/harbour-seaprint.conf
QSettings& store()
{
    static QSettings settings(QDir::homePath() + "/.config/harbour-seaprint/harbour-seaprint.conf",
                              QSettings::IniFormat);
    return settings;
}

QList<MGConfItem*>& items()
{
    static QList<MGConfItem*> list;
    return list;
}

// "/apps/harbour-seaprint/settings/debug-log" is a GConf path; QSettings would
// read a leading slash as an empty first group.
QString settingsKey(const QString& key)
{
    QString k = key;
    while(k.startsWith('/'))
    {
        k.remove(0, 1);
    }
    return k;
}

}

MGConfItem::MGConfItem(const QString& key, QObject* parent) : QObject(parent), _key(key)
{
    items().append(this);
}

MGConfItem::MGConfItem(QObject* parent) : QObject(parent)
{
    items().append(this);
}

MGConfItem::~MGConfItem()
{
    items().removeAll(this);
}

void MGConfItem::setKey(const QString& key)
{
    if(key == _key)
    {
        return;
    }
    _key = key;
    emit keyChanged();
    emit valueChanged();
}

void MGConfItem::setDefaultValue(const QVariant& value)
{
    if(value == _default)
    {
        return;
    }
    _default = value;
    emit defaultValueChanged();
    if(!store().contains(settingsKey(_key)))
    {
        emit valueChanged();
    }
}

QVariant MGConfItem::value() const
{
    return _key.isEmpty() ? QVariant() : store().value(settingsKey(_key));
}

QVariant MGConfItem::value(const QVariant& def) const
{
    if(_key.isEmpty())
    {
        return def;
    }
    const QVariant v = store().value(settingsKey(_key), def);
    // QSettings reads everything back from the ini file as a string; the
    // callers ask for bools and ints, so follow the type of the default.
    if(def.isValid() && v.isValid() && v.type() != def.type())
    {
        QVariant converted = v;
        if(def.type() == QVariant::Bool)
        {
            const QString s = v.toString().toLower();
            return QVariant(s == "true" || s == "1");
        }
        if(converted.convert(def.type()))
        {
            return converted;
        }
    }
    return v;
}

void MGConfItem::set(const QVariant& value)
{
    if(_key.isEmpty() || value == this->value())
    {
        return;
    }
    store().setValue(settingsKey(_key), value);
    store().sync();

    for(MGConfItem* item : items())
    {
        if(item->key() == _key)
        {
            item->peerChanged();
        }
    }
}

void MGConfItem::unset()
{
    if(_key.isEmpty())
    {
        return;
    }
    store().remove(settingsKey(_key));
    store().sync();

    for(MGConfItem* item : items())
    {
        if(item->key() == _key)
        {
            item->peerChanged();
        }
    }
}
