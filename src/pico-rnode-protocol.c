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

int32_t rnode_proto_opcode_length(rnode_opcode_t opcode) {

    switch (opcode) {
        case RNODE_OPCODE_DATA:
            return -1; // variable length
        case RNODE_OPCODE_FREQUENCY:
            return 4;
        case RNODE_OPCODE_BANDWIDTH:
            return 4;
        case RNODE_OPCODE_TXPOWER:
            return 1;
        case RNODE_OPCODE_SF:
            return 1;
        case RNODE_OPCODE_CR:
            return 1;
        case RNODE_OPCODE_RADIO_STATE:
            return 1;
        case RNODE_OPCODE_DETECT:
            return 0;
        case RNODE_OPCODE_LEAVE:
            return 0;
        case RNODE_OPCODE_READY:
            return 0;
        default:
            return -2; // unknown opcode
    }
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
            decoder->opcode_length = rnode_proto_opcode_length(decoder->opcode);

            switch (decoder->opcode_length) {
                case -2:
                    // Unknown opcode, abort decoding and report error 
                    // TODO - include the invalid opcode in the error callback
                    return PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE;
                case -1:
                    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_STREAM_DATA;
                    if (decoder->tx_start_cb) {
                        decoder->tx_start_cb(decoder->context, decoder->interface);
                    }
                    break;
                default:
                    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED;
                    break;                    
            }
        }
        case PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED: {
            decoder->smallbuf[decoder->payload_index++] = byte;
            if (decoder->payload_index >= decoder->opcode_length) {
                // TODO - invoke command callback with decoded fixed-length payload
                decoder->state = PICO_RNODE_PROTO_DECODER_STATE_WAIT_COMMAND;
            }
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
                if (cb_status != PICO_RNODE_PROTO_FRAME_CB_STATUS_ERROR) {
                    decoder->state = PICO_RNODE_PROTO_DECODER_STATE_ABORT;
                    if (decoder->tx_error_cb) {
                        decoder->tx_error_cb(decoder->context, decoder->interface); 
                    }
                }
            }
        }
    }
    return PICO_RNODE_PROTO_DECODER_STATUS_OK;
}
