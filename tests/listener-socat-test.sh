#!/bin/sh
## socat STDIO UDP4-DATAGRAM:192.168.88.255:5000,broadcast,range=192.168.88.0/24
socat STDIO UDP4-RECVFROM:32001,broadcast,reuseaddr,fork
