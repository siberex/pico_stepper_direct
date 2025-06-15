#!/usr/bin/env bash
set -euo pipefail

BASEDIR="$( cd "$(dirname "$0")" ; pwd -P )"
openocd -f "${BASEDIR}/openocd_dbg.cfg"

# Same as:
# openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 10000" -c "init; rtt setup 0x20000000 0x42000 \"SEGGER RTT\"; rtt start; rtt server start 9090 0"
