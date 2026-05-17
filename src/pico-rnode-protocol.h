// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/**
 * Decoder status values returned by the transmit parser.
 */
typedef enum {
    PICO_RNODE_PROTO_FRAME_STATUS_OK = 0,
    PICO_RNODE_PROTO_FRAME_STATUS_ABORTED,
} pico_rnode_proto_data_decoder_status_t;



typedef enum {
    PICO_RNODE_PROTO_FRAME_CB_STATUS_OK = 0,
    PICO_RNODE_PROTO_FRAME_CB_STATUS_ERROR = 1,
} pico_rnode_proto_data_decoder_cb_status_t;

/**
 * Callback invoked for each transmit/receive byte.
 *
 * Parameters:
 * - context: user-provided context pointer
 * - interface: the interface on which the byte was received/transmitted
 * - byte: transmit/receive byte
 * - byte_index: zero-based index within the current transmit/receive frame
 *
 * Return:
 * - PICO_RNODE_PROTO_DATA_DECODER_CB_STATUS_OK to continue decoding
 * - PICO_RNODE_PROTO_DATA_DECODER_CB_STATUS_ERROR to abort the current transmission
 */
typedef pico_rnode_proto_data_decoder_cb_status_t (*pico_rnode_proto_data_decoder_data_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
);

/**
 * Callback invoked when a new transmit frame begins.
 *
 * The decoder calls this after receiving the transmit command any before payload 
 * bytes are delivered.
 */
typedef void (*pico_rnode_proto_data_decoder_start_cb_t)(
    void * context,
    uint8_t interface
);

/**
 * Information passed to the end-of-frame callback.
 *
 * - len: length of the decoded frame payload in bytes
 * - status: final decoder status for the frame
 */
typedef struct {
    uint32_t len;
    pico_rnode_proto_data_decoder_status_t status;
} pico_rnode_proto_frame_info_t;

/**
 * Callback invoked when a transmit/receive frame ends.
 *
 * The decoder calls this after a complete frame has been parsed.
 */
typedef void (*pico_rnode_proto_data_decoder_end_cb_t)(
    void * context,
    uint8_t interface,
    pico_rnode_proto_frame_info_t* status
);

/**
 * Callback invoked when a byte-level transmit/receive error occurs.
 *
 * Parameters:
 * - context: user-provided context pointer
 * - interface: the interface on which the error occurred
 * - status: decoder error code
 * - index: index of the byte that triggered the error within the current transmit/receive frame
 */
typedef void (*pico_rnode_proto_data_decoder_error_cb_t)(
    void * context,
    uint8_t interface,
    pico_rnode_proto_data_decoder_status_t status,
    uint32_t index
);

typedef struct {
    // TODO

} pico_rnode_proto_command_encoder_t;


typedef void (*pico_rnode_proto_cmd_set_frequency_cb_t)(
    void * context,
    uint8_t interface,
    uint32_t hz
);

typedef void (*pico_rnode_proto_cmd_set_bandwidth_cb_t)(
    void * context,
    uint8_t interface,
    uint32_t bandwidth
);

typedef void (*pico_rnode_proto_cmd_set_txpower_cb_t)(
    void * context,
    uint8_t interface,
    int8_t dbm
);

// Decoder states for the command decoder state machine
typedef enum {
    PICO_RNODE_PROTO_DECODER_STATE_WAIT_COMMAND,
    PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED,
    PICO_RNODE_PROTO_DECODER_STATE_STREAM_DATA,
    PICO_RNODE_PROTO_DECODER_STATE_ABORT
} pico_rnode_proto_decode_state_t;

/**
 * Decoder for incoming protocol commands.
 *
 * This decoder processes incoming bytes and invokes the appropriate command callbacks
 * when complete commands are received. It maintains internal state to handle multi-byte
 * commands and payloads.
 * 
 * The contents are intended to be private.
 */
