#!/bin/sh
# > nc -lu -p 5000 &
# echo “HELLO” | socat - UDP-DATAGRAM:192.168.88.255:32001,broadcast
./mt.tcl | socat - UDP-DATAGRAM:192.168.88.255:32001,broadcast
# socat -d -d pty,raw,echo=0 UDP-DATAGRAM:192.168.88.255:32001,broadcast 2>/tmp/mettler-sender.txt &
