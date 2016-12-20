#!/bin/sh
# cat /usr/local/bin/autoinput.sh
sleep 1

exec /usr/bin/autoinput --device=/dev/ttyUSB.RADWAG-AS220R2 \
  --protocol=bosch --bps=4800 --parity=none --stopbits=1 \
  --pcspeaker="aplay -q /usr/share/sounds/sound-icons/beginning-of-line &"
