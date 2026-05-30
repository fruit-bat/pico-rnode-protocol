#include "pico-rnode-protocol-stream.h"

void pico_rnode_proto_stream_init(
    pico_rnode_proto_stream_t *stream,
    pico_rnode_proto_stream_start_cb_t start_cb,
    pico_rnode_proto_stream_data_cb_t data_cb,
    pico_rnode_proto_stream_end_cb_t end_cb
) {
    stream->byte_index = 0;
    stream->start_cb = start_cb;
    stream->data_cb = data_cb;
    stream->end_cb = end_cb;
}

void pico_rnode_proto_stream_start(
    pico_rnode_proto_stream_t *stream,
    void *context,
    uint8_t interface
) {
    stream->byte_index = 0;
    if (stream->start_cb) {
        stream->start_cb(context, interface);
    }
}

void pico_rnode_proto_stream_put_byte(
    pico_rnode_proto_stream_t *stream,
    void * context,
    uint8_t interface,
    uint8_t byte
) {
    if (stream->data_cb) {
        stream->data_cb(context, interface, byte, stream->byte_index);
    }
    stream->byte_index++;
}

void pico_rnode_proto_stream_end(
    pico_rnode_proto_stream_t *stream,
    void *context,
    uint8_t interface
) {
    if (stream->end_cb) {
        stream->end_cb(context, interface, stream->byte_index);
    }
}

