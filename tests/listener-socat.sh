#!/bin/bash
## socat STDIO UDP4-DATAGRAM:192.168.88.255:5000,broadcast,range=192.168.88.0/24
socat -d -d pty,raw,echo=0 UDP4-RECVFROM:32001,broadcast,reuseaddr,fork 2>/tmp/listener.txt &
disown
sleep 7
cat /tmp/listener.txt
device=$(grep PTY /tmp/listener.txt | sed -e "s#.*\(/dev/pts/[0-9]*\)#\1#g")
./caliper --verbose --device=$device
