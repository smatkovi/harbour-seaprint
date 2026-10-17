#!/usr/bin/env python
# Ein Drucker, der nur behauptet, einer zu sein -- zum Pruefen der Suche.
# Laeuft AUF DEM GERAET (Python 2.6) neben der Probe:
#
#   python mdns-stub.py [--loss 50] [--port 8632] [--host 192.168.1.21] &
#   sh discovery-runs.sh 10 6
#
# Es beantwortet die vier Fragen, die SeaPrint stellt (PTR, TXT, SRV, A), und
# mit --loss laesst es den angegebenen Anteil der Antworten absichtlich fallen.
# So laesst sich das Nachfragen pruefen, ohne auf den echten Drucker und sein
# Funkglueck zu warten: ohne Nachfragen findet die Suche bei 50 % Verlust in
# der Haelfte der Laeufe nichts.
#
# Es antwortet **nur auf Fragen vom eigenen Geraet** -- ins Heimnetz hinein
# wird nichts angekuendigt, damit in keinem fremden Druckdialog ein Geist
# auftaucht.
import random
import select
import socket
import struct
import sys

GROUP = "224.0.0.251"
PORT = 5353
SERVICE = "_ipp._tcp.local"
INSTANCE = "SeaPrintTest." + SERVICE
TARGET = "seaprinttest.local"

PTR, TXT, SRV, A = 12, 16, 33, 1

def encode_name(name):
    out = b""
    for part in name.split("."):
        out += struct.pack(">B", len(part)) + part
    return out + b"\x00"

def decode_name(data, pos):
    parts = []
    while True:
        length = ord(data[pos:pos+1])
        pos += 1
        if length == 0:
            break
        if length & 0xc0:  # Zeiger: in einer Frage kommt das nicht vor
            pos += 1
            break
        parts.append(data[pos:pos+length])
        pos += length
    return ".".join(parts), pos

def record(name, rtype, payload, ttl=120):
    return encode_name(name) + struct.pack(">HHIH", rtype, 1, ttl, len(payload)) + payload

def answer_for(name, qtype, host, port):
    lowered = name.lower()
    if qtype in (PTR, 255) and lowered == SERVICE:
        return record(SERVICE, PTR, encode_name(INSTANCE))
    if lowered == INSTANCE.lower():
        if qtype in (TXT, 255):
            txt = b"rp=ipp/print"
            return record(INSTANCE, TXT, struct.pack(">B", len(txt)) + txt)
        if qtype in (SRV, 255):
            payload = struct.pack(">HHH", 0, 0, port) + encode_name(TARGET)
            return record(INSTANCE, SRV, payload)
    if qtype in (A, 255) and lowered == TARGET:
        return record(TARGET, A, socket.inet_aton(host))
    return None

def own_addresses():
    # Alles, was auf diesem Geraet eine Adresse ist -- fremde Fragen werden
    # nicht beantwortet.
    addresses = set(["127.0.0.1"])
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.connect(("192.168.1.1", 53))
        addresses.add(sock.getsockname()[0])
        sock.close()
    except Exception:
        pass
    return addresses

def main(argv):
    loss = 0
    port = 8632
    host = "192.168.1.21"
    i = 1
    while i < len(argv):
        if argv[i] == "--loss":
            loss = int(argv[i+1]); i += 2
        elif argv[i] == "--port":
            port = int(argv[i+1]); i += 2
        elif argv[i] == "--host":
            host = argv[i+1]; i += 2
        else:
            print "unbekannt: %s" % argv[i]; return 2

    mine = own_addresses()
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("", PORT))
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP,
                    socket.inet_aton(GROUP) + socket.inet_aton("0.0.0.0"))
    print "Attrappe laeuft: %s -> ipp://%s:%d/ipp/print, %d%% Verlust, antwortet nur %s" % (
        INSTANCE, host, port, loss, ", ".join(sorted(mine)))
    sys.stdout.flush()

    while True:
        ready, _, _ = select.select([sock], [], [], 1.0)
        if not ready:
            continue
        data, src = sock.recvfrom(4096)
        if src[0] not in mine:
            continue
        try:
            txid, flags, questions = struct.unpack(">HHH", data[0:6])
            pos = 12
            answers = b""
            asked = b""
            count = 0
            for _ in range(questions):
                name, pos = decode_name(data, pos)
                qtype, qclass = struct.unpack(">HH", data[pos:pos+4])
                pos += 4
                asked += encode_name(name) + struct.pack(">HH", qtype, qclass)
                rr = answer_for(name, qtype, host, port)
                if rr is not None:
                    answers += rr
                    count += 1
        except Exception, e:
            continue
        if count == 0:
            continue
        if loss and random.randint(1, 100) <= loss:
            print "  (Antwort auf %s absichtlich fallen gelassen)" % name
            sys.stdout.flush()
            continue
        head = struct.pack(">HHHHHH", txid, 0x8400, questions, count, 0, 0)
        sock.sendto(head + asked + answers, src)
        print "  beantwortet: %s (Typ %d) an %s:%d" % (name, qtype, src[0], src[1])
        sys.stdout.flush()

if __name__ == "__main__":
    sys.exit(main(sys.argv))
