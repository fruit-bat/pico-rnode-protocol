#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "pico-rnode-protocol-consts.h"
#include "pico-rnode-protocol-stream.h"

/**
 * Event decoder status codes.
 */
typedef enum {
    PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK = 0,
    PICO_RNODE_PROTO_EVENT_DECODER_STATUS_ABORTED,
    PICO_RNODE_PROTO_EVENT_DECODER_STATUS_INVALID_LENGTH,
    PICO_RNODE_PROTO_EVENT_DECODER_STATUS_UNKNOWN_OPCODE,
    PICO_RNODE_PROTO_EVENT_DECODER_STATUS_INVALID_ARGUMENT
} pico_rnode_proto_event_decoder_status_t;

/**
 * Callback invoked when event decoding fails.
 */
typedef void (*pico_rnode_proto_event_decoder_error_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t opcode,
    uint32_t index,
    pico_rnode_proto_event_decoder_status_t status
);

/**
 * Event callback for RSSI status updates.
 */
typedef void (*pico_rnode_proto_event_rssi_cb_t)(
    void * context,
    uint8_t interface,
    int8_t rssi
);

/**
 * Event callback for SNR status updates.
 */
typedef void (*pico_rnode_proto_event_snr_cb_t)(
    void * context,
    uint8_t interface,
    int8_t snr
);

/**
 * Event callback for BLINK notifications.
 */
typedef void (*pico_rnode_proto_event_blink_cb_t)(
    void * context
);

/**
 * Event callback for RANDOM notifications.
 */
typedef void (*pico_rnode_proto_event_random_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t random_value
);

/**
 * Event callback for PLATFORM notifications.
 */
typedef void (*pico_rnode_proto_event_platform_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t platform_id
);

/**
 * Event callback for MCU notifications.
 */
typedef void (*pico_rnode_proto_event_mcu_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t mcu_id
);

/**
 * Event callback for FW_VERSION notifications.
 */
typedef void (*pico_rnode_proto_event_fw_version_cb_t)(
    void * context,
    uint8_t interface,
    uint16_t version
);

/**
 * Decoder states for the event decoder state machine.
 */
typedef enum {
    PICO_RNODE_PROTO_EVENT_DECODER_STATE_WAIT_EVENT,
    PICO_RNODE_PROTO_EVENT_DECODER_STATE_READ_FIXED,
    PICO_RNODE_PROTO_EVENT_DECODER_STATE_STREAM_DATA,
    PICO_RNODE_PROTO_EVENT_DECODER_STATE_ABORT
} pico_rnode_proto_event_decode_state_t;

/**
 * Decoder for incoming protocol events.
 */
typedef struct {
    void * context;
    uint32_t payload_index;
    uint8_t smallbuf[4];
    uint8_t interface;
    uint8_t opcode;
    uint8_t opcode_length;
    pico_rnode_proto_event_decode_state_t state;

    pico_rnode_proto_event_rssi_cb_t rssi_cb;
    pico_rnode_proto_event_snr_cb_t snr_cb;
    pico_rnode_proto_event_blink_cb_t blink_cb;
    pico_rnode_proto_event_random_cb_t random_cb;
    pico_rnode_proto_event_platform_cb_t platform_cb;
    pico_rnode_proto_event_mcu_cb_t mcu_cb;
    pico_rnode_proto_event_fw_version_cb_t fw_version_cb;

    pico_rnode_proto_stream_t stream;
    pico_rnode_proto_stream_start_cb_t stat_rx_start_cb;
    pico_rnode_proto_stream_data_cb_t stat_rx_data_cb;
    pico_rnode_proto_stream_end_cb_t stat_rx_end_cb;
    pico_rnode_proto_stream_start_cb_t stat_tx_start_cb;
    pico_rnode_proto_stream_data_cb_t stat_tx_data_cb;
    pico_rnode_proto_stream_end_cb_t stat_tx_end_cb;

    pico_rnode_proto_event_decoder_error_cb_t error_cb;
} pico_rnode_proto_event_decoder_t;

/**
 * Initialize an event decoder instance.
 */
void pico_rnode_proto_event_decoder_init(
    pico_rnode_proto_event_decoder_t *decoder,
    void * context,
    pico_rnode_proto_event_rssi_cb_t rssi_cb,
    pico_rnode_proto_event_snr_cb_t snr_cb,
    pico_rnode_proto_event_blink_cb_t blink_cb,
    pico_rnode_proto_event_random_cb_t random_cb,
    pico_rnode_proto_event_platform_cb_t platform_cb,
    pico_rnode_proto_event_mcu_cb_t mcu_cb,
    pico_rnode_proto_event_fw_version_cb_t fw_version_cb,
    pico_rnode_proto_stream_start_cb_t stat_rx_start_cb,
    pico_rnode_proto_stream_data_cb_t stat_rx_data_cb,
    pico_rnode_proto_stream_end_cb_t stat_rx_end_cb,
    pico_rnode_proto_stream_start_cb_t stat_tx_start_cb,
    pico_rnode_proto_stream_data_cb_t stat_tx_data_cb,
    pico_rnode_proto_stream_end_cb_t stat_tx_end_cb,
    pico_rnode_proto_event_decoder_error_cb_t error_cb
);

/**
 * Reset the decoder state before parsing a new frame.
 */
void pico_rnode_proto_event_decoder_start(
    pico_rnode_proto_event_decoder_t *decoder
);

/**
 * Feed bytes into the event decoder.
 */
pico_rnode_proto_event_decoder_status_t pico_rnode_proto_event_decoder_write(
    pico_rnode_proto_event_decoder_t *decoder,
    const uint8_t * bytes,
    size_t len
);

/**
 * Feed a single byte into the event decoder.
 */
pico_rnode_proto_event_decoder_status_t pico_rnode_proto_event_decoder_put(
    pico_rnode_proto_event_decoder_t *decoder,
    uint8_t byte
);

/**
 * Complete the current event frame.
 */
pico_rnode_proto_event_decoder_status_t pico_rnode_proto_event_decoder_end(
    pico_rnode_proto_event_decoder_t *decoder
);

#ifdef __cplusplus
}
#endif

