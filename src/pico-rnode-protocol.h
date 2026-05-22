// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/*
 * Each KISS frame contains exactly one RNode command/event.
 *
 * The first byte identifies the command opcode/interface.
 * Remaining bytes belong entirely to that command payload.
 *
 * Multiple RNode commands MUST NOT appear in a single KISS frame.
 */
typedef enum {
    PICO_RNODE_PROTO_FRAME_CB_STATUS_OK = 0,
    PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT = 1,
} pico_rnode_proto_frame_cb_status_t;

typedef enum {
    PICO_RNODE_PROTO_DECODER_STATUS_OK = 0,
    PICO_RNODE_PROTO_DECODER_STATUS_ABORTED,
    PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH,
    PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE,
} pico_rnode_proto_decoder_status_t;

typedef enum {
    PICO_RNODE_PROTO_ENCODER_STATUS_OK = 0,
    PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED,

    // A transmission is in progress but an attempt was made to send a command 
    PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR,

} pico_rnode_proto_encoder_status_t;

// -----------------------------------------------------------
// Decoder for incoming protocol commands.
// -----------------------------------------------------------

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
typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_data_decoder_data_cb_t)(
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
 * Callback invoked when a transmit/receive frame ends.
 *
 * The decoder calls this after a complete frame has been parsed.
 */
typedef void (*pico_rnode_proto_data_decoder_end_cb_t)(
    void * context,
    uint8_t interface,
    uint32_t length
);

/**
 * Callback invoked when an error occurs.
 *
 * Parameters:
 * - context: user-provided context pointer
 * - interface: the interface on which the error occurred
 * - opcode: the opcode of the command being parsed when the error occurred
 * - index: the index of the byte that triggered the error within the current command payload
 * - status: decoder error code
 */
typedef void (*pico_rnode_proto_decoder_error_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t opcode,
    uint32_t index,
    pico_rnode_proto_decoder_status_t status
);

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
    
    // Transmit command callbacks
    // TODO lose the tx_prefix(?)
    pico_rnode_proto_data_decoder_start_cb_t tx_start_cb;
    pico_rnode_proto_data_decoder_data_cb_t tx_data_cb;
    pico_rnode_proto_data_decoder_end_cb_t tx_end_cb;

    // Error callback
    pico_rnode_proto_decoder_error_cb_t error_cb;

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
    pico_rnode_proto_decoder_error_cb_t error_cb
);

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_put(
    pico_rnode_proto_command_decoder_t *decoder,
    uint8_t byte
);

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_write(
    pico_rnode_proto_command_decoder_t *decoder,
    const uint8_t* bytes,
    size_t len
);

void pico_rnode_proto_command_decoder_start(
    pico_rnode_proto_command_decoder_t *decoder
);

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_end(
    pico_rnode_proto_command_decoder_t *decoder
);

// -----------------------------------------------------------
// Encoder for outgoing protocol commands.
// -----------------------------------------------------------

typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_cmd_start_cb_t)(
    void * context
);

typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_cmd_put_cb_t)(
    void * context,
    uint8_t byte
);

typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_cmd_end_cb_t)(
    void * context
);

typedef enum {
    PICO_RNODE_PROTO_ENCODER_STATE_IDLE = 0,
    PICO_RNODE_PROTO_ENCODER_STATE_TRANSMITTING,
} pico_rnode_proto_encoder_state_t;

typedef struct {
    void * context;
    pico_rnode_proto_encoder_state_t state;
    pico_rnode_proto_cmd_start_cb_t start_cb;
    pico_rnode_proto_cmd_put_cb_t put_cb;
    pico_rnode_proto_cmd_end_cb_t end_cb;
} pico_rnode_proto_command_encoder_t;

void pico_rnode_proto_command_encoder_init(
    pico_rnode_proto_command_encoder_t *encoder,
    void * context,
    pico_rnode_proto_cmd_start_cb_t start_cb,
    pico_rnode_proto_cmd_put_cb_t put_cb,
    pico_rnode_proto_cmd_end_cb_t end_cb
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_frequency(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t hz
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_bandwidth(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t bandwidth
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_txpower(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    int8_t dbm
);















// typedef struct {
//     // TODO

// } pico_rnode_proto_event_encoder_t;

// typedef struct {
//     void * context;

//     // TODO ...

//     pico_rnode_proto_data_decoder_start_cb_t rx_start_cb;
//     pico_rnode_proto_data_decoder_data_cb_t rx_data_cb;
//     pico_rnode_proto_data_decoder_end_cb_t rx_end_cb;
//     pico_rnode_proto_decoder_error_cb_t error_cb;

// } pico_rnode_proto_event_decoder_t;

#ifdef __cplusplus
}
#endif
