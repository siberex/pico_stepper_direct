# RP2040/RP2350 stepper motor control example

Stepper motor direct control example for Raspberry Pico MCU family.

Full steps or half-stepping with SIO (no microstepping, no PIO).

Compatible with stepper motor drivers of the DRV883* family.

Coils could be connected directly to the GPIO pins (without a driver) if the motor is able to use 12mA per coil or less and could work from 3.3V.


## Build

Prerequisites: cmake, binutils, [Pico SDK 2.1.1](https://github.com/raspberrypi/pico-sdk)

```bash
mkdir -p build && cd build
cmake -DPICO_USE_FASTEST_SUPPORTED_CLOCK=1 -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1 ..
# To clear CMake cache and reset compile options to defaults:
# cmake --fresh ..
make -j4
```

### Flash by SWD

Prerequisites: [openocd](https://github.com/raspberrypi/openocd)

```bash
# SWD flash (openocd required):
make flash

# Note: if gdb server is run, use this to flash via telnet interface:
./flash.sh
```

Reset device programmatically from the host:

```bash
# To reset MCU with SWD:
./reset.sh:
```

### Flash by USB

Prerequisites: [picotool](https://github.com/raspberrypi/picotool)

```bash
# BOOTSEL flash (picotool required):
make flash_usb

# Flash manually with picotool (`make flash_usb` does exactly this):
TARGET=stepper_direct
picotool load -fx ${TARGET}.uf2
```


## Debug

ℹ️ Note: Use `-f target/rp2350.cfg` or `-f target/rp2350-riscv.cfg` for RP2350.
ℹ️ Note: Use debug builds for symbol resolution to work: `cmake -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1 ..`

OpenOCD will start a telnet server (port `4444`) and two GDB servers (rp2040.core0: port `3333`, rp2040.core1: port `3334`):

```bash
# Attach to SWD at 10MHz:
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 10000"

# Open OCD console:
telnet localhost 4444
> halt
> resume
> reset run
# https://openocd.org/doc/html/General-Commands.html#resetcommand
> reset init

# GDB core0:
TARGET=stepper_direct
gdb "${TARGET}.elf" -ex "target extended-remote localhost:3333"
```


ℹ️ Note: RP2040’s ARM Cortex-M0 don't have Serial Wire Output (SWO) over SWD. RTT can be used instead.

### Segger RTT serial console setup over SWD

This will set up RTT console on `127.0.0.1` port `9090`. And gdb server on ports `3333` and `3334`.

Connect with [CoolTerm](https://freeware.the-meiers.org/), [VSCode Serial Monitor](https://marketplace.visualstudio.com/items?itemName=ms-vscode.vscode-serial-monitor), minicom, PuTTY, netcat, telnet or whatever.

In one command:

```bash
# With CMake:
make gdb
# With separate script:
./debug.sh
# Or run directly (`make gdb` does exactly this):
# openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 10000" -c "init; rtt setup 0x20000000 0x42000 \"SEGGER RTT\"; rtt start; rtt server start 9090 0"

rlwrap nc localhost 9090

cd build
TARGET=stepper_direct
# core0:
gdb "${TARGET}.elf" -ex "target extended-remote localhost:3333"
# core1:
gdb "${TARGET}.elf" -ex "target extended-remote localhost:3334"
```


Set up manually (how it works in detail):

```bash
# Verify CMakeLists.txt:
# pico_enable_stdio_rtt(${PROJECT} 1)

# Use the telnet interface to issue RTT-specific commands:
telnet localhost 4444

> halt
# RAM start address: 0x20000000 
# RP2040 SRAM size is 264KB = 0x42000
# RP2350 SRAM size is 520KB = 520 * 1024 = 532480 = 0x82000
> rtt setup 0x20000000 0x42000 "SEGGER RTT"
> rtt start

# Expected output:
# rtt: Searching for control block 'SEGGER RTT'
# rtt: Control block found at 0x20002ab0

# Unexpected output:
# SWD DPIDR 0x0bc12477, DLPIDR 0x00000001
# Failed to read memory at 0x20042000
# rtt: No control block found

# After setup succeed:
> rtt server start 9090 0
# OR, to dump all serial output to file:
#> rtt channelfile 0 rtt_output.txt

# Flash
> program stepper_direct.elf verify reset

# Separate by core:
#> rp2040.core0 rtt setup 0x20000000 0x42000 "SEGGER RTT"
#> rp2040.core0 rtt start
#> rp2040.core0 rtt server start 9090 0
#> rp2040.core1 rtt setup 0x20000000 0x42000 "SEGGER RTT"
#> rp2040.core1 ...

```

See also: https://kb.segger.com/Raspberry_Pi_RP2040


### Other On-Chip Debuggers

#### [probe-rs](https://probe.rs/)

```bash 
# brew install probe-rs/probe-rs/probe-rs
# OR: cargo install probe-rs-tools --locked
probe-rs run --chip RP2040 "${TARGET}.elf"
```


#### [PyOCD](https://github.com/pyocd/pyOCD)

```bash
# python3 -m pip install pyocd
pyocd gdbserver --target rp2040_core0
pyocd rtt
gdb "${TARGET}.elf" -ex "target extended-remote localhost:3333"
```
