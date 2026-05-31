#include "pico-rnode-protocol-encoder.h"
#include "pico-rnode-protocol-consts.h"

#include <stdio.h>

// -----------------------------------------------------------
// Encoder for outgoing protocol commands.
// -----------------------------------------------------------

void pico_rnode_proto_encoder_init(
    pico_rnode_proto_encoder_t *encoder,
    void * context,
    pico_rnode_proto_frame_start_cb_t start_cb,
    pico_rnode_proto_frame_data_cb_t put_cb,
    pico_rnode_proto_frame_end_cb_t end_cb
) {
    encoder->context = context;
    encoder->state = PICO_RNODE_PROTO_ENCODER_STATE_IDLE;
    pico_rnode_proto_frame_init(&encoder->frame, start_cb, put_cb, end_cb);
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

pico_rnode_proto_frame_cb_status_t pico_rnode_proto_encoder_send_byte(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t byte
) {
    return pico_rnode_proto_frame_put_byte(&encoder->frame, encoder->context, byte);
}

pico_rnode_proto_frame_cb_status_t pico_rnode_proto_encoder_send_header(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode
) {
    return pico_rnode_proto_encoder_send_byte(encoder, (interface << 4) | (opcode & 0x0F));
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_start(
    pico_rnode_proto_encoder_t *encoder
) {
    // Check we are not currently transmitting another command
    if (encoder->state == PICO_RNODE_PROTO_ENCODER_STATE_TRANSMITTING) {
        return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR;
    }
    encoder->state = PICO_RNODE_PROTO_ENCODER_STATE_TRANSMITTING;
    return translate_frame_cb_status(pico_rnode_proto_frame_start(&encoder->frame, encoder->context));
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_data(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t byte
) {
    // Check we are currently transmitting a command
    if (encoder->state != PICO_RNODE_PROTO_ENCODER_STATE_TRANSMITTING) {
        return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR;
    }
    return translate_frame_cb_status(pico_rnode_proto_frame_put_byte(&encoder->frame, encoder->context, byte));
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_end(
    pico_rnode_proto_encoder_t *encoder
) {
    // Check we are currently transmitting a command
    if (encoder->state != PICO_RNODE_PROTO_ENCODER_STATE_TRANSMITTING) {
        return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR;
    }
    encoder->state = PICO_RNODE_PROTO_ENCODER_STATE_IDLE;
    return translate_frame_cb_status(pico_rnode_proto_frame_end(&encoder->frame, encoder->context));
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_send_command_and_bytes(
    pico_rnode_proto_encoder_t *encoder,
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
    pico_rnode_proto_frame_cb_status_t frame_status = pico_rnode_proto_frame_start(
        &encoder->frame, 
        encoder->context
    );
    
    if (frame_status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
        return translate_frame_cb_status(frame_status);
    }

    encoder->state = PICO_RNODE_PROTO_ENCODER_STATE_TRANSMITTING;

    // Send the header
    frame_status = pico_rnode_proto_encoder_send_header(encoder, interface, opcode);
    if (frame_status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
        encoder->state = PICO_RNODE_PROTO_ENCODER_STATE_IDLE;
        return translate_frame_cb_status(frame_status);
    }

    // Send the payload
    for (size_t i = 0; i < len; i++) {
        frame_status = pico_rnode_proto_encoder_send_byte(encoder, bytes[i]);
        if (frame_status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
            encoder->state = PICO_RNODE_PROTO_ENCODER_STATE_IDLE;
            return translate_frame_cb_status(frame_status);
        }
    }

    // End the transmission frame
    frame_status = pico_rnode_proto_frame_end(&encoder->frame, encoder->context);
    encoder->state = PICO_RNODE_PROTO_ENCODER_STATE_IDLE;
    if (frame_status != PICO_RNODE_PROTO_FRAME_CB_STATUS_OK) {
        return translate_frame_cb_status(frame_status);
    }

    return PICO_RNODE_PROTO_ENCODER_STATUS_OK;
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_send_command_and_word(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode,
    uint32_t word
) {
    uint8_t bytes[4];
    bytes[0] = (word >> 24) & 0xFF;
    bytes[1] = (word >> 16) & 0xFF;
    bytes[2] = (word >> 8) & 0xFF;
    bytes[3] = word & 0xFF;
    return pico_rnode_proto_encoder_send_command_and_bytes(encoder, interface, opcode, bytes, 4);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_send_command_and_byte(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode,
    uint8_t value
) {
    return pico_rnode_proto_encoder_send_command_and_bytes(encoder, interface, opcode, &value, 1);
}
