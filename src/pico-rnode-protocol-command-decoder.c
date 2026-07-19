// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include "pico-rnode-protocol-command-decoder.h"

#include <stdio.h>
// -----------------------------------------------------------
// Decoder for incoming protocol commands.
// -----------------------------------------------------------

void pico_rnode_proto_command_decoder_init(
    pico_rnode_proto_command_decoder_t *decoder,
    void * context,
    pico_rnode_proto_command_detect_cb_t detect_cb,
    pico_rnode_proto_command_set_frequency_cb_t set_frequency_cb,
    pico_rnode_proto_command_set_bandwidth_cb_t set_bandwidth_cb,
    pico_rnode_proto_command_set_txpower_cb_t set_txpower_cb,
    pico_rnode_proto_command_set_spreading_factor_cb_t set_spreading_factor_cb,
    pico_rnode_proto_command_set_coding_rate_cb_t set_coding_rate_cb,
    pico_rnode_proto_command_set_radio_state_cb_t set_radio_state_cb,
    pico_rnode_proto_command_ready_cb_t ready_cb,
    pico_rnode_proto_command_lock_cb_t lock_cb,
    pico_rnode_proto_command_leave_cb_t leave_cb,
    pico_rnode_proto_stream_start_cb_t tx_start_cb,
    pico_rnode_proto_stream_data_cb_t tx_data_cb,
    pico_rnode_proto_stream_end_cb_t tx_end_cb,
    pico_rnode_proto_decoder_error_cb_t error_cb
) {
    decoder->context = context;
    decoder->payload_index = 0;
    decoder->opcode_length = 0;
    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_WAIT_COMMAND;

    decoder->detect_cb = detect_cb;
    decoder->set_frequency_cb = set_frequency_cb;
    decoder->set_bandwidth_cb = set_bandwidth_cb;
    decoder->set_txpower_cb = set_txpower_cb;
    decoder->set_spreading_factor_cb = set_spreading_factor_cb;
    decoder->set_coding_rate_cb = set_coding_rate_cb;
    decoder->set_radio_state_cb = set_radio_state_cb;
    decoder->ready_cb = ready_cb;
    decoder->lock_cb = lock_cb;
    decoder->leave_cb = leave_cb;

    pico_rnode_proto_stream_init(&decoder->tx_stream, tx_start_cb, tx_data_cb, tx_end_cb);
    decoder->error_cb = error_cb;
}

//
// Core radio configuration commands
// These are the fundamental operational commands.
//
// RNODE commands
// | Opcode | Name        | Payload  |
// | ------ | ----------- | -------- |
// | `0x00` | DATA        | variable | (up to 255 bytes, sent in a stream of multiple frames if needed)
// | `0x01` | FREQUENCY   | `u32`    | (in Hz, e.g. 868100000 for 868.1 MHz center frequency)
// | `0x02` | BANDWIDTH   | `u32`    | (in Hz, e.g. 125000 for 125 kHz LoRa bandwidth)
// | `0x03` | TXPOWER     | `u8`     | (signed dBm value)
// | `0x04` | SF          | `u8`     | (spreading factor, for LoRa radios)
// | `0x05` | CR          | `u8`     | (coding rate, for LoRa radios)
// | `0x06` | RADIO_STATE | `u8`     | (0 = off, 1 = on, 255 = query state)
// | `0x07` | RADIO_LOCK  | `u8`     | (0 = unlock, 1 = lock the radio for exclusive use by the current interface)
// | `0x08` | DETECT      | `u8`     | (interface to detect on)
// | `0x0A` | LEAVE       | `u8`     | (interface to leave)
// | `0x0F` | READY       | empty    | (sent by device when ready to receive commands after power-on or reset)
//
//
// Status / telemetry events
// These are mostly device → host.
//
// | Opcode | Name      | Payload             |
// | ------ | --------- | ------------------- |
// | `0x21` | STAT_RX   | varies              |
// | `0x22` | STAT_TX   | varies              |
// | `0x23` | STAT_RSSI | `i8`                | 
// | `0x24` | STAT_SNR  | `i8` scaled by 0.25 |
//
//
// Device info / management
//
// | Opcode | Name       | Payload    |
// | ------ | ---------- | ---------- |
// | `0x30` | BLINK      | none/small |
// | `0x40` | RANDOM     | `u8`       |
// | `0x48` | PLATFORM   | `u8`       |
// | `0x49` | MCU        | `u8`       |
// | `0x50` | FW_VERSION | 2 bytes    |
// | `0x51` | ROM_READ   | varies     |
// | `0x55` | RESET      | optional   |
// | `0x71` | INTERFACES | varies     |
//
//
// protocol constants
// Detect handshake
//
// | Constant    | Value  |
// | ----------- | ------ |
// | DETECT_REQ  | `0x73` |
// | DETECT_RESP | `0x46` |
//
//
// Radio state constants
//
// | Constant        | Value  |
// | --------------- | ------ |
// | RADIO_STATE_OFF | `0x00` |
// | RADIO_STATE_ON  | `0x01` |
// | RADIO_STATE_ASK | `0xFF` |
//
//
// Error protocol
//
// There is also a distinct ERROR opcode:
//
// | Opcode | Name  |
// | ------ | ----- |
// | `0x90` | ERROR |
//
// with payload values like:
//
// | Error         | Value  |
// | ------------- | ------ |
// | INITRADIO     | `0x01` |
// | TXFAILED      | `0x02` |
// | EEPROM_LOCKED | `0x03` |
//
//

