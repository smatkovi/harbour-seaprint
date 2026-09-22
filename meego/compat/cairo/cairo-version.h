/* The device's cairo, as reported by its soname: libcairo.so.2.11000.0.
 *
 * cairo's own tarball ships this file as a placeholder that the build
 * generates; the MeeGo build compiles against the headers of the version the
 * N9 actually carries, so the numbers are written out here. pdf2printable.cpp
 * tests CAIRO_VERSION to leave out what 1.10 does not have.
 */
#ifndef CAIRO_VERSION_H
#define CAIRO_VERSION_H

#define CAIRO_VERSION_MAJOR 1
#define CAIRO_VERSION_MINOR 10
#define CAIRO_VERSION_MICRO 0

#endif
