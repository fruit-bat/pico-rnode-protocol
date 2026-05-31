
#include "pico-rnode-protocol-frame.h"

void pico_rnode_proto_frame_init(
    pico_rnode_proto_frame_t *frame,
    pico_rnode_proto_frame_start_cb_t start_cb,
    pico_rnode_proto_frame_data_cb_t put_cb,
    pico_rnode_proto_frame_end_cb_t end_cb
) {
    frame->byte_index = 0;
    frame->start_cb = start_cb;
    frame->put_cb = put_cb;
    frame->end_cb = end_cb;
}

pico_rnode_proto_frame_cb_status_t pico_rnode_proto_frame_start(
    pico_rnode_proto_frame_t *frame,
    void *context
) {
    pico_rnode_proto_frame_cb_status_t status = PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
    frame->byte_index = 0;
    if (frame->start_cb) {
        status = frame->start_cb(context);
    }
    return status;
}

pico_rnode_proto_frame_cb_status_t pico_rnode_proto_frame_put_byte(
    pico_rnode_proto_frame_t *frame,
    void *context,
    uint8_t byte
) {
    pico_rnode_proto_frame_cb_status_t status = PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
    if (frame->put_cb) {
        status = frame->put_cb(context, byte);
    }
    frame->byte_index++;
    return status;
}

pico_rnode_proto_frame_cb_status_t pico_rnode_proto_frame_end(
    pico_rnode_proto_frame_t *frame,
    void *context
) {
    pico_rnode_proto_frame_cb_status_t status = PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
    if (frame->end_cb) {
        status = frame->end_cb(context);
    }
    return status;
}