static int32_t rnode_proto_opcode_length(rnode_opcode_t opcode) {

    switch (opcode) {
        case RNODE_OPCODE_DATA:
            return -1; // variable length
        case RNODE_OPCODE_FREQUENCY:
        case RNODE_OPCODE_BANDWIDTH:
            return 4;
        case RNODE_OPCODE_TXPOWER:
        case RNODE_OPCODE_SF:
        case RNODE_OPCODE_CR:
        case RNODE_OPCODE_RADIO_STATE:
        case RNODE_OPCODE_DETECT:
        case RNODE_OPCODE_LEAVE:
        case RNODE_OPCODE_RADIO_LOCK:
            return 1;
        case RNODE_OPCODE_READY:
            return 0;
        default:
            return -2; // unknown opcode
    }
}

static pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_fixed_length_command(
    pico_rnode_proto_command_decoder_t *decoder
) {
    pico_rnode_proto_decoder_status_t status = PICO_RNODE_PROTO_DECODER_STATUS_OK;
    uint32_t value = 0;
    for (uint32_t i = 0; i < decoder->opcode_length; i++) {
        value <<= 8;
        value |= ((uint32_t)decoder->smallbuf[i]);
    }

    switch (decoder->opcode) {
        case RNODE_OPCODE_FREQUENCY:
            if (decoder->set_frequency_cb) {
                decoder->set_frequency_cb(decoder->context, decoder->interface, value);
            }
            break;
        case RNODE_OPCODE_BANDWIDTH:
            if (decoder->set_bandwidth_cb) {
                decoder->set_bandwidth_cb(decoder->context, decoder->interface, value);
            }
            break;
        case RNODE_OPCODE_TXPOWER:
            if (decoder->set_txpower_cb) {
                decoder->set_txpower_cb(decoder->context, decoder->interface, (int8_t)value);
            }
            break;
        case RNODE_OPCODE_SF:
            if (decoder->set_spreading_factor_cb) {
                decoder->set_spreading_factor_cb(decoder->context, decoder->interface, (uint8_t)value);
            }
            break;
        case RNODE_OPCODE_CR:
            if (decoder->set_coding_rate_cb) {
                decoder->set_coding_rate_cb(decoder->context, decoder->interface, (uint8_t)value);
            }
            break;
        case RNODE_OPCODE_RADIO_STATE:
            switch (value) {
                case RNODE_RADIO_STATE_OFF:
                case RNODE_RADIO_STATE_ON:
                case RNODE_RADIO_STATE_ASK:
                    if (decoder->set_radio_state_cb) {
                        decoder->set_radio_state_cb(
                            decoder->context, 
                            decoder->interface, 
                            (pico_rnode_proto_radio_state_t)value
                        );
                    }
                    break;
                default:
                    status = PICO_RNODE_PROTO_DECODER_STATUS_INVALID_ARGUMENT;
                    break;
            }
            break;
        case RNODE_OPCODE_DETECT:
            switch (value) {
                case RNODE_DETECT_REQ:
                    if (decoder->detect_cb) {
                        decoder->detect_cb(decoder->context);
                    }
                    break;
                default:
                    status = PICO_RNODE_PROTO_DECODER_STATUS_INVALID_ARGUMENT;
                    break;
            }
            break;
        case RNODE_OPCODE_LEAVE:
            if (decoder->leave_cb) {
                decoder->leave_cb(decoder->context);
            }
            break;
        case RNODE_OPCODE_READY:
            if (decoder->ready_cb) {
                decoder->ready_cb(decoder->context);
            }
            break;
        case RNODE_OPCODE_RADIO_LOCK:
            if (decoder->lock_cb) {
                decoder->lock_cb(decoder->context, decoder->interface, (uint8_t)value);
            }
            break;
        default:
            status = PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE;
            break;
    }
    if (status != PICO_RNODE_PROTO_DECODER_STATUS_OK && decoder->error_cb) {
        decoder->error_cb(
            decoder->context, 
            decoder->interface,
            decoder->opcode,
            decoder->payload_index,
            status
        );
    }
    return status;
}

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_put(
    pico_rnode_proto_command_decoder_t *decoder,
    uint8_t byte
) {
    switch (decoder->state) {
        case PICO_RNODE_PROTO_DECODER_STATE_WAIT_COMMAND: {
            decoder->interface = (byte >> 4) & 0x0F;
            decoder->opcode = byte & 0x0F;
            decoder->payload_index = 0;
            int32_t opcode_length = rnode_proto_opcode_length(decoder->opcode);

            switch (opcode_length) {
                case -2:
                    // Unknown opcode, abort decoding and report error 
                    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_ABORT;
                    // Include the invalid opcode in the error callback
                    if (decoder->error_cb) {
                        decoder->error_cb(
                            decoder->context, 
                            decoder->interface,
                            decoder->opcode,
                            decoder->payload_index,
                            PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE
                        );
                    }
                    return PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE;
                case -1:
                    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_STREAM_DATA;
                    pico_rnode_proto_stream_start(
                        &decoder->tx_stream,
                        decoder->context,
                        decoder->interface
                    );
                    break;
                default:
                    decoder->opcode_length = (uint8_t)opcode_length;
                    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED;
                    break;                    
            }
            break;
        }
        case PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED: {
            if (decoder->payload_index < 4 /* decoder->opcode_length */) {
                decoder->smallbuf[decoder->payload_index] = byte;
            }
            else {
                // Received more bytes than expected for fixed-length command, abort and report error
                decoder->state = PICO_RNODE_PROTO_DECODER_STATE_ABORT;
                printf("-------------- too many -- expected %u bytes\n", decoder->opcode_length);

                if (decoder->error_cb) {
                    decoder->error_cb(
                        decoder->context, 
                        decoder->interface,
                        decoder->opcode,
                        decoder->payload_index,
                        PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH
                    );
                }
                return PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH;
            }
            ++decoder->payload_index;
            break;
        }
        case PICO_RNODE_PROTO_DECODER_STATE_STREAM_DATA: {
            pico_rnode_proto_stream_cb_status_t cb_status = pico_rnode_proto_stream_data(
                &decoder->tx_stream,
                decoder->context,
                decoder->interface,
                byte
            );
            if (cb_status != PICO_RNODE_PROTO_STREAM_CB_STATUS_OK) {
                decoder->state = PICO_RNODE_PROTO_DECODER_STATE_ABORT;
                if (decoder->error_cb) {
                    decoder->error_cb(
                        decoder->context, 
                        decoder->interface,
                        decoder->opcode,
                        decoder->payload_index,
                        PICO_RNODE_PROTO_DECODER_STATUS_ABORTED
                    );
                }
                return PICO_RNODE_PROTO_DECODER_STATUS_ABORTED;
            }
            decoder->payload_index++;
            break;
        }
    }
    return PICO_RNODE_PROTO_DECODER_STATUS_OK;
}

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_write(
    pico_rnode_proto_command_decoder_t *decoder,
    const uint8_t* bytes,
    size_t len
) {
    for (size_t i = 0; i < len; i++) {
        pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_put(
            decoder,
            bytes[i]
        );
        if (status != PICO_RNODE_PROTO_DECODER_STATUS_OK) {
            return status;
        }
    }
    return PICO_RNODE_PROTO_DECODER_STATUS_OK;
}

