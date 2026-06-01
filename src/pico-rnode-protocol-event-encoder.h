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
// Encoder for incoming protocol events (e.g. from radio to host).
// -----------------------------------------------------------

/**
 * Incoming event encoder instance.
 *
 * This struct wraps a protocol encoder to provide event-specific encoding
 * functions. The protocol encoder handles all frame start/byte/end operations.
 * 
 */
typedef struct {
    /**
     * Internal protocol encoder used to emit event frames.
     *
     * The event encoder delegates all frame operations to this helper.
     */
    pico_rnode_proto_encoder_t encoder;
} pico_rnode_proto_event_encoder_t;

/**
 * Initialize an event encoder instance.
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
void pico_rnode_proto_event_encoder_init(
    pico_rnode_proto_event_encoder_t *encoder,
    void * context,
    pico_rnode_proto_frame_start_cb_t start_cb,
    pico_rnode_proto_frame_data_cb_t put_cb,
    pico_rnode_proto_frame_end_cb_t end_cb
);


pico_rnode_proto_encoder_status_t pico_rnode_proto_event_rssi(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    int8_t rssi
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_snr(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    int8_t snr
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_stats(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint32_t packets_received,
    uint32_t packets_lost
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_error(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t error_code
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_stat_rx(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_stat_tx(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_blink(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_random(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t random_value
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_platform(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t platform_id
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_mcu(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t mcu_id
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_fw_version(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint16_t version
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_rom_read(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_reset(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_event_interfaces(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
);








/**
 * Encode the start of a receive data frame. Subsequent data bytes should be sent
 * using pico_rnode_proto_event_data(), and the frame should be finalized with
 * pico_rnode_proto_event_end().
 * 
 * Parameters:
 * - encoder: encoder instance used for output.
 * - interface: target interface identifier to send transmission to.
 * 
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_start(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface
);

/**
 * Encode a single byte of payload data for the current receive frame.
 * 
 * Parameters:
 * - encoder: encoder instance used for output.
 * - byte: byte to include in the current receive frame.
 * 
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when no frame is currently open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_data(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t byte
);

/**
 * Encode the end of a receive data frame.
 * 
 * Parameters:
 * - encoder: encoder instance used for output.
 * 
 * Returns:
 * - PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 * - PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when no frame is currently open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_end(
    pico_rnode_proto_event_encoder_t *encoder
);

#ifdef __cplusplus
}
#endif
