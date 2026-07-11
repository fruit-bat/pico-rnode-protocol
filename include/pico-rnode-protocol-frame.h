#pragma once
/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 fruit-bat
 * 
 * This module defines a simple framing interface for encoding protocol commands
 * into byte streams suitable for transport over KISS or similar links.
 *
 * A single RNODE frame contains exactly one command or event. The first byte
 * of each frame encodes the interface in the upper nibble and the command opcode
 * in the lower nibble.
 *
 * Higher-level code can use the frame helpers to start a new frame, emit bytes
 * one at a time, and finalize the frame when complete.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    PICO_RNODE_PROTO_FRAME_CB_STATUS_OK = 0,
    PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT = 1,
} pico_rnode_proto_frame_cb_status_t;

/**
 * Callback invoked to begin a new command frame.
 *
 * @param context user-provided opaque context pointer.
 *
 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK to continue encoding.

 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT to abort the command.
 */
typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_frame_start_cb_t)(
    void * context
);

/**
 * Callback invoked to emit a single protocol byte.
 *
 * @param context user-provided opaque context pointer.

 * @param byte byte to emit into the current frame.
 *
 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK to continue encoding.

 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT to abort the command.
 */
typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_frame_data_cb_t)(
    void * context,
    uint8_t byte
);

/**
 * Callback invoked to end the current command frame.
 *
 * @param context user-provided opaque context pointer.
 *
 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK to finalize the frame.

 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT to abort the frame.
 */
typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_frame_end_cb_t)(
    void * context
);

/**
 * Frame context structure.
 * 
 * Pretent the contents are private...
 */
typedef struct {
    uint32_t byte_index; /**< Zero-based index of the next byte in the current frame. */
    pico_rnode_proto_frame_start_cb_t start_cb; /**< Callback invoked at the start of a new frame. */
    pico_rnode_proto_frame_data_cb_t put_cb;    /**< Callback invoked for each emitted byte. */
    pico_rnode_proto_frame_end_cb_t end_cb;     /**< Callback invoked at the end of the frame. */
} pico_rnode_proto_frame_t;

/**
 * Initialize a frame context with the provided callbacks.
 *
 * @param frame pointer to the frame context to initialize.

 * @param start_cb callback invoked at the start of a new frame.

 * @param put_cb callback invoked for each byte emitted into the frame.

 * @param end_cb callback invoked at the end of the frame.
 *
 * @return None.
 */
void pico_rnode_proto_frame_init(
    pico_rnode_proto_frame_t *frame,
    pico_rnode_proto_frame_start_cb_t start_cb,
    pico_rnode_proto_frame_data_cb_t put_cb,
    pico_rnode_proto_frame_end_cb_t end_cb
);

/**
 * Start a new frame by resetting the byte index and invoking the start callback.
 *
 * @param frame pointer to the frame context.

 * @param context opaque user pointer passed to the callback.
 *
 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK if the frame start succeeded.

 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT if the start callback requests abort.
 */
pico_rnode_proto_frame_cb_status_t pico_rnode_proto_frame_start(
    pico_rnode_proto_frame_t *frame,
    void *context
);

/**
 * Emit a byte into the current frame and advance the byte index.
 *
 * @param frame pointer to the frame context.

 * @param context opaque user pointer passed to the callback.

 * @param byte byte to emit into the current frame.
 *
 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK if the byte was accepted.

 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT if the put callback requests abort.
 */
pico_rnode_proto_frame_cb_status_t pico_rnode_proto_frame_put_byte(
    pico_rnode_proto_frame_t *frame,
    void *context,
    uint8_t byte
);

/**
 * End the current frame by invoking the end callback.
 *
 * @param frame pointer to the frame context.

 * @param context opaque user pointer passed to the callback.
 *
 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK if the frame end succeeded.

 * @return PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT if the end callback requests abort.
 */
pico_rnode_proto_frame_cb_status_t pico_rnode_proto_frame_end(
    pico_rnode_proto_frame_t *frame,
    void *context
);

#ifdef __cplusplus
}
#endif
