// Turning a photo the right way up on Qt 4.
//
// Qt 5.5 grew QImageReader::setAutoTransform(), which applies the EXIF
// orientation of a JPEG. Qt 4.7 hands the image over exactly as it is stored,
// and the N9's camera stores every picture in the sensor's own orientation
// with a tag saying how to turn it -- so without this, a photo taken upright
// comes out of the printer on its side.
#ifndef EXIF_H
#define EXIF_H

#include <QImage>
#include <QString>

// 1..8 as in the EXIF specification, 0 if the file says nothing.
int exifOrientation(const QString& fileName);

// The image turned and/or mirrored as its EXIF tag asks.
QImage applyExifTransform(const QImage& image, const QString& fileName);

#endif // EXIF_H
