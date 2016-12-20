#!/bin/sh
# cat /usr/local/bin/autoinput.sh
sleep 1
exec /usr/bin/autoinput --device=/dev/ttyUSB.METTLER-TOLEDO-SG32001 \
  --protocol=mettlertoledo --bps=9600 --parity=even
