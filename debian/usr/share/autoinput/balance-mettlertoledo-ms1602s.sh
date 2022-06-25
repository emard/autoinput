#!/bin/sh
# cat /usr/local/bin/autoinput.sh
sleep 1
exec /usr/bin/autoinput --device=/dev/ttyUSB.METTLER-TOLEDO-MS1602S \
  --protocol=simple --bps=9600 \
  --udp-addr=225.0.0.16
