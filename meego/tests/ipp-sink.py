#!/usr/bin/env python3
# Ein IPP-Drucker, der nichts druckt, sondern mitschreibt.
#
#   meego/tests/ipp-sink.py [port] [ablage] [attrs.raw]
#
# Die App druckt dann auf ipp://<rechner>:<port>/ipp/print, und hier landen die
# Anfrage im Klartext (Operation, Attribute) und das Dokument als Datei. Das ist
# der einzige Weg, die Bytes zu sehen, die wirklich hinausgehen: curls
# Mitschrift zeigt nur die Kopfzeilen, und ein echter Drucker sagt zu dem, was
# ihm nicht passt, meist nur "bad request".
#
# Auf Get-Printer-Attributes antwortet er mit einer mitgeschnittenen Antwort
# eines echten Druckers, wenn eine angegeben ist (meego/tests/ipp-capture.py
# holt sie) -- sonst wuesste die App nicht, was der Drucker kann, und wuerde
# gar nicht erst anfangen zu drucken.
#
# Sonst antwortet er mit successful-ok und einer Auftragsnummer, nimmt
# gestueckelte Rumpfe entgegen und laesst die Verbindung offen, bis der Client
# fertig ist -- also gerade nicht das, was der HP tut, wenn ihm etwas missfaellt.
import os
import socket
import struct
import sys
import threading
import time

TAGS = {0x21: "integer", 0x22: "boolean", 0x23: "enum", 0x30: "octetString",
        0x31: "dateTime", 0x32: "resolution", 0x33: "rangeOfInteger",
        0x34: "begCollection", 0x35: "textWithLanguage", 0x36: "nameWithLanguage",
        0x37: "endCollection", 0x41: "textWithoutLanguage", 0x42: "nameWithoutLanguage",
        0x44: "keyword", 0x45: "uri", 0x46: "uriScheme", 0x47: "charset",
        0x48: "naturalLanguage", 0x49: "mimeMediaType", 0x4a: "memberAttrName"}
GROUPS = {0x01: "operation-attributes", 0x02: "job-attributes",
          0x04: "printer-attributes", 0x05: "unsupported-attributes"}
OPERATIONS = {0x0002: "Print-Job", 0x0004: "Validate-Job", 0x0005: "Create-Job",
              0x0006: "Send-Document", 0x0008: "Cancel-Job", 0x0009: "Get-Job-Attributes",
              0x000a: "Get-Jobs", 0x000b: "Get-Printer-Attributes"}


def describe(data):
    """Die IPP-Attribute einer Anfrage, und wo das Dokument anfaengt."""
    out = []
    version = "%d.%d" % (data[0], data[1])
    operation = struct.unpack(">H", data[2:4])[0]
    out.append("IPP %s, %s (0x%04x), request-id %d"
               % (version, OPERATIONS.get(operation, "?"), operation,
                  struct.unpack(">I", data[4:8])[0]))
    pos = 8
    name = ""
    while pos < len(data):
        tag = data[pos]
        if tag < 0x10:
            pos += 1
            if tag == 0x03:
                break
            out.append("  -- %s" % GROUPS.get(tag, "group 0x%02x" % tag))
            continue
        name_len = struct.unpack(">H", data[pos+1:pos+3])[0]
        this_name = data[pos+3:pos+3+name_len].decode("latin-1")
        at = pos + 3 + name_len
        value_len = struct.unpack(">H", data[at:at+2])[0]
        value = data[at+2:at+2+value_len]
        pos = at + 2 + value_len
        if this_name:
            name = this_name
        if tag in (0x21, 0x23) and value_len == 4:
            shown = str(struct.unpack(">i", value)[0])
        elif tag == 0x32 and value_len == 9:
            x, y, units = struct.unpack(">IIB", value)
            shown = "%dx%d %s" % (x, y, "dpi" if units == 3 else "units %d" % units)
        elif tag == 0x22:
            shown = str(bool(value[0])) if value_len else "(leer)"
        else:
            shown = value.decode("latin-1", "replace")
        out.append("     %-34s %-20s %s" % (name if this_name else "(weiter)",
                                            TAGS.get(tag, "0x%02x" % tag), shown))
    return "\n".join(out), pos


