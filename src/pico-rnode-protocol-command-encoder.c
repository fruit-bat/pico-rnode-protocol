
#include "pico-rnode-protocol-command-encoder.h"
#include "pico-rnode-protocol-consts.h"

#include <stdio.h>

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
