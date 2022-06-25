#!/bin/sh

# autoinput broadcasts UDP data in "simple" protocol

# debug: dump ip traffic:
# tcpdump -i enp3s0 -X -t udp port 32001

# debug: broadcast receiver (socat) prints to stdout (works only for 1st transmission)
# socat STDIO udp4-listen:32001

# multicast receiver (socat) fork on UDP arrival it exec's "xclip" command
# that copies UDP payload to paste buffer
# click middle button to paste
# 32001 is port server sends to
# 225.0.0.1 is multicast destination address server sends to
# autoinput --udp-addr=225.0.0.1
# 192.168.0.1 is local ethernet card IP address
socat UDP4-RECVFROM:32001,ip-add-membership=225.0.0.1:192.168.0.1,fork EXEC:xclip

# broadcast receiver (socat) fork on UDP arrival it exec's "xclip" command
# server sends to broadcast address
# autoinput --udp-addr=255.255.255.255
# that copies UDP payload to paste buffer
# click middle button to paste
# 32001 is port server sends to
#socat udp4-recvfrom:32001,fork EXEC:xclip
