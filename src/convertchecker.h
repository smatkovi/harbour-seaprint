#ifndef CONVERTCHECKER_H
#define CONVERTCHECKER_H
#include <QObject>
#include <QMutex>
#include "madness.h"

class ConvertChecker : public QObject
{
    Q_OBJECT
public:
    static ConvertChecker* instance();
    Q_PROPERTY(bool calligra READ calligra CONSTANT)

    bool calligra() const {return _calligra;}

    Q_INVOKABLE int pdfPages(QString pdf);

signals:
protected:
private:
    static ConvertChecker* m_Instance;

    ConvertChecker();
    ~ConvertChecker();
    ConvertChecker(const ConvertChecker &);
    ConvertChecker& operator=(const ConvertChecker &);

    LibLoader libpoppler;
    bool _calligra;
};

#endif // CONVERTCHECKER_H
