#include <poppler/glib/poppler.h>
#include <poppler/glib/poppler-document.h>

#include "convertchecker.h"
#include <QProcess>
#include <QtDebug>

#ifndef POPPLER_GLIB_SO
// Sailfish carries poppler as libpoppler-glib.so.8; the MeeGo build points
// this at the .so.6 the N9 has (meego/compat/qt4compat.h).
#define POPPLER_GLIB_SO "libpoppler-glib.so.8"
#endif

ConvertChecker::ConvertChecker() : libpoppler(POPPLER_GLIB_SO)
{
    _calligra = false;

    QProcess calligraconverter(this);
    // QProcess::setProgram()/setArguments() are Qt 5.1; this spelling starts
    // the same process on both. On Harmattan there is no calligraconverter,
    // so the check simply answers no and the office formats stay hidden.
    calligraconverter.start("calligraconverter", QStringList() << "-h");

    if(calligraconverter.waitForFinished(2000))
    {
      if(calligraconverter.exitStatus() == QProcess::NormalExit && calligraconverter.exitCode() == 0)
      {
          _calligra = true;
      }
    }
}

ConvertChecker::~ConvertChecker() {

}

ConvertChecker* ConvertChecker::m_Instance = 0;

ConvertChecker* ConvertChecker::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new ConvertChecker;

        mutex.unlock();
    }

    return m_Instance;
}

int ConvertChecker::pdfPages(QString filename)
{
// glib initialises its type system by itself only from 2.36 on. On Harmattan
// (glib 2.24) poppler's first call walks an empty type table and the process
// dies on the spot, so the old entry point is called first.
#if !GLIB_CHECK_VERSION(2, 36, 0)
  g_type_init();
#endif

    FUNC(poppler, PopplerDocument*, poppler_document_new_from_file, const char*, const char*, GError**);
    FUNC(poppler, int, poppler_document_get_n_pages, PopplerDocument*);
    std::string url("file://");
    url.append(filename.toStdString());
    GError* error = nullptr;
    PopplerDocument* doc = poppler_document_new_from_file(url.c_str(), nullptr, &error);

    int pages = poppler_document_get_n_pages(doc);

    g_object_unref(doc);
    return pages;
}