void pico_rnode_proto_command_decoder_start(
    pico_rnode_proto_command_decoder_t *decoder
) {
    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_WAIT_COMMAND;
    decoder->payload_index = 0;
}

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_end(
    pico_rnode_proto_command_decoder_t *decoder
) {
    pico_rnode_proto_decoder_status_t status = PICO_RNODE_PROTO_DECODER_STATUS_OK;
    switch (decoder->state) {
        case PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED: {
            if (decoder->payload_index == decoder->opcode_length) {
                // Invoke command callback with decoded fixed-length payload
                pico_rnode_proto_command_decoder_fixed_length_command(decoder);
            }
            else {
                // Received fewer bytes than expected for fixed-length command, abort and report error
                status = PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH;
                printf("-------------- fewer -- expected %u bytes\n", decoder->opcode_length);
                if (decoder->error_cb) {
                    decoder->error_cb(
                        decoder->context, 
                        decoder->interface,
                        decoder->opcode,
                        decoder->payload_index,
                        PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH
                    ); 
                }
            }
            break;
        }
        case PICO_RNODE_PROTO_DECODER_STATE_STREAM_DATA: {
            pico_rnode_proto_stream_cb_status_t cb_status = pico_rnode_proto_stream_end(
                &decoder->tx_stream,
                decoder->context,
                decoder->interface
            );
            if (cb_status != PICO_RNODE_PROTO_STREAM_CB_STATUS_OK) {
                decoder->state = PICO_RNODE_PROTO_DECODER_STATE_ABORT;
                if (decoder->error_cb) {
                    decoder->error_cb(
                        decoder->context, 
                        decoder->interface,
                        decoder->opcode,
                        decoder->payload_index,
                        PICO_RNODE_PROTO_DECODER_STATUS_ABORTED
                    );
                }
                return PICO_RNODE_PROTO_DECODER_STATUS_ABORTED;
            }
            break;
        }
        default: {
            status = PICO_RNODE_PROTO_DECODER_STATUS_ABORTED;
            break;
        }
    }
    pico_rnode_proto_command_decoder_start(decoder);

    return status;
}

