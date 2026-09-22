/* Poppler's types, without poppler's headers.
 *
 * ppm2pwg never links against poppler: every call goes through dlsym() (see
 * ppm2pwg/lib/madness.h and meego/libfuncs), so the only thing the compiler
 * needs is the name of the two objects being passed around as pointers. The
 * N9's libpoppler-glib.so.6 is poppler 0.16, whose glib headers are not in the
 * MADDE sysroot and would drag in a generated poppler-enums.h and a dozen
 * files describing an API this code never touches.
 */
#ifndef POPPLER_DOCUMENT_SHIM_H
#define POPPLER_DOCUMENT_SHIM_H

#include <glib.h>
#include <glib-object.h>

typedef struct _PopplerDocument PopplerDocument;
typedef struct _PopplerPage PopplerPage;

#endif
