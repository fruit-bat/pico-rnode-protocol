// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include "pico-rnode-protocol.h"

void pico_rnode_proto_command_decoder_init(
    pico_rnode_proto_command_decoder_t *decoder,
    void * context,
    pico_rnode_proto_cmd_set_frequency_cb_t set_frequency_cb,
    pico_rnode_proto_cmd_set_bandwidth_cb_t set_bandwidth_cb,
    pico_rnode_proto_cmd_set_txpower_cb_t set_txpower_cb,
    pico_rnode_proto_data_decoder_start_cb_t tx_start_cb,
    pico_rnode_proto_data_decoder_data_cb_t tx_data_cb,
    pico_rnode_proto_data_decoder_end_cb_t tx_end_cb,
    pico_rnode_proto_data_decoder_error_cb_t tx_error_cb
) {
    decoder->context = context;
    decoder->payload_index = 0;
    decoder->opcode_length = 0;
    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_WAIT_COMMAND;

    decoder->set_frequency_cb = set_frequency_cb;
    decoder->set_bandwidth_cb = set_bandwidth_cb;
    decoder->set_txpower_cb = set_txpower_cb;

    decoder->tx_start_cb = tx_start_cb;
    decoder->tx_data_cb = tx_data_cb;
    decoder->tx_end_cb = tx_end_cb;
    decoder->tx_error_cb = tx_error_cb;
}

typedef enum {
    RNODE_OPCODE_DATA              = 0x00,
    RNODE_OPCODE_FREQUENCY         = 0x01,
    RNODE_OPCODE_BANDWIDTH         = 0x02,
    RNODE_OPCODE_TXPOWER           = 0x03,
    RNODE_OPCODE_SF                = 0x04,
    RNODE_OPCODE_CR                = 0x05,
    RNODE_OPCODE_RADIO_STATE       = 0x06,
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
        // TODO - handle other fixed-length commands
        default:
            // This should never happen since we validate opcode length in the main decoder function
            return PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE;
    }
    return PICO_RNODE_PROTO_DECODER_STATUS_OK;
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
                    // TODO - include the invalid opcode in the error callback
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
                // TODO - include the opcode and expected length in the error callback
                return PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH;
            }
            ++decoder->payload_index;
            break;
        }
        case PICO_RNODE_PROTO_DECODER_STATE_STREAM_DATA: {
            if (decoder->tx_data_cb) {
                pico_rnode_proto_data_decoder_cb_status_t cb_status = decoder->tx_data_cb(
                    decoder->context,
                    decoder->interface,
                    byte,
                    decoder->payload_index
                );
                if (cb_status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
                    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_ABORT;
                    if (decoder->tx_error_cb) {
                        decoder->tx_error_cb(
                            decoder->context, 
                            decoder->interface,
                            PICO_RNODE_PROTO_DECODER_STATUS_ABORTED,
                            decoder->payload_index
                        ); 
                    }
                    return PICO_RNODE_PROTO_DECODER_STATUS_ABORTED;
                }
            }
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

void pico_rnode_proto_command_decoder_end(
    pico_rnode_proto_command_decoder_t *decoder
) {
    switch (decoder->state) {
        case PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED: {
            if (decoder->payload_index == decoder->opcode_length) {
                // Invoke command callback with decoded fixed-length payload
                pico_rnode_proto_command_decoder_fixed_length_command(decoder);
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
        }
        default: {
            // Decoding was aborted due to an error, nothing to do
            // TODO - consider invoking an error callback here to report that the frame ended unexpectedly after an error
            break;
        }
    }
    pico_rnode_proto_command_decoder_start(decoder);
}
