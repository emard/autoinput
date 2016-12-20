#!/bin/sh
# for protocol "denver"
# echo " 1234.567\\r" | socat - UDP:localhost:32001
# for protocol "simple"
echo "1234.567" | socat - UDP:localhost:32001