def handle(conn, addr, store, counter, attributes):
    conn.settimeout(120)
    data = b""
    try:
        while b"\r\n\r\n" not in data:
            chunk = conn.recv(65536)
            if not chunk:
                return
            data += chunk
        head, _, rest = data.partition(b"\r\n\r\n")
        headers = head.decode("latin-1")
        chunked = "chunked" in headers.lower()
        length = None
        for line in headers.split("\r\n"):
            if line.lower().startswith("content-length:"):
                length = int(line.split(":", 1)[1])

        body = b""
        if chunked:
            buf = rest
            while True:
                while b"\r\n" not in buf:
                    more = conn.recv(65536)
                    if not more:
                        break
                    buf += more
                if b"\r\n" not in buf:
                    break
                size_line, _, buf = buf.partition(b"\r\n")
                size = int(size_line.split(b";")[0], 16)
                if size == 0:
                    break
                while len(buf) < size + 2:
                    more = conn.recv(65536)
                    if not more:
                        break
                    buf += more
                body += buf[:size]
                buf = buf[size+2:]
        else:
            body = rest
            while length is not None and len(body) < length:
                more = conn.recv(65536)
                if not more:
                    break
                body += more

        print("\n=== %s, %s, %d Byte Rumpf%s"
              % (time.strftime("%H:%M:%S"), addr[0], len(body), " (gestueckelt)" if chunked else ""))
        for line in headers.split("\r\n")[:1]:
            print("   ", line)
        try:
            text, document_at = describe(body)
            print(text)
            document = body[document_at:]
            if document:
                counter[0] += 1
                path = os.path.join(store, "job%d.bin" % counter[0])
                with open(path, "wb") as handle_:
                    handle_.write(document)
                print("  Dokument: %d Byte -> %s" % (len(document), path))
            else:
                print("  kein Dokument im Rumpf")
        except Exception as problem:
            print("  (Anfrage nicht lesbar: %s)" % problem)

        if len(body) >= 4 and struct.unpack(">H", body[2:4])[0] == 0x000b and attributes:
            # Get-Printer-Attributes: die mitgeschnittene Antwort, mit der
            # request-id dieser Anfrage.
            reply = attributes[:4] + body[4:8] + attributes[8:]
            conn.sendall(("HTTP/1.1 200 OK\r\nContent-Type: application/ipp\r\n"
                          "Content-Length: %d\r\nConnection: close\r\n\r\n" % len(reply)).encode() + reply)
            print("  (Druckerattribute aus der Aufzeichnung geliefert)")
            return

        # Immer erfolgreich, mit einer Auftragsnummer.
        reply = struct.pack(">BBHI", 1, 1, 0x0000, 1) + b"\x01"
        for tag, name, value in ((0x47, "attributes-charset", b"utf-8"),
                                 (0x48, "attributes-natural-language", b"en-us")):
            reply += struct.pack(">BH", tag, len(name)) + name.encode() + struct.pack(">H", len(value)) + value
        reply += b"\x02"
        name = b"job-id"
        reply += struct.pack(">BH", 0x21, len(name)) + name + struct.pack(">H", 4) + struct.pack(">I", counter[0])
        name = b"job-state"
        reply += struct.pack(">BH", 0x23, len(name)) + name + struct.pack(">H", 4) + struct.pack(">I", 9)
        reply += b"\x03"
        conn.sendall(("HTTP/1.1 200 OK\r\nContent-Type: application/ipp\r\n"
                      "Content-Length: %d\r\nConnection: close\r\n\r\n" % len(reply)).encode() + reply)
    except Exception as problem:
        print("  (Verbindung: %s)" % problem)
    finally:
        conn.close()


def main(argv):
    port = int(argv[1]) if len(argv) > 1 else 8633
    store = argv[2] if len(argv) > 2 else "/tmp/ipp-sink"
    attributes = b""
    if len(argv) > 3:
        with open(argv[3], "rb") as handle_:
            attributes = handle_.read()
    os.makedirs(store, exist_ok=True)
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("", port))
    server.listen(5)
    print("IPP-Mithoerer auf Port %d, Ablage %s" % (port, store))
    counter = [0]
    while True:
        conn, addr = server.accept()
        threading.Thread(target=handle, args=(conn, addr, store, counter, attributes),
                         daemon=True).start()


if __name__ == "__main__":
    sys.exit(main(sys.argv))
