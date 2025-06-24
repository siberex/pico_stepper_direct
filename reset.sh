#!/usr/bin/env bash
set -euo pipefail

BASEDIR="$( cd "$(dirname "$0")" ; pwd -P )"

# Use `picotool reboot -f` to reboot via USB

if nc -z localhost 4444; then
    echo Resetting via Telnet interface...
    echo "reset" | nc localhost 4444
else
    echo Resetting via openocd...
    openocd -f "${BASEDIR}/openocd.cfg" -c "init; reset; exit"
fi
