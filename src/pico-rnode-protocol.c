// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include "pico-rnode-protocol.h"

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
    pico_rnode_proto_data_decoder_start_cb_t tx_start_cb,
    pico_rnode_proto_data_decoder_data_cb_t tx_data_cb,
    pico_rnode_proto_data_decoder_end_cb_t tx_end_cb,
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

    decoder->tx_start_cb = tx_start_cb;
    decoder->tx_data_cb = tx_data_cb;
    decoder->tx_end_cb = tx_end_cb;
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

typedef enum {
    RNODE_DETECT_REQ = 0x73,
    RNODE_DETECT_RESP = 0x46
} pico_rnode_proto_detect_constants_t;

typedef enum {
    RNODE_OPCODE_DATA              = 0x00,
    RNODE_OPCODE_FREQUENCY         = 0x01,
    RNODE_OPCODE_BANDWIDTH         = 0x02,
    RNODE_OPCODE_TXPOWER           = 0x03,
    RNODE_OPCODE_SF                = 0x04,
    RNODE_OPCODE_CR                = 0x05,
    RNODE_OPCODE_RADIO_STATE       = 0x06,
    RNODE_OPCODE_RADIO_LOCK        = 0x07,
    RNODE_OPCODE_DETECT            = 0x08,
    RNODE_OPCODE_LEAVE             = 0x0A,
    RNODE_OPCODE_READY             = 0x0F,
} rnode_opcode_t;

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
            return 1;
        case RNODE_OPCODE_DETECT:
        case RNODE_OPCODE_LEAVE:
        case RNODE_OPCODE_READY:
            return 0;
        case RNODE_OPCODE_RADIO_LOCK:
            return 1;
        default:
            return -2; // unknown opcode
    }
}

