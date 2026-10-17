#include "convertproc.h"

#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QStringList>
#include <QtDebug>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

namespace {

const char* ARG = "--convert";

// The parameters as plain lines, one per field: the parent and the child are
// the same binary from the same build, but writing the struct out as bytes
// would still be a trap -- it holds a string and a vector. Names and numbers
// cost nothing here and can be read in a log.
QByteArray serializeParams(const PrintParameters& p)
{
    QByteArray out;
    out += "format "         + QByteArray::number((int)p.format)         + "\n";
    out += "colorMode "      + QByteArray::number((int)p.colorMode)      + "\n";
    out += "quality "        + QByteArray::number((int)p.quality)        + "\n";
    out += "antiAlias "      + QByteArray::number(p.antiAlias ? 1 : 0)   + "\n";
    out += "hwResW "         + QByteArray::number((qulonglong)p.hwResW)  + "\n";
    out += "hwResH "         + QByteArray::number((qulonglong)p.hwResH)  + "\n";
    out += "paperSizeUnits " + QByteArray::number((int)p.paperSizeUnits) + "\n";
    out += "paperSizeW "     + QByteArray::number(p.paperSizeW)          + "\n";
    out += "paperSizeH "     + QByteArray::number(p.paperSizeH)          + "\n";
    out += "duplexMode "     + QByteArray::number((int)p.duplexMode)     + "\n";
    out += "backXformMode "  + QByteArray::number((int)p.backXformMode)  + "\n";
    out += "copies "         + QByteArray::number((qulonglong)p.copies)  + "\n";
    out += "collatedCopies " + QByteArray::number(p.collatedCopies ? 1 : 0) + "\n";
    out += "mediaPosition "  + QByteArray::number((int)p.mediaPosition)  + "\n";
    for(size_t i = 0; i < p.pageRangeList.size(); i++)
    {
        out += "pageRange " + QByteArray::number((qulonglong)p.pageRangeList[i].first)
             + " " + QByteArray::number((qulonglong)p.pageRangeList[i].second) + "\n";
    }
    // The two strings last on their line, so that anything in them survives.
    out += "paperSizeName " + QByteArray(p.paperSizeName.c_str()) + "\n";
    out += "mediaType "     + QByteArray(p.mediaType.c_str())     + "\n";
    return out;
}

PrintParameters parseParams(const QByteArray& text)
{
    PrintParameters p;
    p.pageRangeList.clear();

    const QList<QByteArray> lines = text.split('\n');
    for(int i = 0; i < lines.size(); i++)
    {
        const QByteArray line = lines.at(i).trimmed();
        if(line.isEmpty())
        {
            continue;
        }
        const int space = line.indexOf(' ');
        if(space < 0)
        {
            continue;
        }
        const QByteArray key = line.left(space);
        const QByteArray value = line.mid(space + 1);
        const long number = value.toLong();

        if(key == "format")              p.format = (PrintParameters::Format)number;
        else if(key == "colorMode")      p.colorMode = (PrintParameters::ColorMode)number;
        else if(key == "quality")        p.quality = (PrintParameters::Quality)number;
        else if(key == "antiAlias")      p.antiAlias = number != 0;
        else if(key == "hwResW")         p.hwResW = (size_t)number;
        else if(key == "hwResH")         p.hwResH = (size_t)number;
        else if(key == "paperSizeUnits") p.paperSizeUnits = (PrintParameters::PaperSizeUnits)number;
        else if(key == "paperSizeW")     p.paperSizeW = value.toFloat();
        else if(key == "paperSizeH")     p.paperSizeH = value.toFloat();
        else if(key == "duplexMode")     p.duplexMode = (PrintParameters::DuplexMode)number;
        else if(key == "backXformMode")  p.backXformMode = (PrintParameters::BackXformMode)number;
        else if(key == "copies")         p.copies = (size_t)number;
        else if(key == "collatedCopies") p.collatedCopies = number != 0;
        else if(key == "mediaPosition")  p.mediaPosition = (PrintParameters::MediaPosition)number;
        else if(key == "paperSizeName")  p.paperSizeName = std::string(value.constData());
        else if(key == "mediaType")      p.mediaType = std::string(value.constData());
        else if(key == "pageRange")
        {
            const QList<QByteArray> ends = value.split(' ');
            if(ends.size() == 2)
            {
                p.pageRangeList.push_back(std::make_pair((size_t)ends.at(0).toLong(),
                                                         (size_t)ends.at(1).toLong()));
            }
        }
    }
    return p;
}

// Progress travels on stderr as one line per page, next to whatever poppler
// and cairo have to say there.
const char* PROGRESS = "seaprint-progress";

}

bool isConvertChild(int argc, char** argv)
{
    return argc >= 3 && QByteArray(argv[1]) == ARG;
}

