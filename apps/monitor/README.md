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
