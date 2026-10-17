#!/usr/bin/env python3
# Schickt eine Datei als Druckauftrag an einen IPP-Drucker -- ohne App, ohne
# Umwandlung, damit sich ein Format fuer sich allein pruefen laesst.
#
#   meego/tests/send-ipp.py <ipp://host[:port]/pfad> <datei> <format> [attr=wert ...]
#
#   meego/tests/send-ipp.py ipp://192.168.1.5/ipp/printer seite.pclm application/PCLm \
#       pclm-source-resolution=600x600
#
# Zwei Dinge, die dieser HP LaserJet 400 verlangt und die hier eingebaut sind:
# der Rumpf muss **gestueckelt** gehen (mit Content-Length ueber ein paar MB
# antwortet er gar nicht mehr), und zu application/PCLm gehoert
# pclm-source-resolution, sonst gibt es 0x0400 client-error-bad-request.
#
# Erlaubte Attributformen: name=600x600 (Auflösung), name=42 (Zahl),
# name=text (Schluesselwort). Sie gehen in die Auftragsattribute.
import socket
import struct
import sys
import time
from urllib.parse import urlparse

TAG_INTEGER = 0x21
TAG_RESOLUTION = 0x32
TAG_KEYWORD = 0x44
TAG_NAME = 0x42
TAG_URI = 0x45
TAG_CHARSET = 0x47
TAG_LANGUAGE = 0x48
TAG_MIMETYPE = 0x49

STATUS = {0x0000: "successful-ok", 0x0400: "client-error-bad-request",
          0x0401: "client-error-forbidden", 0x040a: "client-error-document-format-not-supported",
          0x040b: "client-error-attributes-or-values-not-supported",
          0x0500: "server-error-internal-error", 0x0501: "server-error-operation-not-supported"}


def attribute(tag, name, value):
    name = name.encode("ascii")
    return struct.pack(">BH", tag, len(name)) + name + struct.pack(">H", len(value)) + value


def text_attribute(tag, name, value):
    return attribute(tag, name, value.encode("utf-8"))


def job_attribute(spec):
    """'name=wert' in ein IPP-Attribut, Typ aus der Form des Werts."""
    name, _, value = spec.partition("=")
    if "x" in value and all(part.isdigit() for part in value.split("x", 1)):
        x, y = (int(part) for part in value.split("x", 1))
        return attribute(TAG_RESOLUTION, name, struct.pack(">IIB", x, y, 3))  # 3 = dpi
    if value.lstrip("-").isdigit():
        return attribute(TAG_INTEGER, name, struct.pack(">i", int(value)))
    return text_attribute(TAG_KEYWORD, name, value)


def print_job(url, path, document_format, job_attrs, chunk_size=32768):
    parsed = urlparse(url)
    host = parsed.hostname
    port = parsed.port or 631
    resource = parsed.path or "/"
    with open(path, "rb") as handle:
        data = handle.read()

    body = struct.pack(">BBHI", 1, 1, 0x0002, 1) + b"\x01"           # Print-Job
    body += text_attribute(TAG_CHARSET, "attributes-charset", "utf-8")
    body += text_attribute(TAG_LANGUAGE, "attributes-natural-language", "en-us")
    body += text_attribute(TAG_URI, "printer-uri", "ipp://%s%s" % (host, resource))
    body += text_attribute(TAG_NAME, "requesting-user-name", "user")
    body += text_attribute(TAG_NAME, "job-name", path.rsplit("/", 1)[-1])
    body += text_attribute(TAG_MIMETYPE, "document-format", document_format)
    if job_attrs:
        body += b"\x02" + b"".join(job_attribute(spec) for spec in job_attrs)
    body += b"\x03"

    head = ("POST %s HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: application/ipp\r\n"
            "Accept-Encoding: identity\r\nTransfer-Encoding: chunked\r\n\r\n"
            % (resource, host, port)).encode("ascii")

    started = time.time()
    sock = socket.create_connection((host, port), 15)
    sock.settimeout(300)
    sock.sendall(head)
    for piece in [body] + [data[i:i + chunk_size] for i in range(0, len(data), chunk_size)]:
        sock.sendall(("%x\r\n" % len(piece)).encode("ascii") + piece + b"\r\n")
    sock.sendall(b"0\r\n\r\n")
    print("%.1f MB gesendet in %.1f s, warte auf die Antwort" % (len(data) / 1048576, time.time() - started))

    buf = b""
    try:
        while True:
            chunk = sock.recv(65536)
            if not chunk:
                break
            buf += chunk
            headers = buf.split(b"\r\n\r\n")[0].lower()
            if b"content-length:" in headers:
                length = int(headers.split(b"content-length:")[1].split(b"\r\n")[0])
                if len(buf.split(b"\r\n\r\n", 1)[1]) >= length:
                    break
    except socket.timeout:
        print("Zeitüberschreitung beim Warten auf die Antwort")
    sock.close()

    if not buf:
        print("keine Antwort")
        return 1
    print(buf.split(b"\r\n")[0].decode("latin-1"))
    ipp = buf.split(b"\r\n\r\n", 1)[1]
    status = struct.unpack(">H", ipp[2:4])[0]
    job_id = None
    at = ipp.find(b"job-id")
    if at >= 0:
        job_id = struct.unpack(">I", ipp[at + 8:at + 12])[0]
    print("IPP-Status 0x%04x %s, job-id %s, nach %.1f s"
          % (status, STATUS.get(status, ""), job_id, time.time() - started))
    return 0 if status == 0 else 1


def main(argv):
    if len(argv) < 4:
        print("usage: send-ipp.py <ipp://host/pfad> <datei> <format> [attr=wert ...]")
        return 2
    return print_job(argv[1], argv[2], argv[3], argv[4:])


if __name__ == "__main__":
    sys.exit(main(sys.argv))
