#!/usr/bin/env python3
# Sitzt zwischen App und Drucker und schreibt mit, wer wann was sagt.
#
#   meego/tests/ipp-relay.py [port] [drucker] [drucker-port] [ablage]
#
# Die App druckt auf ipp://<rechner>:<port>/ipp/printer, der Verkehr geht
# unveraendert an den echten Drucker weiter, und hier steht hinterher, in
# welcher Reihenfolge und mit welchen Pausen die Bytes geflossen sind -- die
# einzige Art, ein "der Drucker antwortet zu frueh und macht zu" zu belegen,
# statt es zu vermuten.
#
# Je Verbindung entsteht eine Datei mit dem Hin- und Rueckverkehr als
# Zeitachse; die Nutzlast selbst wird daneben roh abgelegt.
import os
import socket
import struct
import sys
import threading
import time


def pump(source, sink, log, direction, state, store):
    raw = open(os.path.join(store, "%s-%d.bin" % (direction, state["n"])), "wb")
    total = 0
    try:
        while True:
            data = source.recv(65536)
            if not data:
                break
            total += len(data)
            raw.write(data)
            raw.flush()
            head = data[:80].replace(b"\r\n", b" | ")
            log("%7.2f s  %-4s %6d Byte (gesamt %8d)  %s"
                % (time.time() - state["start"], direction, len(data), total,
                   head.decode("latin-1", "replace")[:70]))
            sink.sendall(data)
    except Exception as problem:
        log("%7.2f s  %-4s Ende: %s" % (time.time() - state["start"], direction, problem))
    finally:
        raw.close()
        try:
            sink.shutdown(socket.SHUT_WR)
        except Exception:
            pass


def handle(client, addr, printer_host, printer_port, store, counter):
    counter[0] += 1
    state = {"start": time.time(), "n": counter[0]}
    path = os.path.join(store, "verkehr-%d.txt" % counter[0])
    handle_ = open(path, "w")

    def log(line):
        handle_.write(line + "\n")
        handle_.flush()
        print(line, flush=True)

    log("== Verbindung %d von %s nach %s:%d" % (counter[0], addr[0], printer_host, printer_port))
    try:
        printer = socket.create_connection((printer_host, printer_port), 15)
    except Exception as problem:
        log("  Drucker nicht erreichbar: %s" % problem)
        client.close()
        return

    threads = [threading.Thread(target=pump, args=(client, printer, log, "App->", state, store)),
               threading.Thread(target=pump, args=(printer, client, log, "<-Drk", state, store))]
    for thread in threads:
        thread.daemon = True
        thread.start()
    for thread in threads:
        thread.join()
    log("== Verbindung %d zu, nach %.2f s" % (counter[0], time.time() - state["start"]))
    handle_.close()
    client.close()
    printer.close()


def main(argv):
    port = int(argv[1]) if len(argv) > 1 else 8634
    printer_host = argv[2] if len(argv) > 2 else "192.168.1.5"
    printer_port = int(argv[3]) if len(argv) > 3 else 631
    store = argv[4] if len(argv) > 4 else "/tmp/ipp-relay"
    os.makedirs(store, exist_ok=True)

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("", port))
    server.listen(5)
    print("Weiterleiter auf Port %d -> %s:%d, Ablage %s"
          % (port, printer_host, printer_port, store), flush=True)
    counter = [0]
    while True:
        client, addr = server.accept()
        threading.Thread(target=handle,
                         args=(client, addr, printer_host, printer_port, store, counter),
                         daemon=True).start()


if __name__ == "__main__":
    sys.exit(main(sys.argv))
