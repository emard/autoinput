#!/bin/bash
socat -d -d pty,raw,echo=0 pty,raw,echo=0 2>/tmp/virtualcable.txt &
disown
sleep 1
cat /tmp/virtualcable.txt
