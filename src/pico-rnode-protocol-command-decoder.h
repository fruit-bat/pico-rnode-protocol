// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "pico-rnode-protocol-consts.h"
#include "pico-rnode-protocol-stream.h"

/*
 * Each KISS frame contains exactly one RNode command/event.
 *
 * The first byte identifies the command opcode/interface.
 * Remaining bytes belong entirely to that command payload.
 *
 * Multiple RNode commands MUST NOT appear in a single KISS frame.
 */

/**
 * Decoder status codes.
 *
 * These values are returned by decoder operations to indicate whether the
 * incoming command byte stream was accepted, rejected, or aborted.
 */
typedef enum {
    PICO_RNODE_PROTO_DECODER_STATUS_OK = 0,         /**< Decoding succeeded. */
    PICO_RNODE_PROTO_DECODER_STATUS_ABORTED,        /**< Decoding aborted by callback or state error. */
    PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH, /**< Command payload length was invalid. */
    PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE, /**< Command opcode is not recognized. */
    PICO_RNODE_PROTO_DECODER_STATUS_INVALID_ARGUMENT /**< Command payload contained invalid arguments. */
} pico_rnode_proto_decoder_status_t;

// -----------------------------------------------------------
// Decoder for incoming protocol commands (e.g. from radio to host).
// -----------------------------------------------------------

/**
 * Callback invoked when a decoder error occurs.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier where the error occurred.
 * - opcode: command opcode being decoded when the error was detected.
 * - index: byte index within the current payload where the error occurred.
 * - status: decoder status code describing the error.
 */
typedef void (*pico_rnode_proto_decoder_error_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t opcode,
    uint32_t index,
    pico_rnode_proto_decoder_status_t status
);

/**
 * Command handler for setting the RF frequency.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier for the command.
 * - hz: frequency in Hertz to apply.
 */
typedef void (*pico_rnode_proto_command_set_frequency_cb_t)(
    void * context,
    uint8_t interface,
    uint32_t hz
);

/**
 * Command handler for setting the RF bandwidth.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier for the command.
 * - bandwidth: bandwidth in Hertz to apply.
 */
typedef void (*pico_rnode_proto_command_set_bandwidth_cb_t)(
    void * context,
    uint8_t interface,
    uint32_t bandwidth
);

/**
 * Command handler for setting the transmit power.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier for the command.
 * - dbm: transmit power in dBm.
 */
typedef void (*pico_rnode_proto_command_set_txpower_cb_t)(
    void * context,
    uint8_t interface,
    int8_t dbm
);

/**
 * Command handler for setting the LoRa spreading factor.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier for the command.
 * - sf: LoRa spreading factor value.
 */
typedef void (*pico_rnode_proto_command_set_spreading_factor_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t sf
);

/**
 * Command handler for setting the LoRa coding rate.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier for the command.
 * - cr: LoRa coding rate value.
 */
typedef void (*pico_rnode_proto_command_set_coding_rate_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t cr
);

/**
 * Command handler for radio detection.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 */
typedef void (*pico_rnode_proto_command_detect_cb_t)(
    void * context
);

/**
 * Command handler for changing radio state.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 * - interface: interface identifier for the command.
 * - state: radio state to apply.
 */
typedef void (*pico_rnode_proto_command_set_radio_state_cb_t)(
    void * context,
    uint8_t interface,
    pico_rnode_proto_radio_state_t state
);

/**
 * Command handler for receiving a ready notification.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 */
typedef void (*pico_rnode_proto_command_ready_cb_t)(
    void * context
);

/**
 * Command handler for leaving the current mode or network.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 */
typedef void (*pico_rnode_proto_command_leave_cb_t)(
    void * context
);

/**
 * Command handler for locking the radio or protocol state.
 *
 * Parameters:
 * - context: opaque user pointer passed to the callback.
 */
typedef void (*pico_rnode_proto_command_lock_cb_t)(
    void * context,
    uint8_t interface,
    uint8_t lock_state
);

/**
 * Decoder states for the command decoder state machine.
 */
typedef enum {
    PICO_RNODE_PROTO_DECODER_STATE_WAIT_COMMAND, /**< Waiting for a new command opcode. */
    PICO_RNODE_PROTO_DECODER_STATE_READ_FIXED,   /**< Reading a fixed-length command payload. */
    PICO_RNODE_PROTO_DECODER_STATE_STREAM_DATA,  /**< Streaming command payload bytes until end-of-frame. */
    PICO_RNODE_PROTO_DECODER_STATE_ABORT         /**< Aborted due to error or callback request. */
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

    /** User context pointer passed to callbacks. */
    void * context;

    /** Current index into the active command payload or streaming data. */
    uint32_t payload_index;

    /** Small scratch buffer used for fixed-length command payload decoding. */
    uint8_t smallbuf[4];

    /** Interface identifier for the current command. */
    uint8_t interface;

    /** Opcode of the command currently being decoded. */
    uint8_t opcode;

    /** Expected payload length for the current opcode. */
    uint8_t opcode_length;

    /** Internal decoder state machine state. */
    pico_rnode_proto_decode_state_t state;

    /** Command callbacks. */
    pico_rnode_proto_command_detect_cb_t detect_cb;
    pico_rnode_proto_command_set_frequency_cb_t set_frequency_cb;
    pico_rnode_proto_command_set_bandwidth_cb_t set_bandwidth_cb;
    pico_rnode_proto_command_set_txpower_cb_t set_txpower_cb;
    pico_rnode_proto_command_set_spreading_factor_cb_t set_spreading_factor_cb;
    pico_rnode_proto_command_set_coding_rate_cb_t set_coding_rate_cb;
    pico_rnode_proto_command_set_radio_state_cb_t set_radio_state_cb;
    pico_rnode_proto_command_ready_cb_t ready_cb;
    pico_rnode_proto_command_lock_cb_t lock_cb;
    pico_rnode_proto_command_leave_cb_t leave_cb;

    /** Transmit frame callbacks invoked for each in-flight data frame. */
    pico_rnode_proto_stream_t tx_stream;

    /** Error callback invoked when decoding fails. */
    pico_rnode_proto_decoder_error_cb_t error_cb;

} pico_rnode_proto_command_decoder_t;

