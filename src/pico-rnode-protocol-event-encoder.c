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
