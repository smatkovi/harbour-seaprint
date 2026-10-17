#!/usr/bin/env python
# Asks a printer for its attributes over plain HTTP and saves the raw IPP body.
#
#   meego/tests/ipp-capture.py 192.168.1.5 /ipp/printer attrs.raw [laeufe]
#
# The point is to have the exact bytes a printer sends, without the app in the
# way: meego/tests/run-parse-test.sh then puts the parser through them. It
# speaks no library beyond the standard one and runs on the device's Python 2.6
# as well as on Python 3, so the same capture can be taken from the phone's own
# network position.
import socket
import struct
import sys

def attribute(tag, name, value):
    name = name.encode("ascii") if hasattr(name, "encode") else name
    value = value.encode("utf-8") if hasattr(value, "encode") else value
    return struct.pack(">BH", tag, len(name)) + name + struct.pack(">H", len(value)) + value

def get_printer_attributes(host, rp, user="user"):
    # Operation 0x000b, IPP 1.1, with the four operation attributes SeaPrint
    # sends -- in that order, because printers have been known to care.
    body = struct.pack(">BBHI", 1, 1, 0x000B, 1) + b"\x01"
    body += attribute(0x47, "attributes-charset", "utf-8")
    body += attribute(0x48, "attributes-natural-language", "en-us")
    body += attribute(0x45, "printer-uri", "ipp://%s%s" % (host, rp))
    body += attribute(0x42, "requesting-user-name", user)
    body += b"\x03"

    head = ("POST %s HTTP/1.1\r\nHost: %s:631\r\nContent-Type: application/ipp\r\n"
            "Content-Length: %d\r\nAccept: */*\r\n\r\n" % (rp, host, len(body)))
    request = head.encode("ascii") + body

    sock = socket.create_connection((host, 631), 10)
    sock.settimeout(20)
    sock.sendall(request)
    # The printer keeps the connection open, so the end of the body has to be
    # worked out from the headers rather than waited for.
    buf = b""
    length = None
    while True:
        chunk = sock.recv(65536)
        if not chunk:
            break
        buf += chunk
        if length is None and b"\r\n\r\n" in buf:
            headers = buf.split(b"\r\n\r\n")[0].lower()
            if b"content-length:" in headers:
                length = int(headers.split(b"content-length:")[1].split(b"\r\n")[0])
        if length is not None and len(buf.split(b"\r\n\r\n", 1)[1]) >= length:
            break
    sock.close()
    headers, _, rest = buf.partition(b"\r\n\r\n")
    if b"chunked" in headers.lower():
        out, pos = b"", 0
        while True:
            eol = rest.index(b"\r\n", pos)
            size = int(rest[pos:eol].split(b";")[0], 16)
            if size == 0:
                break
            out += rest[eol + 2:eol + 2 + size]
            pos = eol + 2 + size + 2
        rest = out
    return headers.split(b"\r\n")[0].decode("latin-1"), rest

def main(argv):
    host = argv[1] if len(argv) > 1 else "192.168.1.5"
    rp = argv[2] if len(argv) > 2 else "/ipp/printer"
    out = argv[3] if len(argv) > 3 else "attrs.raw"
    runs = int(argv[4]) if len(argv) > 4 else 1

    for run in range(runs):
        status, body = get_printer_attributes(host, rp)
        ipp_status = struct.unpack(">H", body[2:4])[0] if len(body) >= 4 else -1
        print("%s | %d Byte | IPP-Status 0x%04x" % (status, len(body), ipp_status))
        if run == 0:
            handle = open(out, "wb")
            handle.write(body)
            handle.close()
            print("-> %s" % out)
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv))