int convertChildMain(int argc, char** argv)
{
    // Be the first thing the kernel takes if memory runs out. That is the
    // whole point of this process: its death is survivable, the app's is not.
    // Raising one's own oom_adj needs no privileges; failing is no reason not
    // to convert.
    QFile oomAdj("/proc/self/oom_adj");
    if(oomAdj.open(QIODevice::WriteOnly))
    {
        oomAdj.write("15\n");
        oomAdj.close();
    }

    const QString file = QString::fromLocal8Bit(argv[2]);
    const bool verbose = argc > 3 && QByteArray(argv[3]) == "-v";

    // The parameters arrive on stdin and are complete when it closes. Plain
    // stdio for both ends of the pipe: a QFile on a pipe has to guess about
    // sizes, and here the only thing that matters is reading to the end and
    // writing every byte.
    QByteArray text;
    char buffer[4096];
    size_t got;
    while((got = fread(buffer, 1, sizeof(buffer), stdin)) > 0)
    {
        text.append(buffer, (int)got);
    }
    const PrintParameters params = parseParams(text);

    WriteFun writeFun([](unsigned char const* buf, unsigned int len) -> bool
    {
        if(len == 0)
        {
            return true;
        }
        // A short write means the other end is gone; saying so stops the
        // conversion instead of filling a pipe nobody reads.
        return fwrite(buf, 1, len, stdout) == len;
    });

    ProgressFun progressFun([](size_t page, size_t total) -> void
    {
        fprintf(stderr, "%s %lu %lu\n", PROGRESS, (unsigned long)page, (unsigned long)total);
        fflush(stderr);
    });

    const int res = pdf_to_printable(file.toStdString(), writeFun, params, progressFun, verbose);
    if(fflush(stdout) != 0)
    {
        fprintf(stderr, "convert: could not flush the output\n");
        return 3;
    }
    return res == 0 ? 0 : 2;
}

bool convertInChildProcess(const QString& file, const PrintParameters& params, bool verbose,
                           const WriteFun& writeFun, const ProgressFun& progressFun,
                           QString* error)
{
    const QString self = QCoreApplication::applicationFilePath();
    QStringList args;
    args << ARG << file;
    if(verbose)
    {
        args << "-v";
    }

    QProcess child;
    child.setProcessChannelMode(QProcess::SeparateChannels);
    child.setReadChannel(QProcess::StandardOutput);
    child.start(self, args);
    if(!child.waitForStarted(10000))
    {
        if(error) *error = QCoreApplication::translate("PrinterWorker", "Could not start the conversion");
        qDebug() << "convert: could not start" << self << child.errorString();
        return false;
    }

    child.write(serializeParams(params));
    child.closeWriteChannel();

    QByteArray errLine;
    bool writeFailed = false;

    // Whatever the child says on stderr: the progress lines drive the busy
    // page, the rest goes to the log, where poppler's complaints belong.
    auto drainStderr = [&]()
    {
        errLine += child.readAllStandardError();
        int nl;
        while((nl = errLine.indexOf('\n')) >= 0)
        {
            const QByteArray line = errLine.left(nl).trimmed();
            errLine = errLine.mid(nl + 1);
            if(line.startsWith(PROGRESS))
            {
                const QList<QByteArray> parts = line.split(' ');
                if(parts.size() == 3 && progressFun != nullptr)
                {
                    progressFun((size_t)parts.at(1).toLong(), (size_t)parts.at(2).toLong());
                }
            }
            else if(!line.isEmpty())
            {
                qDebug() << "convert:" << line.constData();
            }
        }
    };

    while(child.state() == QProcess::Running || child.bytesAvailable() > 0)
    {
        // No event loop here -- this runs in the worker thread, where the
        // blocking half of QProcess is the honest way to pump a pipe.
        if(!child.waitForReadyRead(120000) && child.state() == QProcess::Running
           && child.bytesAvailable() == 0)
        {
            qDebug() << "convert: the child went quiet for two minutes";
            child.kill();
            child.waitForFinished(5000);
            if(error) *error = QCoreApplication::translate("PrinterWorker", "The conversion stopped responding");
            return false;
        }

        const QByteArray chunk = child.readAllStandardOutput();
        if(!chunk.isEmpty() && !writeFun((unsigned char const*)chunk.constData(), chunk.size()))
        { // The printer connection is gone; no reason to keep converting.
            writeFailed = true;
            child.kill();
            child.waitForFinished(5000);
            break;
        }
        drainStderr();
    }

    child.waitForFinished(10000);
    const QByteArray rest = child.readAllStandardOutput();
    if(!writeFailed && !rest.isEmpty())
    {
        writeFailed = !writeFun((unsigned char const*)rest.constData(), rest.size());
    }
    drainStderr();

    if(writeFailed)
    {
        if(error) *error = QCoreApplication::translate("PrinterWorker", "Could not send the document");
        return false;
    }

    if(child.exitStatus() != QProcess::NormalExit)
    {
        // The kernel takes the most expendable process when memory runs out,
        // and this one asked to be it. Anything else that kills it ends up
        // here too, so the message says what was seen, not why.
        qDebug() << "convert: the child was killed, code" << child.exitCode();
        if(error) *error = QCoreApplication::translate("PrinterWorker", "The conversion was stopped - the memory was probably not enough");
        return false;
    }
    if(child.exitCode() != 0)
    {
        qDebug() << "convert: the child failed, code" << child.exitCode();
        if(error) *error = QCoreApplication::translate("PrinterWorker", "Conversion failed");
        return false;
    }
    return true;
}
