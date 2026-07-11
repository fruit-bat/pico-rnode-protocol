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
 * @param encoder encoder instance to initialize.

 * @param context opaque user context passed to callbacks.

 * @param start_cb callback invoked at frame start.

 * @param put_cb callback invoked for each emitted byte.

 * @param end_cb callback invoked at frame end.
 *
 * @return None.
 */
void pico_rnode_proto_event_encoder_init(
    pico_rnode_proto_event_encoder_t *encoder,
    void * context,
    pico_rnode_proto_frame_start_cb_t start_cb,
    pico_rnode_proto_frame_data_cb_t put_cb,
    pico_rnode_proto_frame_end_cb_t end_cb
);

/**
 * Encode an RSSI event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param rssi received signal strength indicator in dBm.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_rssi(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    int8_t rssi
);

/**
 * Encode an SNR event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param snr signal-to-noise ratio in dB.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_snr(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    int8_t snr
);

/**
 * Encode a statistics event reporting packet counts.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param packets_received number of packets received.

 * @param packets_lost number of packets lost.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_stats(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint32_t packets_received,
    uint32_t packets_lost
);

/**
 * Encode an error event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param error_code protocol-specific error code.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_error(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t error_code
);

/**
 * Encode a received payload event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param payload pointer to payload data.

 * @param len payload length in bytes.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_stat_rx(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
);

/**
 * Encode a transmitted payload event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param payload pointer to payload data.

 * @param len payload length in bytes.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_stat_tx(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
);

/**
 * Encode a blink event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_blink(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface
);

/**
 * Encode a random value event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param random_value random value to encode.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_random(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t random_value
);

/**
 * Encode a platform identifier event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param platform_id platform identifier value.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_platform(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t platform_id
);

/**
 * Encode an MCU identifier event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param mcu_id MCU identifier value.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_mcu(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint8_t mcu_id
);

/**
 * Encode a firmware version event.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param version firmware version value.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_fw_version(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    uint16_t version
);

/**
 * Encode a ROM read event with payload data.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param payload pointer to ROM data.

 * @param len payload length in bytes.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_rom_read(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
);

/**
 * Encode a reset event with optional payload.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param payload pointer to reset payload data.

 * @param len payload length in bytes.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_reset(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface,
    const uint8_t *payload,
    size_t len
);

/**
 * Encode an interfaces event with payload data.
 *
 * @param encoder encoder instance used for output.

 * @param interface source interface identifier.

 * @param payload pointer to interface payload data.

 * @param len payload length in bytes.
 */
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
 * @param encoder encoder instance used for output.
 
 * @param interface target interface identifier to send transmission to.
 * 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when a frame is already open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_start(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t interface
);

/**
 * Encode a single byte of payload data for the current receive frame.
 * 
 * @param encoder encoder instance used for output.
 
 * @param byte byte to include in the current receive frame.
 * 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when no frame is currently open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_data(
    pico_rnode_proto_event_encoder_t *encoder,
    uint8_t byte
);

/**
 * Encode the end of a receive data frame.
 * 
 * @param encoder encoder instance used for output.
 * 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_OK when encoding succeeded.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED when encoding was aborted.
 
 * @return PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR when no frame is currently open.
 */
pico_rnode_proto_encoder_status_t pico_rnode_proto_event_end(
    pico_rnode_proto_event_encoder_t *encoder
);

#ifdef __cplusplus
}
#endif
