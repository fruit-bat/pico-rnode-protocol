// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "pico-rnode-protocol-consts.h"
#include "pico-rnode-protocol-encoder.h"

// -----------------------------------------------------------
// Encoder for outgoing protocol commands (e.g. from host to radio).
// -----------------------------------------------------------

/**
 * Command encoder is a wrapper around the protocol encoder.
 *
 * The command encoder provides command-specific encoding functions that
 * use the underlying protocol encoder for frame management.
 */

/**
 * Outgoing command encoder instance.
 *
 * This struct wraps a protocol encoder to provide command-specific encoding
 * functions. The protocol encoder handles all frame start/byte/end operations.
 * 
 */
typedef struct {
    /**
     * Internal protocol encoder used to emit command frames.
     *
     * The command encoder delegates all frame operations to this helper.
     */
    pico_rnode_proto_encoder_t encoder;
} pico_rnode_proto_command_encoder_t;

/**
 * Initialize a command encoder instance.
 *
 * @param encoder encoder instance to initialize.

 * @param context opaque user context passed to callbacks.

 * @param start_cb callback invoked at frame start.

 * @param put_cb callback invoked for each emitted byte.

 * @param end_cb callback invoked at frame end.
 *
 * @return None.
 */
void pico_rnode_proto_command_encoder_init(
    pico_rnode_proto_command_encoder_t *encoder,
    void * context,
    pico_rnode_proto_frame_start_cb_t start_cb,
    pico_rnode_proto_frame_data_cb_t put_cb,
    pico_rnode_proto_frame_end_cb_t end_cb
);

/**
 * Encode a set frequency command.
 *
 * @param encoder encoder instance used for output.

 * @param interface target interface identifier.

 * @param hz frequency in Hertz.
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_frequency(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t hz
);

/**
 * Encode a set bandwidth command.
 *
 * @param encoder encoder instance used for output.

 * @param interface target interface identifier.

 * @param bandwidth bandwidth in Hertz.
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_bandwidth(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t bandwidth
);

/**
 * Encode a set transmit power command.
 *
 * @param encoder encoder instance used for output.

 * @param interface target interface identifier.

 * @param dbm transmit power in dBm.
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_txpower(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    int8_t dbm
);

/**
 * Encode a set spreading factor command.
 *
 * @param encoder encoder instance used for output.

 * @param interface target interface identifier.

 * @param sf LoRa spreading factor.
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_spreading_factor(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t sf
);

/**
 * Encode a set coding rate command.
 *
 * @param encoder encoder instance used for output.

 * @param interface target interface identifier.

 * @param cr LoRa coding rate.
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_coding_rate(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t cr
);

/**
 * Encode a set radio state command.
 *
 * @param encoder encoder instance used for output.

 * @param interface target interface identifier.

 * @param state radio state value.
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_radio_state(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    pico_rnode_proto_radio_state_t state
);

/**
 * Encode a radio lock command.
 *
 * @param encoder encoder instance used for output.

 * @param interface target interface identifier.

 * @param lock_state lock state value (0 = unlock, 1 = lock).
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_radio_lock(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t lock_state
);

/**
 * Encode a radio detect command.
 *
 * @param encoder encoder instance used for output.
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_detect(
    pico_rnode_proto_command_encoder_t *encoder
);

/**
 * Encode a ready notification command.
 *
 * @param encoder encoder instance used for output.
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_ready(
    pico_rnode_proto_command_encoder_t *encoder
);

/**
 * Encode a leave command.
 *
 * @param encoder encoder instance used for output.
 *
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.

 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_leave(
    pico_rnode_proto_command_encoder_t *encoder
);

/**
 * Encode the start of a transmit data frame. Subsequent data bytes should be sent
 * using pico_rnode_proto_command_data(), and the frame should be finalized with
 * pico_rnode_proto_command_end().
 * 
 * @param encoder encoder instance used for output.
 * 
 * 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_start(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface
);

/**
 * Encode a single byte of payload data for the current transmit frame.
 * 
 * @param encoder encoder instance used for output.
 
 * @param byte byte to include in the current transmit frame.
 * 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when no frame is currently open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_data(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t byte
);

/**
 * Encode the end of a transmit data frame.
 * 
 * @param encoder encoder instance used for output.
 * 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when no frame is currently open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_command_end(
    pico_rnode_proto_command_encoder_t *encoder
);

#ifdef __cplusplus
}
#endif
