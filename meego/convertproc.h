#ifndef CONVERTPROC_H
#define CONVERTPROC_H

// The conversion in a process of its own.
//
// Turning a page into something a printer takes is the one step in this app
// that asks for real memory: a big scanned PDF peaks at 34 MB (measured with
// meego/tests/print-watch.sh), a page of raster at printer resolution needs
// ten times that. The user interface holds some 35 MB of its own, and an N950
// in daily use has about 70 MB left over -- so the two together sat right at
// the edge, and when the kernel had to take something, it took the app in the
// middle of the job. To the person holding the phone that looked like the app
// quitting by itself, with nothing in the log to say why.
//
// So the app starts itself again with --convert, hands the parameters over on
// stdin, reads the converted stream back from stdout and passes it on to the
// printer. The child makes itself the kernel's first choice (oom_adj), carries
// no user interface, and dies with the job -- and if it is killed, the app is
// still there to say so.
#include <QString>

#include "pdf2printable.h"
#include "printparameters.h"

// Runs the conversion in a child and feeds what it produces to writeFun.
// Returns false and fills error with something sayable if the child could not
// be started, died, or the conversion itself failed.
bool convertInChildProcess(const QString& file, const PrintParameters& params, bool verbose,
                           const WriteFun& writeFun, const ProgressFun& progressFun,
                           QString* error);

// True when these are the arguments of a conversion child.
bool isConvertChild(int argc, char** argv);

// What the child does, called from main() before anything else exists.
int convertChildMain(int argc, char** argv);

#endif // CONVERTPROC_H
