// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat

#include <stdio.h>
#include <stdarg.h>
#include "pico-rnode-protocol-command-decoder-text.h"

static FILE *text_decoder_out(void *context) {
    pico_rnode_proto_command_decoder_text_t *text_decoder = (pico_rnode_proto_command_decoder_text_t *)context;
    return text_decoder && text_decoder->out ? text_decoder->out : stdout;
}

static void text_decoder_log(void *context, const char *format, ...) {
    FILE *out = text_decoder_out(context);
    if (!out) {
        return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(out, format, args);
    va_end(args);
    fprintf(out, "\n");
}

static const char *text_decoder_radio_state_name(pico_rnode_proto_radio_state_t state) {
    switch (state) {
        case RNODE_RADIO_STATE_OFF: return "OFF";
        case RNODE_RADIO_STATE_ON: return "ON";
        case RNODE_RADIO_STATE_ASK: return "ASK";
        default: return "UNKNOWN";
    }
}

static const char *text_decoder_opcode_name(uint8_t opcode) {
    switch (opcode) {
        case RNODE_OPCODE_DATA: return "DATA";
        case RNODE_OPCODE_FREQUENCY: return "FREQUENCY";
        case RNODE_OPCODE_BANDWIDTH: return "BANDWIDTH";
        case RNODE_OPCODE_TXPOWER: return "TXPOWER";
        case RNODE_OPCODE_SF: return "SF";
        case RNODE_OPCODE_CR: return "CR";
        case RNODE_OPCODE_RADIO_STATE: return "RADIO_STATE";
        case RNODE_OPCODE_DETECT: return "DETECT";
        case RNODE_OPCODE_LEAVE: return "LEAVE";
        case RNODE_OPCODE_READY: return "READY";
        case RNODE_OPCODE_RADIO_LOCK: return "RADIO_LOCK";
        default: return "UNKNOWN";
    }
}

static pico_rnode_proto_stream_cb_status_t text_decoder_tx_start_cb(
    void *context,
    uint8_t interface
) {
    text_decoder_log(context, "TX START interface=%u", interface);
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t text_decoder_tx_data_cb(
    void *context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
) {
    text_decoder_log(context, "TX DATA interface=%u index=%u byte=0x%02X", interface, byte_index, byte);
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t text_decoder_tx_end_cb(
    void *context,
    uint8_t interface,
    uint32_t length
) {
    text_decoder_log(context, "TX END interface=%u length=%u", interface, length);
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static void text_decoder_detect_cb(void *context) {
    text_decoder_log(context, "DETECT");
}

static void text_decoder_set_frequency_cb(
    void *context,
    uint8_t interface,
    uint32_t hz
) {
    text_decoder_log(context, "FREQUENCY interface=%u hz=%u", interface, hz);
}

static void text_decoder_set_bandwidth_cb(
    void *context,
    uint8_t interface,
    uint32_t bandwidth
) {
    text_decoder_log(context, "BANDWIDTH interface=%u bandwidth=%u", interface, bandwidth);
}

static void text_decoder_set_txpower_cb(
    void *context,
    uint8_t interface,
    int8_t dbm
) {
    text_decoder_log(context, "TXPOWER interface=%u dbm=%d", interface, dbm);
}

static void text_decoder_set_spreading_factor_cb(
    void *context,
    uint8_t interface,
    uint8_t sf
) {
    text_decoder_log(context, "SF interface=%u sf=%u", interface, sf);
}

static void text_decoder_set_coding_rate_cb(
    void *context,
    uint8_t interface,
    uint8_t cr
) {
    text_decoder_log(context, "CR interface=%u cr=%u", interface, cr);
}

static void text_decoder_set_radio_state_cb(
    void *context,
    uint8_t interface,
    pico_rnode_proto_radio_state_t state
) {
    text_decoder_log(context, "RADIO_STATE interface=%u state=%s", interface, text_decoder_radio_state_name(state));
}

static void text_decoder_ready_cb(void *context) {
    text_decoder_log(context, "READY");
}

static void text_decoder_lock_cb(
    void *context,
    uint8_t interface,
    uint8_t lock_state
) {
    text_decoder_log(context, "LOCK interface=%u state=%u", interface, lock_state);
}

static void text_decoder_leave_cb(void *context) {
    text_decoder_log(context, "LEAVE");
}

static void text_decoder_error_cb(
    void *context,
    uint8_t interface,
    uint8_t opcode,
    uint32_t index,
    pico_rnode_proto_decoder_status_t status
) {
    text_decoder_log(
        context,
        "ERROR interface=%u opcode=0x%02X (%s) index=%u status=%u",
        interface,
        opcode,
        text_decoder_opcode_name(opcode),
        index,
        status
    );
}

void pico_rnode_proto_command_decoder_text_init(
    pico_rnode_proto_command_decoder_text_t *text_decoder,
    FILE *out
) {
    if (!text_decoder) {
        return;
    }

    text_decoder->out = out ? out : stdout;

    pico_rnode_proto_command_decoder_init(
        &text_decoder->decoder,
        text_decoder,
        text_decoder_detect_cb,
        text_decoder_set_frequency_cb,
        text_decoder_set_bandwidth_cb,
        text_decoder_set_txpower_cb,
        text_decoder_set_spreading_factor_cb,
        text_decoder_set_coding_rate_cb,
        text_decoder_set_radio_state_cb,
        text_decoder_ready_cb,
        text_decoder_lock_cb,
        text_decoder_leave_cb,
        text_decoder_tx_start_cb,
        text_decoder_tx_data_cb,
        text_decoder_tx_end_cb,
        text_decoder_error_cb
    );
}

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_text_put(
    pico_rnode_proto_command_decoder_text_t *text_decoder,
    uint8_t byte
) {
    if (!text_decoder) {
        return PICO_RNODE_PROTO_DECODER_STATUS_ABORTED;
    }
    return pico_rnode_proto_command_decoder_put(&text_decoder->decoder, byte);
}

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_text_write(
    pico_rnode_proto_command_decoder_text_t *text_decoder,
    const uint8_t *bytes,
    size_t len
) {
    if (!text_decoder || !bytes) {
        return PICO_RNODE_PROTO_DECODER_STATUS_ABORTED;
    }
    return pico_rnode_proto_command_decoder_write(&text_decoder->decoder, bytes, len);
}

void pico_rnode_proto_command_decoder_text_start(
    pico_rnode_proto_command_decoder_text_t *text_decoder
) {
    if (!text_decoder) {
        return;
    }
    pico_rnode_proto_command_decoder_start(&text_decoder->decoder);
}

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_text_end(
    pico_rnode_proto_command_decoder_text_t *text_decoder
) {
    if (!text_decoder) {
        return PICO_RNODE_PROTO_DECODER_STATUS_ABORTED;
    }
    return pico_rnode_proto_command_decoder_end(&text_decoder->decoder);
}
