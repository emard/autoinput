#!/bin/sh
# cat /usr/local/bin/autoinput.sh
sleep 1

exec /usr/bin/autoinput --device=/dev/ttyUSB.RADWAG-APP25.3Y \
  --protocol=radwag --bps=4800 --parity=none --stopbits=1 \
  --udp-addr=225.0.0.25 \
  --pcspeaker="aplay -q /usr/share/sounds/sound-icons/beginning-of-line &"
