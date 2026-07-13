

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico-rnode-protocol-event-decoder.h"
#include "pico-rnode-protocol-event-encoder.h"

typedef struct {
    uint8_t expected_interface;
    int8_t expected_rssi;
    int8_t expected_snr;
    uint32_t rssi_count;
    uint32_t snr_count;
    int8_t last_rssi;
    int8_t last_snr;
    uint32_t blink_count;
    uint32_t stat_rx_start_count;
    uint32_t stat_rx_data_count;
    uint32_t stat_rx_end_count;
    uint32_t stat_rx_length;
    uint32_t stat_tx_start_count;
    uint32_t stat_tx_data_count;
    uint32_t stat_tx_end_count;
    uint32_t stat_tx_length;
    uint32_t error_count;
} roundtrip_state_t;

static roundtrip_state_t rt_state;
static uint8_t rt_buffer[64];
static size_t rt_buffer_len;

static void reset_roundtrip_state(void) {
    memset(&rt_state, 0, sizeof(rt_state));
    rt_buffer_len = 0;
}

static pico_rnode_proto_frame_cb_status_t roundtrip_encoder_start_cb(void *context) {
    (void)context;
    rt_buffer_len = 0;
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static pico_rnode_proto_frame_cb_status_t roundtrip_encoder_put_cb(
    void *context,
    uint8_t byte
) {
    (void)context;
    assert(rt_buffer_len < sizeof(rt_buffer));
    rt_buffer[rt_buffer_len++] = byte;
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static pico_rnode_proto_frame_cb_status_t roundtrip_encoder_end_cb(void *context) {
    (void)context;
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static void roundtrip_decoder_error_cb(
    void *context,
    uint8_t interface,
    uint8_t opcode,
    uint32_t index,
    pico_rnode_proto_event_decoder_status_t status
) {
    (void)context;
    (void)interface;
    (void)opcode;
    (void)index;
    (void)status;
    rt_state.error_count++;
}

static void roundtrip_decoder_rssi_cb(
    void *context,
    uint8_t interface,
    int8_t rssi
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(rssi == state->expected_rssi);
    state->rssi_count++;
    state->last_rssi = rssi;
}

static void roundtrip_decoder_snr_cb(
    void *context,
    uint8_t interface,
    int8_t snr
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(snr == state->expected_snr);
    state->snr_count++;
    state->last_snr = snr;
}

static void roundtrip_decoder_blink_cb(void *context) {
    roundtrip_state_t *state = context;
    state->blink_count++;
}

static pico_rnode_proto_stream_cb_status_t roundtrip_decoder_stat_rx_start_cb(
    void *context,
    uint8_t interface
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    state->stat_rx_start_count++;
    state->stat_rx_length = 0;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t roundtrip_decoder_stat_rx_data_cb(
    void *context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(byte_index == state->stat_rx_length);
    state->stat_rx_data_count++;
    state->stat_rx_length++;
    (void)byte;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t roundtrip_decoder_stat_rx_end_cb(
    void *context,
    uint8_t interface,
    uint32_t length
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(length == state->stat_rx_length);
    state->stat_rx_end_count++;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t roundtrip_decoder_stat_tx_start_cb(
    void *context,
    uint8_t interface
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    state->stat_tx_start_count++;
    state->stat_tx_length = 0;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t roundtrip_decoder_stat_tx_data_cb(
    void *context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(byte_index == state->stat_tx_length);
    state->stat_tx_data_count++;
    state->stat_tx_length++;
    (void)byte;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t roundtrip_decoder_stat_tx_end_cb(
    void *context,
    uint8_t interface,
    uint32_t length
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(length == state->stat_tx_length);
    state->stat_tx_end_count++;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static void init_roundtrip_decoder(
    pico_rnode_proto_event_decoder_t *decoder,
    roundtrip_state_t *state
) {
    pico_rnode_proto_event_decoder_init(
        decoder,
        state,
        roundtrip_decoder_rssi_cb,
        roundtrip_decoder_snr_cb,
        roundtrip_decoder_blink_cb,
        NULL,
        NULL,
        NULL,
        NULL,
        roundtrip_decoder_stat_rx_start_cb,
        roundtrip_decoder_stat_rx_data_cb,
        roundtrip_decoder_stat_rx_end_cb,
        roundtrip_decoder_stat_tx_start_cb,
        roundtrip_decoder_stat_tx_data_cb,
        roundtrip_decoder_stat_tx_end_cb,
        roundtrip_decoder_error_cb
    );
}

static void init_roundtrip_encoder(
    pico_rnode_proto_event_encoder_t *encoder,
    roundtrip_state_t *state
) {
    (void)state;
    pico_rnode_proto_event_encoder_init(
        encoder,
        state,
        roundtrip_encoder_start_cb,
        roundtrip_encoder_put_cb,
        roundtrip_encoder_end_cb
    );
}

static void roundtrip_decode_buffer(
    pico_rnode_proto_event_decoder_t *decoder
) {
    pico_rnode_proto_event_decoder_status_t status;

    pico_rnode_proto_event_decoder_start(decoder);
    status = pico_rnode_proto_event_decoder_write(decoder, rt_buffer, rt_buffer_len);
    assert(status == PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK);
    status = pico_rnode_proto_event_decoder_end(decoder);
    assert(status == PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK);
}

static void test_round_trip_rssi(void) {
    pico_rnode_proto_event_encoder_t encoder = {0};
    pico_rnode_proto_event_decoder_t decoder = {0};

    reset_roundtrip_state();
    rt_state.expected_interface = 1;
    rt_state.expected_rssi = -42;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_event_rssi(&encoder, 1, -42) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 2);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.rssi_count == 1);
    assert(rt_state.last_rssi == -42);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_snr(void) {
    pico_rnode_proto_event_encoder_t encoder = {0};
    pico_rnode_proto_event_decoder_t decoder = {0};

    reset_roundtrip_state();
    rt_state.expected_interface = 1;
    rt_state.expected_snr = 11;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_event_snr(&encoder, 1, 11) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 2);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.snr_count == 1);
    assert(rt_state.last_snr == 11);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_blink(void) {
    pico_rnode_proto_event_encoder_t encoder = {0};
    pico_rnode_proto_event_decoder_t decoder = {0};

    reset_roundtrip_state();
    rt_state.expected_interface = 4;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_event_blink(&encoder, 4) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 1);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.blink_count == 1);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_stat_rx(void) {
    pico_rnode_proto_event_encoder_t encoder = {0};
    pico_rnode_proto_event_decoder_t decoder = {0};
    const uint8_t payload[] = {0x0A, 0x0B, 0x0C};

    reset_roundtrip_state();
    rt_state.expected_interface = 2;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_event_stat_rx(&encoder, 2, payload, sizeof(payload)) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == sizeof(payload) + 1);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.stat_rx_start_count == 1);
    assert(rt_state.stat_rx_data_count == sizeof(payload));
    assert(rt_state.stat_rx_end_count == 1);
    assert(rt_state.stat_rx_length == sizeof(payload));
    assert(rt_state.error_count == 0);
}

static void test_round_trip_stat_tx(void) {
    pico_rnode_proto_event_encoder_t encoder = {0};
    pico_rnode_proto_event_decoder_t decoder = {0};
    const uint8_t payload[] = {0x10, 0x20};

    reset_roundtrip_state();
    rt_state.expected_interface = 3;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_event_stat_tx(&encoder, 3, payload, sizeof(payload)) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == sizeof(payload) + 1);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.stat_tx_start_count == 1);
    assert(rt_state.stat_tx_data_count == sizeof(payload));
    assert(rt_state.stat_tx_end_count == 1);
    assert(rt_state.stat_tx_length == sizeof(payload));
    assert(rt_state.error_count == 0);
}

static void run_test(const char *name, void (*fn)(void)) {
    printf("[ RUN ] %s\n", name);
    reset_roundtrip_state();
    fn();
    printf("[ PASS ] %s\n", name);
}

void test_event_round_trip(void) {
    run_test("event_round_trip_rssi", test_round_trip_rssi);
    run_test("event_round_trip_snr", test_round_trip_snr);
    run_test("event_round_trip_blink", test_round_trip_blink);
    run_test("event_round_trip_stat_rx", test_round_trip_stat_rx);
    run_test("event_round_trip_stat_tx", test_round_trip_stat_tx);
    printf("All event round trip tests passed.\n");
}