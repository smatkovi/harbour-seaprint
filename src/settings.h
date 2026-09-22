#ifndef SETTINGS_H
#define SETTINGS_H

#include <mlite5/MGConfItem>
#include <QObject>

class Settings : public QObject
   {
       Q_OBJECT
public:

    Q_PROPERTY(QString ignoreSslErrorsPath READ ignoreSslErrorsPath CONSTANT)
    Q_PROPERTY(bool ignoreSslErrorsDefault READ ignoreSslErrorsDefault CONSTANT)

    Q_PROPERTY(QString debugLogPath READ debugLogPath CONSTANT)
    Q_PROPERTY(bool debugLogDefault READ debugLogDefault CONSTANT)

    Q_PROPERTY(QString allowExternalConnectionsPath READ allowExternalConnectionsPath CONSTANT)
    Q_PROPERTY(bool allowExternalConnectionsDefault READ allowExternalConnectionsDefault CONSTANT)

public:
    static Settings* instance();

    // Getters rather than MEMBER, which moc 4.7 does not know.
    bool ignoreSslErrorsDefault() const {return _ignoreSslErrorsDefault;}
    bool debugLogDefault() const {return _debugLogDefault;}
    bool allowExternalConnectionsDefault() const {return _allowExternalConnectionsDefault;}

    bool ignoreSslErrors();
    bool debugLog();
    bool allowExternalConnections();

    QString ignoreSslErrorsPath();
    QString debugLogPath();
    QString allowExternalConnectionsPath();

private:
    Settings();
    ~Settings();
    Settings(const Settings &);
    Settings& operator=(const Settings &);

    static Settings* m_Instance;

    MGConfItem _ignoreSslErrorsSetting;
    MGConfItem _debugLogSetting;
    MGConfItem _allowExternalConnectionsSetting;

    bool _ignoreSslErrorsDefault = true;
    bool _debugLogDefault = false;
    bool _allowExternalConnectionsDefault = false;

};

#endif // SETTINGS_H
