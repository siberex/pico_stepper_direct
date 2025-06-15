#!/usr/bin/env bash
set -euo pipefail

BASEDIR="$( cd "$(dirname "$0")" ; pwd -P )"

TARGET=stepper_direct

if nc -z localhost 4444; then
    echo Flashing via Telnet interface...
    # telnet localhost 4444
    echo "pwd" | nc localhost 4444
    echo "program ${BASEDIR}/build/${TARGET}.elf verify reset" | nc localhost 4444
else
    echo Flashing via openocd...
    openocd -f "${BASEDIR}/openocd.cfg" -c "program ${BASEDIR}/build/${TARGET}.elf verify reset exit"
fi
