# rnode-monitor

This app bridges a real serial device to a virtual PTY, optionally records traffic, and prints decoded RNode commands and events to stdout in a readable form.

## Build

From the monitor app directory:

```sh
cmake -S . -B build
cmake --build build -j2
```

The resulting executable is:

```sh
./build/rnode-monitor
```

## Usage

```sh
./build/rnode-monitor [<real_tty_device>] [baud_rate] [--record <file>] [--replay <file>] [--help]
```

It appears that ESP32 based RNODE announce themselves on startup...

```sh
$ cat /dev/ttyACM0 | hexdump -C
00000000  c0 55 f8 c0 45 53 50 2d  52 4f 4d 3a 65 73 70 33  |.U..ESP-ROM:esp3|
00000010  32 73 33 2d 32 30 32 31  30 33 32 37 0a 0a 42 75  |2s3-20210327..Bu|
00000020  69 6c 64 3a 4d 61 72 20  32 37 20 32 30 32 31 0a  |ild:Mar 27 2021.|
00000030  0a 72 73 74 3a 30 78 63  20 28 52 54 43 5f 53 57  |.rst:0xc (RTC_SW|
00000040  5f 43 50 55 5f 52 53 54  29 2c 62 6f 6f 74 3a 30  |_CPU_RST),boot:0|
00000050  78 38 20 28 53 50 49 5f  46 41 53 54 5f 46 4c 41  |x8 (SPI_FAST_FLA|
00000060  53 48 5f 42 4f 4f 54 29  0a 0a 53 61 76 65 64 20  |SH_BOOT)..Saved |
00000070  50 43 3a 30 78 34 32 31  30 30 39 38 65 0a 0a 53  |PC:0x4210098e..S|
00000080  50 49 57 50 3a 30 78 65  65 0a 0a 6d 6f 64 65 3a  |PIWP:0xee..mode:|
00000090  44 49 4f 2c 20 63 6c 6f  63 6b 20 64 69 76 3a 31  |DIO, clock div:1|
000000a0  0a 0a 6c 6f 61 64 3a 30  78 33 66 63 65 33 38 30  |..load:0x3fce380|
000000b0  38 2c 6c 65 6e 3a 30 78  34 62 63 0a 0a 6c 6f 61  |8,len:0x4bc..loa|
000000c0  64 3a 30 78 34 30 33 63  39 37 30 30 2c 6c 65 6e  |d:0x403c9700,len|
000000d0  3a 30 78 62 64 38 0a 0a  6c 6f 61 64 3a 30 78 34  |:0xbd8..load:0x4|
000000e0  30 33 63 63 37 30 30 2c  6c 65 6e 3a 30 78 32 61  |03cc700,len:0x2a|
000000f0  30 63 0a 0a 65 6e 74 72  79 20 30 78 34 30 33 63  |0c..entry 0x403c|
```
... which will result in a bunch of event decoding errors.


### Options

- `<real_tty_device>`: path to the physical serial device, for example `/dev/ttyUSB0`
- `baud_rate`: baud rate to use when opening the serial device, for example `115200`
- `--record <file>`: save forwarded traffic to a recording file
- `--replay <file>`: replay a previously recorded session through the decoder
- `--help`: show the built-in help text

## Examples

### Monitor a real serial device

```sh
./build/rnode-monitor /dev/ttyUSB0 115200
```

### Record traffic to a file

```sh
./build/rnode-monitor /dev/ttyUSB0 115200 --record capture.bin
```

### Replay a saved recording

```sh
./build/rnode-monitor --replay capture.bin
```

## Sample output

The app decodes KISS frames and prints readable summaries to stdout. Typical output looks like this:

```text
H->D COMMAND FREQUENCY interface=1 hz=868100000
H<-D EVENT RSSI interface=1 rssi=-72
H<-D EVENT SNR interface=1 snr=5
H->D COMMAND RADIO_STATE interface=1 state=ON
H<-D EVENT BLINK
```

## Notes

- Traffic flowing from the host toward the device is shown as commands.
- Traffic flowing from the device toward the host is shown as events.
- Decoding errors and frame-level issues are also reported to stdout/stderr so the traffic stream remains understandable.
