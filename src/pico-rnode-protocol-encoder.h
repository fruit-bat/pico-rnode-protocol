// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "pico-rnode-protocol-consts.h"
#include "pico-rnode-protocol-frame.h"

// -----------------------------------------------------------
// Encoder for outgoing protocol commands (e.g. from host to radio).
// -----------------------------------------------------------

/**
 * Encoder status codes.
 *
 * These values indicate whether a command was encoded successfully or whether
 * the encoder encountered an error such as an in-progress frame conflict.
 */
typedef enum {
    PICO_RNODE_PROTO_ENCODER_STATUS_OK = 0,    /**< Command encoded successfully. */
    PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED,   /**< Encoding aborted by callback or internal error. */
    PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR, /**< A frame is already open when a new command was requested. */
} pico_rnode_proto_encoder_status_t;

/**
 * Encoder state machine state values.
 */
typedef enum {
    PICO_RNODE_PROTO_ENCODER_STATE_IDLE = 0,        /**< Encoder is ready for a new command. */
    PICO_RNODE_PROTO_ENCODER_STATE_TRANSMITTING,    /**< A command frame is currently being emitted. */
} pico_rnode_proto_encoder_state_t;

/**
 * Outgoing command encoder instance.
 *
 * This struct holds encoder state and the callbacks required to write bytes
 * into the transport layer.
 */
typedef struct {
    void * context;                         /**< User-provided callback context. */
    pico_rnode_proto_encoder_state_t state; /**< Current encoder state. */
    /**
     * Frame helper used to emit a complete command frame.
     *
     * The encoder delegates frame start/byte/end operations to this helper
     * which wraps the user-provided callbacks.
     */
    pico_rnode_proto_frame_t frame;
} pico_rnode_proto_encoder_t;

/**
 * Initialize a command encoder instance.
 *
 * Parameters:
 * - encoder: encoder instance to initialize.
 * - context: opaque user context passed to callbacks.
 * - start_cb: callback invoked at frame start.
 * - put_cb: callback invoked for each emitted byte.
 * - end_cb: callback invoked at frame end.
 *
 * Returns:
 * - None.
 */
void pico_rnode_proto_encoder_init(
    pico_rnode_proto_encoder_t *encoder,
    void * context,
    pico_rnode_proto_frame_start_cb_t start_cb,
    pico_rnode_proto_frame_data_cb_t put_cb,
    pico_rnode_proto_frame_end_cb_t end_cb
);

/**
 * Start a new encoder frame by invoking the configured start callback.
 *
 * Parameters:
 * - encoder: encoder instance handling output.
 * - interface: target interface identifier to send transmission to.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when the start callback succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when the start callback requests abort.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_start(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t interface
);

/**
 * Emit a single byte for the currently-open encoder frame.
 *
 * Parameters:
 * - encoder: encoder instance handling output.
 * - byte: byte to emit.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when the byte was accepted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when the put callback requests abort.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_data(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t byte
);

/**
 * End the current encoder frame by invoking the configured end callback.
 *
 * Parameters:
 * - encoder: encoder instance handling output.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when the end callback succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when the end callback requests abort.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_end(
    pico_rnode_proto_encoder_t *encoder
);

/**
 * Encode a complete command frame containing a raw payload byte array.
 *
 * Parameters:
 * - encoder: encoder instance handling output.
 * - interface: target interface identifier.
 * - opcode: command opcode to encode.
 * - bytes: payload bytes to include after the header.
 * - len: number of payload bytes.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when a callback aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_send_command_and_bytes(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode,
    uint8_t* bytes,
    size_t len
);

/**
 * Encode a complete command frame with a 32-bit big-endian payload.
 *
 * Parameters:
 * - encoder: encoder instance handling output.
 * - interface: target interface identifier.
 * - opcode: command opcode to encode.
 * - word: 32-bit word payload.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when a callback aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_send_command_and_word(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode,
    uint32_t word
);

/**
 * Encode a complete command frame with a single-byte payload.
 *
 * Parameters:
 * - encoder: encoder instance handling output.
 * - interface: target interface identifier.
 * - opcode: command opcode to encode.
 * - value: payload byte.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when a callback aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_encoder_send_command_and_byte(
    pico_rnode_proto_encoder_t *encoder,
    uint8_t interface,
    rnode_opcode_t opcode,
    uint8_t value
);

#ifdef __cplusplus
}
#endif
