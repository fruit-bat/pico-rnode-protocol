
#include "pico-rnode-protocol-frame.h"

void init_pico_rnode_proto_frame(
    pico_rnode_proto_frame_t *frame,
    pico_rnode_proto_cmd_start_cb_t start_cb,
    pico_rnode_proto_cmd_put_cb_t put_cb,
    pico_rnode_proto_cmd_end_cb_t end_cb
) {
    frame->byte_index = 0;
    frame->start_cb = start_cb;
    frame->put_cb = put_cb;
    frame->end_cb = end_cb;
}

void pico_rnode_proto_frame_start(
    pico_rnode_proto_frame_t *frame,
    void *context
) {
    frame->byte_index = 0;
    if (frame->start_cb) {
        frame->start_cb(context);
    }
}

void pico_rnode_proto_frame_put_byte(
    pico_rnode_proto_frame_t *frame,
    void *context,
    uint8_t byte
) {
    if (frame->put_cb) {
        frame->put_cb(context, byte);
    }
    frame->byte_index++;
}

void pico_rnode_proto_frame_end(
    pico_rnode_proto_frame_t *frame,
    void *context
) {
    if (frame->end_cb) {
        frame->end_cb(context);
    }
}