typedef struct {

    // User context pointer passed to callbacks     
    void * context;

    // Internal state for command decoding
    uint32_t payload_index;
    uint8_t smallbuf[4];
    uint8_t interface;
    uint8_t opcode;
    uint8_t opcode_length;
    pico_rnode_proto_decode_state_t state;

    // Command callbacks
    pico_rnode_proto_cmd_set_frequency_cb_t set_frequency_cb;
    pico_rnode_proto_cmd_set_bandwidth_cb_t set_bandwidth_cb;
    pico_rnode_proto_cmd_set_txpower_cb_t set_txpower_cb;
    
    // transmit command callbacks
    pico_rnode_proto_data_decoder_start_cb_t tx_start_cb;
    pico_rnode_proto_data_decoder_data_cb_t tx_data_cb;
    pico_rnode_proto_data_decoder_end_cb_t tx_end_cb;
    pico_rnode_proto_data_decoder_error_cb_t tx_error_cb;

} pico_rnode_proto_command_decoder_t;

void pico_rnode_proto_command_decoder_init(
    pico_rnode_proto_command_decoder_t *decoder,
    void * context,
    pico_rnode_proto_cmd_set_frequency_cb_t set_frequency_cb,
    pico_rnode_proto_cmd_set_bandwidth_cb_t set_bandwidth_cb,
    pico_rnode_proto_cmd_set_txpower_cb_t set_txpower_cb,
    pico_rnode_proto_data_decoder_start_cb_t tx_start_cb,
    pico_rnode_proto_data_decoder_data_cb_t tx_data_cb,
    pico_rnode_proto_data_decoder_end_cb_t tx_end_cb,
    pico_rnode_proto_data_decoder_error_cb_t tx_error_cb
);

typedef enum {
    PICO_RNODE_PROTO_DECODER_STATUS_OK = 0,
    PICO_RNODE_PROTO_DECODER_STATUS_ABORTED,
    PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH,
    PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE,
} pico_rnode_proto_decoder_status_t;

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_put(
    pico_rnode_proto_command_decoder_t *decoder,
    uint8_t byte
);

void pico_rnode_proto_command_decoder_start(
    pico_rnode_proto_command_decoder_t *decoder
);

void pico_rnode_proto_command_decoder_end(
    pico_rnode_proto_command_decoder_t *decoder
);

typedef struct {
    // TODO

} pico_rnode_proto_event_encoder_t;

typedef struct {
    void * context;

    // TODO ...

    pico_rnode_proto_data_decoder_start_cb_t rx_start_cb;
    pico_rnode_proto_data_decoder_data_cb_t rx_data_cb;
    pico_rnode_proto_data_decoder_end_cb_t rx_end_cb;
    pico_rnode_proto_data_decoder_error_cb_t rx_error_cb;

} pico_rnode_proto_event_decoder_t;



typedef enum {
    RNODE_CMD_SET_FREQUENCY,
    RNODE_CMD_SET_BANDWIDTH,
    RNODE_CMD_SET_TXPOWER,
    RNODE_CMD_TRANSMIT,
} pico_rnode_proto_command_type_t;

typedef enum {
    RNODE_EVENT_PACKET_RECEIVED,
    RNODE_EVENT_READY,
    RNODE_EVENT_RSSI,
    RNODE_EVENT_SNR,
} pico_rnode_proto_event_type_t;



typedef enum {
    RNODE_EVENT_DATA,
    RNODE_EVENT_SET_FREQUENCY,
    RNODE_EVENT_SET_BANDWIDTH,
    RNODE_EVENT_SET_TXPOWER,
    RNODE_EVENT_SET_SF,
    RNODE_EVENT_SET_CR,
    RNODE_EVENT_READY,
    RNODE_EVENT_UNKNOWN
} rnode_event_type_t;

typedef struct {
    rnode_event_type_t type;
    uint8_t interface;

    union {
        struct {
            const uint8_t *data;
            size_t len;
        } data;

        struct {
            uint32_t hz;
        } frequency;

        struct {
            uint32_t bandwidth;
        } bandwidth;

        struct {
            int8_t dbm;
        } txpower;

        struct {
            uint8_t sf;
        } sf;

        struct {
            uint8_t cr;
        } cr;
    };
} rnode_event_t;


#ifdef __cplusplus
}
#endif
