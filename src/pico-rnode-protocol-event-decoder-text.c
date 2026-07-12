// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat

#include <stdio.h>
#include <stdarg.h>
#include "pico-rnode-protocol-event-decoder-text.h"

static FILE *event_decoder_out(void *context) {
    pico_rnode_proto_event_decoder_text_t *text_decoder = (pico_rnode_proto_event_decoder_text_t *)context;
    return text_decoder && text_decoder->out ? text_decoder->out : stdout;
}

static const char *event_decoder_prefix(void *context) {
    pico_rnode_proto_event_decoder_text_t *text_decoder = (pico_rnode_proto_event_decoder_text_t *)context;
    return text_decoder && text_decoder->prefix ? text_decoder->prefix : "";
}

static void event_decoder_log(void *context, const char *format, ...) {
    FILE *out = event_decoder_out(context);
    const char *prefix = event_decoder_prefix(context);
    if (!out) {
        return;
    }

    if (prefix[0] != '\0') {
        fprintf(out, "%s ", prefix);
    }

    va_list args;
    va_start(args, format);
    vfprintf(out, format, args);
    va_end(args);
    fprintf(out, "\n");
}

static const char *event_decoder_opcode_name(uint8_t opcode) {
    switch (opcode) {
        case RNODE_OPCODE_STAT_RX: return "STAT_RX";
        case RNODE_OPCODE_STAT_TX: return "STAT_TX";
        case RNODE_OPCODE_STAT_RSSI: return "STAT_RSSI";
        case RNODE_OPCODE_STAT_SNR: return "STAT_SNR";
        case RNODE_OPCODE_BLINK: return "BLINK";
        case RNODE_OPCODE_RANDOM: return "RANDOM";
        case RNODE_OPCODE_PLATFORM: return "PLATFORM";
        case RNODE_OPCODE_MCU: return "MCU";
        case RNODE_OPCODE_FW_VERSION: return "FW_VERSION";
        default: return "UNKNOWN";
    }
}

static void event_decoder_rssi_cb(
    void *context,
    uint8_t interface,
    int8_t rssi
) {
    event_decoder_log(context, "EVENT RSSI interface=%u rssi=%d", interface, rssi);
}

static void event_decoder_snr_cb(
    void *context,
    uint8_t interface,
    int8_t snr
) {
    event_decoder_log(context, "EVENT SNR interface=%u snr=%d", interface, snr);
}

static void event_decoder_blink_cb(void *context) {
    event_decoder_log(context, "EVENT BLINK");
}

static void event_decoder_random_cb(
    void *context,
    uint8_t interface,
    uint8_t random_value
) {
    event_decoder_log(context, "EVENT RANDOM interface=%u value=0x%02X", interface, random_value);
}

static void event_decoder_platform_cb(
    void *context,
    uint8_t interface,
    uint8_t platform_id
) {
    event_decoder_log(context, "EVENT PLATFORM interface=%u id=0x%02X", interface, platform_id);
}

static void event_decoder_mcu_cb(
    void *context,
    uint8_t interface,
    uint8_t mcu_id
) {
    event_decoder_log(context, "EVENT MCU interface=%u id=0x%02X", interface, mcu_id);
}

static void event_decoder_fw_version_cb(
    void *context,
    uint8_t interface,
    uint16_t version
) {
    event_decoder_log(context, "EVENT FW_VERSION interface=%u version=0x%04X", interface, version);
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_rx_start_cb(
    void *context,
    uint8_t interface
) {
    event_decoder_log(context, "EVENT STAT_RX START interface=%u", interface);
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_rx_data_cb(
    void *context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
) {
    event_decoder_log(context, "EVENT STAT_RX interface=%u index=%u byte=0x%02X", interface, byte_index, byte);
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_rx_end_cb(
    void *context,
    uint8_t interface,
    uint32_t length
) {
    event_decoder_log(context, "EVENT STAT_RX END interface=%u length=%u", interface, length);
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_tx_start_cb(
    void *context,
    uint8_t interface
) {
    event_decoder_log(context, "EVENT STAT_TX START interface=%u", interface);
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_tx_data_cb(
    void *context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
) {
    event_decoder_log(context, "EVENT STAT_TX interface=%u index=%u byte=0x%02X", interface, byte_index, byte);
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_tx_end_cb(
    void *context,
    uint8_t interface,
    uint32_t length
) {
    event_decoder_log(context, "EVENT STAT_TX END interface=%u length=%u", interface, length);
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static void event_decoder_error_cb(
    void *context,
    uint8_t interface,
    uint8_t opcode,
    uint32_t index,
    pico_rnode_proto_event_decoder_status_t status
) {
    event_decoder_log(
        context,
        "EVENT ERROR interface=%u opcode=0x%02X (%s) index=%u status=%u",
        interface,
        opcode,
        event_decoder_opcode_name(opcode),
        index,
        status
    );
}

void pico_rnode_proto_event_decoder_text_init(
    pico_rnode_proto_event_decoder_text_t *text_decoder,
    pico_rnode_proto_event_decoder_t *decoder,
    FILE *out,
    const char *prefix
) {
    if (!text_decoder || !decoder) {
        return;
    }

    text_decoder->out = out ? out : stdout;
    text_decoder->prefix = prefix ? prefix : "";

    pico_rnode_proto_event_decoder_init(
        decoder,
        text_decoder,
        event_decoder_rssi_cb,
        event_decoder_snr_cb,
        event_decoder_blink_cb,
        event_decoder_random_cb,
        event_decoder_platform_cb,
        event_decoder_mcu_cb,
        event_decoder_fw_version_cb,
        event_decoder_stat_rx_start_cb,
        event_decoder_stat_rx_data_cb,
        event_decoder_stat_rx_end_cb,
        event_decoder_stat_tx_start_cb,
        event_decoder_stat_tx_data_cb,
        event_decoder_stat_tx_end_cb,
        event_decoder_error_cb
    );
}
