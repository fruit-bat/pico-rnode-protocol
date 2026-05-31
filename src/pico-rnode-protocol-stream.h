#pragma once
/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 fruit-bat
 * 
 * This module defines a simple streaming interface for transmit/receive data frames.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    PICO_RNODE_PROTO_STREAM_CB_STATUS_OK = 0,
    PICO_RNODE_PROTO_STREAM_CB_STATUS_ABORT = 1,
} pico_rnode_proto_stream_cb_status_t;

/**
 * Callback invoked when a new transmit/receive frame begins.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier for the new transmit/receive frame.
 *
 * The decoder calls this immediately after parsing a transmit start command.
 */
typedef pico_rnode_proto_stream_cb_status_t (*pico_rnode_proto_stream_start_cb_t)(
    void * context,
    uint8_t interface
);

/**
 * Callback invoked for each transmit/receive byte.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier for the current frame.
 * - byte: next byte in the current frame.
 * - byte_index: zero-based index within the current transmit/receive frame.
 *
 * Return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK to continue decoding, or
 * PICO_RNODE_PROTO_STREAM_CB_STATUS_ABORT to abort the current frame.
 */
typedef pico_rnode_proto_stream_cb_status_t (*pico_rnode_proto_stream_data_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
);

/**
 * Callback invoked when a transmit/receive frame ends.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier for the completed frame.
 * - length: number of payload bytes contained in the completed frame.
 */
typedef pico_rnode_proto_stream_cb_status_t (*pico_rnode_proto_stream_end_cb_t)(
    void * context,
    uint8_t interface,
    uint32_t length
);

/**
 * Stream context structure.
 * 
 * Pretent the contents are private...
 */
typedef struct {
    uint32_t byte_index;
    pico_rnode_proto_stream_start_cb_t start_cb;
    pico_rnode_proto_stream_data_cb_t data_cb;
    pico_rnode_proto_stream_end_cb_t end_cb;
} pico_rnode_proto_stream_t;

/**
 * Initialize a stream instance with the provided callbacks.
 * Parameters:
 * - stream: pointer to the stream instance to initialize.
 * - start_cb: callback invoked at the start of a new stream frame.
 * - data_cb: callback invoked for each byte in the stream frame.   
 * - end_cb: callback invoked when a stream frame ends.
 */
void pico_rnode_proto_stream_init(
    pico_rnode_proto_stream_t *stream,
    pico_rnode_proto_stream_start_cb_t start_cb,
    pico_rnode_proto_stream_data_cb_t data_cb,
    pico_rnode_proto_stream_end_cb_t end_cb
);

/**
 * Notify the stream that a new frame has started.
 * Parameters:
 * - stream: pointer to the stream instance.
 * - context: opaque user pointer passed to the callbacks.
 * - interface: interface identifier for the new stream frame.
 */
pico_rnode_proto_stream_cb_status_t pico_rnode_proto_stream_start(
    pico_rnode_proto_stream_t *stream,
    void *context,
    uint8_t interface
);

/**
 * Notify the stream of a new byte in the current frame.
 * Parameters:
 * - stream: pointer to the stream instance.
 * - context: opaque user pointer passed to the callbacks.
 * - interface: interface identifier for the current stream frame.
 * - byte: next byte in the current stream frame.
 */
pico_rnode_proto_stream_cb_status_t pico_rnode_proto_stream_data(
    pico_rnode_proto_stream_t *stream,
    void * context,
    uint8_t interface,
    uint8_t byte
);

/**
 * Notify the stream that the current frame has ended.
 * Parameters:
 * - stream: pointer to the stream instance.
 * - context: opaque user pointer passed to the callbacks.
 * - interface: interface identifier for the completed stream frame.
 */
pico_rnode_proto_stream_cb_status_t pico_rnode_proto_stream_end(
    pico_rnode_proto_stream_t *stream,
    void *context,
    uint8_t interface
);

#ifdef __cplusplus
}
#endif
