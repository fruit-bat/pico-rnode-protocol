#include "pico-rnode-protocol-event-encoder.h"
#include "pico-rnode-protocol-consts.h"

void pico_rnode_proto_event_encoder_init(
    pico_rnode_proto_event_encoder_t *encoder,
    void * context,
    pico_rnode_proto_frame_start_cb_t start_cb,
    pico_rnode_proto_frame_data_cb_t put_cb,
    pico_rnode_proto_frame_end_cb_t end_cb
) {
    encoder->encoder.context = context;
    encoder->encoder.state = PICO_RNODE_PROTO_ENCODER_STATE_IDLE;
    pico_rnode_proto_frame_init(&encoder->encoder.frame, start_cb, put_cb, end_cb);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_error(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t error_code
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_DATA, error_code);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_stat_rx(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
) {
    return pico_rnode_proto_encoder_send_command_and_bytes(&encoder->encoder, interface, RNODE_OPCODE_STAT_RX, payload, len);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_stat_tx(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
) {
    return pico_rnode_proto_encoder_send_command_and_bytes(&encoder->encoder, interface, RNODE_OPCODE_STAT_TX, payload, len);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_rssi(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    int8_t rssi
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_STAT_RSSI, (uint8_t)rssi);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_snr(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    int8_t snr
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_STAT_SNR, (uint8_t)snr);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_blink(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface
) {
    return pico_rnode_proto_encoder_send_command_and_bytes(&encoder->encoder, interface, RNODE_OPCODE_BLINK, NULL, 0);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_random(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t random_value
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_RANDOM, random_value);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_platform(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t platform_id
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_PLATFORM, platform_id);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_mcu(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t mcu_id
) {
    return pico_rnode_proto_encoder_send_command_and_byte(&encoder->encoder, interface, RNODE_OPCODE_MCU, mcu_id);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_fw_version(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint16_t version
) {
    uint8_t bytes[2];
    bytes[0] = (version >> 8) & 0xFF;
    bytes[1] = version & 0xFF;
    return pico_rnode_proto_encoder_send_command_and_bytes(&encoder->encoder, interface, RNODE_OPCODE_FW_VERSION, bytes, 2);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_rom_read(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
) {
    return pico_rnode_proto_encoder_send_command_and_bytes(&encoder->encoder, interface, RNODE_OPCODE_ROM_READ, payload, len);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_reset(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
) {
    return pico_rnode_proto_encoder_send_command_and_bytes(&encoder->encoder, interface, RNODE_OPCODE_RESET, payload, len);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_interfaces(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
) {
    return pico_rnode_proto_encoder_send_command_and_bytes(&encoder->encoder, interface, RNODE_OPCODE_INTERFACES, payload, len);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_start(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface
) {
    return pico_rnode_proto_encoder_start(&encoder->encoder, interface);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_data(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t byte
) {
    return pico_rnode_proto_encoder_data(&encoder->encoder, byte);
}

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_end(
    pico_rnode_proto_event_encoder_t *encoder
) {
    return pico_rnode_proto_encoder_end(&encoder->encoder);
}