/**
 * Initialize a command decoder instance.
 *
 * The decoder will use the provided callbacks when commands are successfully
 * parsed from incoming bytes.
 *
 * Parameters:
 * - decoder: pointer to the decoder instance to initialize.
 * - context: opaque user context passed through to all callbacks.
 * - detect_cb: callback invoked for a radio detect command.
 * - set_frequency_cb: callback invoked for a set frequency command.
 * - set_bandwidth_cb: callback invoked for a set bandwidth command.
 * - set_txpower_cb: callback invoked for a set transmit power command.
 * - set_spreading_factor_cb: callback invoked for a set spreading factor command.
 * - set_coding_rate_cb: callback invoked for a set coding rate command.
 * - set_radio_state_cb: callback invoked for a set radio state command.
 * - ready_cb: callback invoked for a ready command.
 * - lock_cb: callback invoked for a lock command.
 * - leave_cb: callback invoked for a leave command.
 * - tx_start_cb: callback invoked when a transmit frame begins.
 * - tx_data_cb: callback invoked for each byte inside a transmit frame.
 * - tx_end_cb: callback invoked when a transmit frame ends.
 * - error_cb: callback invoked when a decoder error occurs.
 *
 * Returns:
 * - None.
 */
void pico_rnode_proto_command_decoder_init(
    pico_rnode_proto_command_decoder_t *decoder,
    void * context,
    pico_rnode_proto_command_detect_cb_t detect_cb,
    pico_rnode_proto_command_set_frequency_cb_t set_frequency_cb,
    pico_rnode_proto_command_set_bandwidth_cb_t set_bandwidth_cb,
    pico_rnode_proto_command_set_txpower_cb_t set_txpower_cb,
    pico_rnode_proto_command_set_spreading_factor_cb_t set_spreading_factor_cb,
    pico_rnode_proto_command_set_coding_rate_cb_t set_coding_rate_cb,
    pico_rnode_proto_command_set_radio_state_cb_t set_radio_state_cb,
    pico_rnode_proto_command_ready_cb_t ready_cb,
    pico_rnode_proto_command_lock_cb_t lock_cb,
    pico_rnode_proto_command_leave_cb_t leave_cb,
    pico_rnode_proto_stream_start_cb_t tx_start_cb,
    pico_rnode_proto_stream_data_cb_t tx_data_cb,
    pico_rnode_proto_stream_end_cb_t tx_end_cb,
    pico_rnode_proto_decoder_error_cb_t error_cb
);

/**
 * Decode a single byte from the incoming command stream.
 *
 * Parameters:
 * - decoder: decoder instance handling parsing state.
 * - byte: next byte from the incoming command stream.
 *
 * Returns:
 * - PICO_RNODE_PROTO_DECODER_STATUS_OK when decoding succeeded.
 * - PICO_RNODE_PROTO_DECODER_STATUS_ABORTED when decoding was aborted.
 * - PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH when the command payload is invalid.
 * - PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE when the opcode cannot be handled.
 * - PICO_RNODE_PROTO_DECODER_STATUS_INVALID_ARGUMENT when a command argument is invalid.
 */
pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_put(
    pico_rnode_proto_command_decoder_t *decoder,
    uint8_t byte
);

/**
 * Decode a buffer of incoming bytes.
 *
 * Parameters:
 * - decoder: decoder instance handling parsing state.
 * - bytes: pointer to the input byte buffer.
 * - len: number of bytes to decode from the buffer.
 *
 * Returns:
 * - PICO_RNODE_PROTO_DECODER_STATUS_OK when decoding succeeded.
 * - PICO_RNODE_PROTO_DECODER_STATUS_ABORTED when decoding was aborted.
 * - PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH when the command payload is invalid.
 * - PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE when the opcode cannot be handled.
 * - PICO_RNODE_PROTO_DECODER_STATUS_INVALID_ARGUMENT when a command argument is invalid.
 */
pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_write(
    pico_rnode_proto_command_decoder_t *decoder,
    const uint8_t* bytes,
    size_t len
);

/**
 * Notify the decoder that a new command frame has started.
 *
 * Parameters:
 * - decoder: decoder instance handling parsing state.
 */
void pico_rnode_proto_command_decoder_start(
    pico_rnode_proto_command_decoder_t *decoder
);

/**
 * Notify the decoder that the current command frame has ended.
 *
 * Parameters:
 * - decoder: decoder instance handling parsing state.
 *
 * Returns:
 * - PICO_RNODE_PROTO_DECODER_STATUS_OK when the frame ended successfully.
 * - PICO_RNODE_PROTO_DECODER_STATUS_ABORTED when decoding was aborted.
 * - PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH when the final frame length was invalid.
 * - PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE when the command opcode was not recognized.
 * - PICO_RNODE_PROTO_DECODER_STATUS_INVALID_ARGUMENT when the command payload was invalid.
 */
pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_end(
    pico_rnode_proto_command_decoder_t *decoder
);

#ifdef __cplusplus
}
#endif
