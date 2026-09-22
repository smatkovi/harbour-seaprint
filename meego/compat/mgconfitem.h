// Sailfish reads its settings through mlite5's MGConfItem (a GConf key), and
// the QML reads the same keys through Nemo.Configuration's ConfigurationValue.
// Harmattan has GConf as well, but nothing else in this port needs
// libmeegotouch, so both names are served out of one QSettings file in
// ~/.config/harbour-seaprint/. The keys keep their GConf spelling
// ("/apps/harbour-seaprint/settings/debug-log") because the QML names those
// paths and writes them back.
//
// Instances sharing a key tell each other about a write, as GConf would: the
// C++ singleton and the ConfigurationValue on the settings page are two
// objects looking at the same value.
#ifndef MGCONFITEM_H
#define MGCONFITEM_H

#include <QObject>
#include <QString>
#include <QVariant>

class MGConfItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(QVariant value READ valueOrDefault WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE setDefaultValue NOTIFY defaultValueChanged)

public:
    explicit MGConfItem(const QString& key, QObject* parent = 0);
    explicit MGConfItem(QObject* parent = 0);
    ~MGConfItem();

    QString key() const { return _key; }
    void setKey(const QString& key);

    QVariant value() const;
    QVariant value(const QVariant& def) const;
    QVariant valueOrDefault() const { return value(_default); }

    QVariant defaultValue() const { return _default; }
    void setDefaultValue(const QVariant& value);

    void set(const QVariant& value);
    void unset();

    // Called by the store when another instance writes the same key.
    void peerChanged() { emit valueChanged(); }

signals:
    void keyChanged();
    void valueChanged();
    void defaultValueChanged();

public slots:
    void setValue(const QVariant& value) { set(value); }

private:
    QString _key;
    QVariant _default;
};

#endif // MGCONFITEM_H
