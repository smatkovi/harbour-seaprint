#!/usr/bin/env python3
# Schreibt ein PCLm aus einem PDF -- der Prototyp zum C++-Schreiber.
#
#   meego/tests/make-pclm.py <datei.pdf> <aus.pclm> [erste] [letzte] [dpi] [streifenhoehe]
#
# PCLm ist eine enge Teilmenge von PDF: jede Seite besteht aus waagrechten
# Streifen als Bild-XObjects, die der Drucker einen nach dem anderen verarbeitet
# -- 16 Zeilen auf einmal, nie eine ganze Seite. Genau deshalb kommt ein
# Laserdrucker mit 384 MB damit durch, wo ihm das PDF einer Magazinseite den
# Speicher sprengt.
#
# Dieses Skript ist nicht fuer das Geraet gedacht (es rendert die ganze Seite
# auf einmal, mit pdftoppm), sondern um die Form zu belegen: welche
# Woerterbuch-Eintraege, welche Reihenfolge, welche Kompression der Drucker
# wirklich annimmt. Was sich hier als druckbar erweist, baut der C++-Schreiber
# danach bandweise nach.
#
# Ghostscripts eigenes pclm-Gerät (10.07.1) taugt dafuer nicht: es schreibt
# einen leeren Inhaltsstrom mit behaupteter Laenge, die Seiten zeichnen nichts.
import io
import os
import subprocess
import sys

from PIL import Image


class Pdf:
    """Ein PDF, das in einem Durchgang nach vorne geschrieben wird."""

    def __init__(self):
        self.out = bytearray()
        self.offsets = {}   # Objektnummer -> Offset
        self.next_num = 1

    def reserve(self):
        num = self.next_num
        self.next_num += 1
        return num

    def write(self, data):
        self.out += data if isinstance(data, bytes) else data.encode("latin-1")

    def obj(self, num, body, stream=None):
        self.offsets[num] = len(self.out)
        self.write("%d 0 obj\n" % num)
        self.write(body)
        if stream is not None:
            self.write("\nstream\n")
            self.write(stream)
            self.write("\nendstream")
        self.write("\nendobj\n")

    def finish(self, root):
        xref = len(self.out)
        highest = max(self.offsets) if self.offsets else 0
        self.write("xref\n0 %d\n" % (highest + 1))
        self.write("0000000000 65535 f \n")
        for num in range(1, highest + 1):
            self.write("%010d 00000 n \n" % self.offsets.get(num, 0))
        self.write("trailer\n<<\n/Size %d\n/Root %d 0 R\n>>\n" % (highest + 1, root))
        self.write("startxref\n%d\n%%%%EOF\n" % xref)
        return bytes(self.out)


def render(pdf_path, page, dpi):
    """Eine Seite als PIL-Bild, über pdftoppm."""
    # Ohne Ausgabepraefix schreibt pdftoppm das PPM nach stdout.
    out = subprocess.run(["pdftoppm", "-r", str(dpi), "-f", str(page), "-l", str(page), pdf_path],
                         check=True, stdout=subprocess.PIPE).stdout
    return Image.open(io.BytesIO(out)).convert("RGB")


def jpeg(image, quality):
    buf = io.BytesIO()
    image.save(buf, format="JPEG", quality=quality, optimize=False, progressive=False)
    return buf.getvalue()


def make(pdf_path, out_path, first, last, dpi, strip_height, quality=85):
    doc = Pdf()
    doc.write("%PDF-1.7\n%PCLm-1.0\n")

    catalog = doc.reserve()
    pages = doc.reserve()
    page_nums = []

    for page in range(first, last + 1):
        image = render(pdf_path, page, dpi)
        width, height = image.size
        strips = (height + strip_height - 1) // strip_height
        # PDF rechnet in Punkten (1/72 Zoll) und von unten links.
        width_pt = width * 72.0 / dpi
        height_pt = height * 72.0 / dpi

        page_num = doc.reserve()
        contents = doc.reserve()
        strip_nums = [doc.reserve() for _ in range(strips)]
        page_nums.append(page_num)

        # Die Seite nennt ihre Streifen, bevor sie kommen -- der Drucker liest
        # von vorne nach hinten und braucht die Namen zuerst.
        xobjects = " ".join("/Im%d %d 0 R" % (i, n) for i, n in enumerate(strip_nums))
        doc.obj(page_num,
                "<<\n/Type /Page\n/Parent %d 0 R\n/MediaBox [0 0 %.3f %.3f]\n"
                "/Resources <<\n/XObject <<\n%s\n>>\n>>\n/Contents %d 0 R\n>>"
                % (pages, width_pt, height_pt, xobjects, contents))

        # Jeder Streifen wird an seine Zeile gesetzt: oberster zuerst.
        ops = []
        for i in range(strips):
            rows = min(strip_height, height - i * strip_height)
            strip_pt = rows * 72.0 / dpi
            y_pt = height_pt - (i * strip_height) * 72.0 / dpi - strip_pt
            ops.append("q %.3f 0 0 %.3f 0 %.3f cm /Im%d Do Q" % (width_pt, strip_pt, y_pt, i))
        content = ("\n".join(ops) + "\n").encode("latin-1")
        doc.obj(contents, "<<\n/Length %d\n>>" % len(content), content)

        for i, num in enumerate(strip_nums):
            top = i * strip_height
            rows = min(strip_height, height - top)
            data = jpeg(image.crop((0, top, width, top + rows)), quality)
            doc.obj(num,
                    "<<\n/Type /XObject\n/Subtype /Image\n/Width %d\n/Height %d\n"
                    "/ColorSpace /DeviceRGB\n/BitsPerComponent 8\n/Filter /DCTDecode\n"
                    "/Length %d\n>>" % (width, rows, len(data)), data)
        image.close()

    doc.obj(pages, "<<\n/Type /Pages\n/Count %d\n/Kids [%s]\n>>"
            % (len(page_nums), " ".join("%d 0 R" % n for n in page_nums)))
    doc.obj(catalog, "<<\n/Type /Catalog\n/Pages %d 0 R\n>>" % pages)

    data = doc.finish(catalog)
    with open(out_path, "wb") as handle:
        handle.write(data)
    print("%s: %d Seite(n), %.1f MB" % (out_path, len(page_nums), len(data) / 1048576))


def main(argv):
    if len(argv) < 3:
        print(__doc__ or "usage: make-pclm.py <pdf> <out.pclm> [first] [last] [dpi] [strip]")
        return 2
    pdf_path, out_path = argv[1], argv[2]
    first = int(argv[3]) if len(argv) > 3 else 1
    last = int(argv[4]) if len(argv) > 4 else first
    dpi = int(argv[5]) if len(argv) > 5 else 600
    strip = int(argv[6]) if len(argv) > 6 else 16
    make(pdf_path, out_path, first, last, dpi, strip)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
