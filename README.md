# pico-rnode-protocol
RNODE protocol layer

Inspired by, but not officially part of, [Reticulum](https://github.com/markqvist/Reticulum);
thanks to [markqvist](https://github.com/markqvist) for a very interesting project.

This library is part of an attempt to split the Reticulum protocol into testable, replaceable layers, written in C.
It is intended to be suitable for use on microcontrollers, and where possible, 
avoids allocations and external dependencies. 

This library is a work in progress.

## Protocol overview

Each RNODE frame carries exactly one command or event. The first byte in a frame
encodes both the target interface and the command opcode:

- high nibble: interface identifier (0-15)
- low nibble: opcode (0-15)

Commands with fixed-length payloads include frequency, bandwidth, tx power,
spreading factor, coding rate, and radio state. A `READY` command carries no
payload, while data frames are streamed as a variable-length payload.

This library currently provides:

- `pico_rnode_proto_command_encoder_t` for building outbound RNODE commands
- `pico_rnode_proto_command_decoder_t` for parsing inbound RNODE commands
- `pico_rnode_proto_frame_t` as a small helper for frame start/byte/end callbacks

### Command opcode summary

| Opcode | Name         | Payload | Description |
| ------ | ------------ | ------- | ----------- |
| `0x00` | `DATA`       | variable | Streamed payload bytes for transmission |
| `0x01` | `FREQUENCY`  | `u32`    | Set radio frequency in Hertz |
| `0x02` | `BANDWIDTH`  | `u32`    | Set LoRa bandwidth in Hertz |
| `0x03` | `TXPOWER`    | `u8`     | Set transmit power in dBm |
| `0x04` | `SF`         | `u8`     | Set spreading factor |
| `0x05` | `CR`         | `u8`     | Set coding rate |
| `0x06` | `RADIO_STATE`| `u8`     | Set radio on/off/query state |
| `0x07` | `RADIO_LOCK` | `u8`     | Lock or unlock radio usage |
| `0x08` | `DETECT`     | `u8`     | Request radio detection |
| `0x0A` | `LEAVE`      | `u8`     | Leave the current interface |
| `0x0F` | `READY`      | none     | Notify readiness after reset |

### Simple encoder example

```c
pico_rnode_proto_command_encoder_t encoder;
pico_rnode_proto_command_encoder_init(&encoder, context, start_cb, put_cb, end_cb);
if (pico_rnode_proto_command_set_frequency(&encoder, 0, 868100000) != PICO_RNODE_PROTO_ENCODER_STATUS_OK) {
    // handle error
}
```

### Simple decoder example

```c
pico_rnode_proto_command_decoder_t decoder;
pico_rnode_proto_command_decoder_init(
    &decoder,
    context,
    detect_cb,
    set_frequency_cb,
    set_bandwidth_cb,
    set_txpower_cb,
    set_spreading_factor_cb,
    set_coding_rate_cb,
    set_radio_state_cb,
    ready_cb,
    lock_cb,
    leave_cb,
    tx_start_cb,
    tx_data_cb,
    tx_end_cb,
    error_cb
);
pico_rnode_proto_command_decoder_start(&decoder);
status = pico_rnode_proto_command_decoder_write(&decoder, frame_bytes, frame_len);
status = pico_rnode_proto_command_decoder_end(&decoder);
```

This repository is part of a suite:
* [pico-serial-proxy](https://github.com/fruit-bat/pico-serial-proxy)
* [pico-serial-recording](https://github.com/fruit-bat/pico-serial-recording)
* [pico-kiss-protocol](https://github.com/fruit-bat/pico-kiss-protocol)
* [pico-rnode-protocol](https://github.com/fruit-bat/pico-rnode-protocol)

## Host unit tests
A simple host test target is available under `tests/`.

Build and run from the repository root:

```bash
cd tests
cmake -S . -B build
cmake --build build && ctest --verbose --test-dir build
```

or for just specific tests
```bash
cd tests
cmake -S . -B build
cmake --build build && ctest --verbose --test-dir . -R host_tests_round_trip
```

This compiles `pico-rnode-protocol` as a normal host executable and runs the unit tests without Pico hardware.

## Monitor application
The monitor app can proxy a real serial device to a virtual PTY, optionally record the traffic, and replay a saved recording through the decoder path.

Build and run:

```bash
cmake -S apps/monitor -B apps/monitor/build
cmake --build apps/monitor/build --target rnode-monitor
./apps/monitor/build/rnode-monitor /dev/ttyUSB0 9600 --record capture.rec
```

### Monitor CLI options

The RNODE monitor supports:

```bash
./apps/monitor/build/rnode-monitor --help
```

- `--help` prints usage information.
- `--record <file>` writes captured traffic to a recording file.
- `--replay <file>` replays a saved recording through the RNODE decoder path.

Example:

```bash
./apps/monitor/build/rnode-monitor /dev/ttyUSB0 9600 --record capture.rec
./apps/monitor/build/rnode-monitor --replay capture.rec
```

Replay a saved recording:

```bash
./apps/monitor/build/rnode-monitor --replay capture.rec
```

Replay a saved recording with the shared replay helper:

```bash
cmake -S ../../pico-serial-recording/apps/replay -B ../../pico-serial-recording/apps/replay/build
cmake --build ../../pico-serial-recording/apps/replay/build
./pico-serial-recording/apps/replay/build/pico-serial-recording-replay capture.rec
```
