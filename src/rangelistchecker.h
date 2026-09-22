#ifndef RANGELISTCHECKER_H
#define RANGELISTCHECKER_H

#include <QValidator>
#include <QMutex>
#include <QJsonArray>
#include <QJsonObject>

#include "printparameters.h"

class RangeListChecker : public QObject
{
    Q_OBJECT
public:
    static RangeListChecker* instance();

    // A QVariantList rather than a QJsonArray: QtDeclarative turns that into
    // a JavaScript array for the dialog, and Qt 5 does the same.
    Q_INVOKABLE QVariantList parse(QString str) const;

private:
    static RangeListChecker* m_Instance;

    RangeListChecker();
    ~RangeListChecker();
    RangeListChecker(const RangeListChecker &);
    RangeListChecker& operator=(const RangeListChecker &);

};

#endif // RANGELISTCHECKER_H
