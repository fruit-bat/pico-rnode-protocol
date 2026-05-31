
#include "pico-rnode-protocol-command-encoder.h"
#include "pico-rnode-protocol-consts.h"

#include <stdio.h>

// -----------------------------------------------------------
// Encoder for outgoing protocol commands.
// -----------------------------------------------------------

void pico_rnode_proto_command_encoder_init(
    pico_rnode_proto_command_encoder_t *encoder,
    void * context,
    pico_rnode_proto_frame_start_cb_t start_cb,
    pico_rnode_proto_frame_data_cb_t put_cb,
    pico_rnode_proto_frame_end_cb_t end_cb
) {
    pico_rnode_proto_encoder_init(&encoder->encoder, context, start_cb, put_cb, end_cb);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_frequency(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t hz
) {
    return pico_rnode_proto_encoder_send_command_and_word(&encoder->encoder, interface, RNODE_OPCODE_FREQUENCY, hz);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_bandwidth(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t bandwidth
) {
    return pico_rnode_proto_encoder_send_command_and_word(&encoder->encoder, interface, RNODE_OPCODE_BANDWIDTH, bandwidth);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_txpower(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    int8_t dbm
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_TXPOWER, (uint8_t)dbm);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_spreading_factor(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t sf // spreading factor, for LoRa radios (typically 6-12)
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_SF, sf);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_coding_rate(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t cr // coding rate, for LoRa radios (typically 5-8)
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_CR, cr);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_radio_state(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    pico_rnode_proto_radio_state_t state // radio state, for LoRa radios (typically 0-2)
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_RADIO_STATE, (uint8_t)state);
}

/**
 * Send DETECT command on interface 0 (global) to perform radio detection.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_detect(
    pico_rnode_proto_command_encoder_t *encoder
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, 0, RNODE_OPCODE_DETECT, RNODE_DETECT_REQ);
}

/**
 * Send READY command on interface 0 (global).
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_ready(
    pico_rnode_proto_command_encoder_t *encoder
) {
    return pico_rnode_proto_encoder_send_command_and_bytes(&encoder->encoder, 0, RNODE_OPCODE_READY, NULL, 0);
}

/**
 * Send LEAVE command on interface 0 (global).
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_leave(
    pico_rnode_proto_command_encoder_t *encoder
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, 0, RNODE_OPCODE_LEAVE, 0);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_start(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface
) {
    return pico_rnode_proto_encoder_start(&encoder->encoder, interface);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_data(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t byte
) {
    return pico_rnode_proto_encoder_data(&encoder->encoder, byte);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_end(
    pico_rnode_proto_command_encoder_t *encoder
) {
    return pico_rnode_proto_encoder_end(&encoder->encoder);
}
