#!/bin/sh
# cat /usr/local/bin/autoinput.sh
sleep 1
exec /usr/bin/autoinput --device=/dev/ttyUSB.KERN-770GJ  \
  --protocol=denver --bps=1200 --parity=odd \
  --pcspeaker="aplay -q /usr/share/sounds/sound-icons/beginning-of-line &"
