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
} pico_rnode_proto_command_encoder_t;

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
void pico_rnode_proto_command_encoder_init(
    pico_rnode_proto_command_encoder_t *encoder,
    void * context,
    pico_rnode_proto_cmd_start_cb_t start_cb,
    pico_rnode_proto_cmd_put_cb_t put_cb,
    pico_rnode_proto_cmd_end_cb_t end_cb
);

/**
 * Encode a set frequency command.
 *
 * Parameters:
 * - encoder: encoder instance used for output.
 * - interface: target interface identifier.
 * - hz: frequency in Hertz.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_frequency(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t hz
);

/**
 * Encode a set bandwidth command.
 *
 * Parameters:
 * - encoder: encoder instance used for output.
 * - interface: target interface identifier.
 * - bandwidth: bandwidth in Hertz.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_bandwidth(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t bandwidth
);

/**
 * Encode a set transmit power command.
 *
 * Parameters:
 * - encoder: encoder instance used for output.
 * - interface: target interface identifier.
 * - dbm: transmit power in dBm.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_txpower(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    int8_t dbm
);

/**
 * Encode a set spreading factor command.
 *
 * Parameters:
 * - encoder: encoder instance used for output.
 * - interface: target interface identifier.
 * - sf: LoRa spreading factor.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_spreading_factor(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t sf
);

/**
 * Encode a set coding rate command.
 *
 * Parameters:
 * - encoder: encoder instance used for output.
 * - interface: target interface identifier.
 * - cr: LoRa coding rate.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_coding_rate(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t cr
);

/**
 * Encode a set radio state command.
 *
 * Parameters:
 * - encoder: encoder instance used for output.
 * - interface: target interface identifier.
 * - state: radio state value.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_radio_state(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    pico_rnode_proto_radio_state_t state
);

/**
 * Encode a radio detect command.
 *
 * Parameters:
 * - encoder: encoder instance used for output.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_detect(
    pico_rnode_proto_command_encoder_t *encoder
);

/**
 * Encode a ready notification command.
 *
 * Parameters:
 * - encoder: encoder instance used for output.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_ready(
    pico_rnode_proto_command_encoder_t *encoder
);

/**
 * Encode a leave command.
 *
 * Parameters:
 * - encoder: encoder instance used for output.
 *
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_leave(
    pico_rnode_proto_command_encoder_t *encoder
);

#ifdef __cplusplus
}
#endif