static const char* rnode_proto_opcode_name(rnode_opcode_t opcode) {
    switch (opcode) {
        case RNODE_OPCODE_DATA: return "DATA";
        case RNODE_OPCODE_FREQUENCY: return "FREQUENCY";
        case RNODE_OPCODE_BANDWIDTH: return "BANDWIDTH";
        case RNODE_OPCODE_TXPOWER: return "TXPOWER";
        case RNODE_OPCODE_SF: return "SF";
        case RNODE_OPCODE_CR: return "CR";
        case RNODE_OPCODE_RADIO_STATE: return "RADIO_STATE";
        case RNODE_OPCODE_DETECT: return "DETECT";
        case RNODE_OPCODE_LEAVE: return "LEAVE";
        case RNODE_OPCODE_READY: return "READY";
        default: return "UNKNOWN";
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
                decoder->lock_cb(decoder->context);
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
                    if (decoder->tx_start_cb) {
                        decoder->tx_start_cb(decoder->context, decoder->interface);
                    }
                    break;
                default:
                    decoder->opcode_length = (uint8_t)opcode_length;
                    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED;
                    break;                    
            }
            break;
        }
        case PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED: {
            if (decoder->payload_index < decoder->opcode_length) {
                decoder->smallbuf[decoder->payload_index] = byte;
            }
            else {
                // Received more bytes than expected for fixed-length command, abort and report error
                decoder->state = PICO_RNODE_PROTO_DECODER_STATE_ABORT;
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
            if (decoder->tx_data_cb) {
                pico_rnode_proto_frame_cb_status_t cb_status = decoder->tx_data_cb(
                    decoder->context,
                    decoder->interface,
                    byte,
                    decoder->payload_index
                );
                if (cb_status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
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
            if (decoder->tx_end_cb) {
                decoder->tx_end_cb(
                    decoder->context,
                    decoder->interface,
                    decoder->payload_index
                );
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

// -----------------------------------------------------------
// Encoder for outgoing protocol commands.
// -----------------------------------------------------------

void pico_rnode_proto_command_encoder_init(
    pico_rnode_proto_command_encoder_t *encoder,
    void * context,
    pico_rnode_proto_cmd_start_cb_t start_cb,
    pico_rnode_proto_cmd_put_cb_t put_cb,
    pico_rnode_proto_cmd_end_cb_t end_cb
) {
    encoder->context = context;
    encoder->state = PICO_RNODE_PROTO_ENCODER_STATE_IDLE;
    encoder->start_cb = start_cb;
    encoder->put_cb = put_cb;
    encoder->end_cb = end_cb;
}

static pico_rnode_proto_frame_cb_status_t pico_rnode_proto_command_send_byte(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t byte
) {
    if (encoder->put_cb) {
        return encoder->put_cb(encoder->context, byte);
    }
    else {
        return PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT;
    }
}

static pico_rnode_proto_frame_cb_status_t pico_rnode_proto_command_send_header(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode
) {
    return pico_rnode_proto_command_send_byte(encoder, (interface << 4) | (opcode & 0x0F));
}

// Status translation helper for command encoder functions
static pico_rnode_proto_encoder_status_t translate_frame_cb_status(
    pico_rnode_proto_frame_cb_status_t frame_status
) {
    switch (frame_status) {
        case PICO_RNODE_PROTO_FRAME_CB_STATUS_OK:
            return PICO_RNODE_PROTO_ENCODER_STATUS_OK;
        case PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT:
            return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED;
        default:
            return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED; // Treat unknown status as abort
    }
}

static pico_rnode_proto_encoder_status_t pico_rnode_proto_command_send_command_and_bytes(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode,
    uint8_t* bytes,
    size_t len // 0-4
) {
    // Check we are not currently transmitting another command
    if (encoder->state == PICO_RNODE_PROTO_ENCODER_STATE_TRANSMITTING) {
        return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR;
    }

    // Start the transmission frame
    pico_rnode_proto_encoder_status_t status = encoder->start_cb(encoder->context);

    if (status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
        return translate_frame_cb_status(status);
    }

    // Send the header
    status = pico_rnode_proto_command_send_header(encoder, interface, opcode);

    if (status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
        return translate_frame_cb_status(status);
    }

    // Send the payload
    for (size_t i = 0; i < len; i++) {
        status = pico_rnode_proto_command_send_byte(encoder, bytes[i]);
        if (status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
            return translate_frame_cb_status(status);
        }
    }

    // End the transmission frame
    status = encoder->end_cb(encoder->context);
    if (status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
        return translate_frame_cb_status(status);
    }

    return PICO_RNODE_PROTO_ENCODER_STATUS_OK;
}

static pico_rnode_proto_encoder_status_t pico_rnode_proto_command_send_command_and_word(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode,
    uint32_t word
) {
    uint8_t bytes[4];
    bytes[0] = (word >> 24) & 0xFF;
    bytes[1] = (word >> 16) & 0xFF;
    bytes[2] = (word >> 8) & 0xFF;
    bytes[3] = word & 0xFF;
    return pico_rnode_proto_command_send_command_and_bytes(encoder, interface, opcode, bytes, 4);
}

static pico_rnode_proto_encoder_status_t pico_rnode_proto_command_send_command_and_byte(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode,
    uint8_t value
) {
    return pico_rnode_proto_command_send_command_and_bytes(encoder, interface, opcode, &value, 1);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_frequency(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t hz
) {
    return pico_rnode_proto_command_send_command_and_word(encoder, interface, RNODE_OPCODE_FREQUENCY, hz);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_bandwidth(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t bandwidth
) {
    return pico_rnode_proto_command_send_command_and_word(encoder, interface, RNODE_OPCODE_BANDWIDTH, bandwidth);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_txpower(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    int8_t dbm
) {
    return pico_rnode_proto_command_send_command_and_byte(encoder, interface, RNODE_OPCODE_TXPOWER, (uint8_t)dbm);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_spreading_factor(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t sf // spreading factor, for LoRa radios (typically 6-12)
) {
    return pico_rnode_proto_command_send_command_and_byte(encoder, interface, RNODE_OPCODE_SF, sf);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_coding_rate(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t cr // coding rate, for LoRa radios (typically 5-8)
) {
    return pico_rnode_proto_command_send_command_and_byte(encoder, interface, RNODE_OPCODE_CR, cr);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_radio_state(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    pico_rnode_proto_radio_state_t state // radio state, for LoRa radios (typically 0-2)
) {
    return pico_rnode_proto_command_send_command_and_byte(encoder, interface, RNODE_OPCODE_RADIO_STATE, (uint8_t)state);
}

/**
 * Send DETECT command on interface 0 (global) to perform radio detection.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_detect(
    pico_rnode_proto_command_encoder_t *encoder
) {
    return pico_rnode_proto_command_send_command_and_byte(encoder, 0, RNODE_OPCODE_DETECT, RNODE_DETECT_REQ);
}

/**
 * Send READY command on interface 0 (global).
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_ready(
    pico_rnode_proto_command_encoder_t *encoder
) {
    return pico_rnode_proto_command_send_command_and_bytes(encoder, 0, RNODE_OPCODE_READY, NULL, 0);
}

/**
 * Send LEAVE command on interface 0 (global).
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_leave(
    pico_rnode_proto_command_encoder_t *encoder
) {
    return pico_rnode_proto_command_send_command_and_byte(encoder, 0, RNODE_OPCODE_LEAVE, 0);
}

