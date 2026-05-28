// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "pico-rnode-protocol-command-encoder.h"
#include "pico-rnode-protocol-command-decoder.h"
#include "test_utils.h"

typedef struct {
    uint8_t expected_interface;
    uint32_t expected_frequency;
    uint32_t expected_bandwidth;
    int8_t expected_txpower;
    uint8_t expected_spreading_factor;
    uint8_t expected_coding_rate;
    pico_rnode_proto_radio_state_t expected_radio_state;

    uint32_t detect_count;
    uint32_t frequency_count;
    uint32_t bandwidth_count;
    uint32_t txpower_count;
    uint32_t spreading_factor_count;
    uint32_t coding_rate_count;
    uint32_t radio_state_count;
    uint32_t ready_count;
    uint32_t leave_count;
    uint32_t error_count;

    uint32_t last_interface;
    uint32_t last_length;
} roundtrip_state_t;

static roundtrip_state_t rt_state;
static uint8_t rt_buffer[32];
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
    pico_rnode_proto_decoder_status_t status
) {
    (void)context;
    (void)opcode;
    (void)index;
    (void)status;
    rt_state.error_count++;
}

static void roundtrip_decoder_detect_cb(void *context) {
    roundtrip_state_t *state = context;
    state->detect_count++;
}

static void roundtrip_decoder_set_frequency_cb(
    void *context,
    uint8_t interface,
    uint32_t frequency_hz
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(frequency_hz == state->expected_frequency);
    state->frequency_count++;
}

static void roundtrip_decoder_set_bandwidth_cb(
    void *context,
    uint8_t interface,
    uint32_t bandwidth
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(bandwidth == state->expected_bandwidth);
    state->bandwidth_count++;
}

static void roundtrip_decoder_set_txpower_cb(
    void *context,
    uint8_t interface,
    int8_t dbm
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(dbm == state->expected_txpower);
    state->txpower_count++;
}

static void roundtrip_decoder_set_spreading_factor_cb(
    void *context,
    uint8_t interface,
    uint8_t sf
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(sf == state->expected_spreading_factor);
    state->spreading_factor_count++;
}

static void roundtrip_decoder_set_coding_rate_cb(
    void *context,
    uint8_t interface,
    uint8_t cr
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(cr == state->expected_coding_rate);
    state->coding_rate_count++;
}

static void roundtrip_decoder_set_radio_state_cb(
    void *context,
    uint8_t interface,
    pico_rnode_proto_radio_state_t state_value
) {
    roundtrip_state_t *state = context;
    assert(interface == state->expected_interface);
    assert(state_value == state->expected_radio_state);
    state->radio_state_count++;
}

static void roundtrip_decoder_ready_cb(void *context) {
    roundtrip_state_t *state = context;
    state->ready_count++;
}

static void roundtrip_decoder_lock_cb(void *context) {
    (void)context;
}

static void roundtrip_decoder_leave_cb(void *context) {
    roundtrip_state_t *state = context;
    state->leave_count++;
}

static void roundtrip_decoder_tx_start_cb(
    void *context,
    uint8_t interface
) {
    (void)context;
    (void)interface;
}

static pico_rnode_proto_frame_cb_status_t roundtrip_decoder_tx_data_cb(
    void *context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
) {
    (void)context;
    (void)interface;
    (void)byte;
    (void)byte_index;
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static void roundtrip_decoder_tx_end_cb(
    void *context,
    uint8_t interface,
    uint32_t len
) {
    (void)context;
    (void)interface;
    (void)len;
}

static void init_roundtrip_decoder(
    pico_rnode_proto_command_decoder_t *decoder,
    roundtrip_state_t *state
) {
    pico_rnode_proto_command_decoder_init(
        decoder,
        state,
        roundtrip_decoder_detect_cb,
        roundtrip_decoder_set_frequency_cb,
        roundtrip_decoder_set_bandwidth_cb,
        roundtrip_decoder_set_txpower_cb,
        roundtrip_decoder_set_spreading_factor_cb,
        roundtrip_decoder_set_coding_rate_cb,
        roundtrip_decoder_set_radio_state_cb,
        roundtrip_decoder_ready_cb,
        roundtrip_decoder_lock_cb,
        roundtrip_decoder_leave_cb,
        roundtrip_decoder_tx_start_cb,
        roundtrip_decoder_tx_data_cb,
        roundtrip_decoder_tx_end_cb,
        roundtrip_decoder_error_cb
    );
}

static void init_roundtrip_encoder(
    pico_rnode_proto_command_encoder_t *encoder,
    roundtrip_state_t *state
) {
    (void)state;
    pico_rnode_proto_command_encoder_init(
        encoder,
        state,
        roundtrip_encoder_start_cb,
        roundtrip_encoder_put_cb,
        roundtrip_encoder_end_cb
    );
}

static void roundtrip_decode_buffer(
    pico_rnode_proto_command_decoder_t *decoder
) {
    pico_rnode_proto_decoder_status_t status;
    pico_rnode_proto_command_decoder_start(decoder);
    status = pico_rnode_proto_command_decoder_write(decoder, rt_buffer, rt_buffer_len);
    assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    status = pico_rnode_proto_command_decoder_end(decoder);
    assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
}

static void test_round_trip_set_frequency(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_command_decoder_t decoder = {0};

    reset_roundtrip_state();
    rt_state.expected_interface = 2;
    rt_state.expected_frequency = 867252736U;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_command_set_frequency(&encoder, 2, 867252736U) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 5);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.frequency_count == 1);
    assert(rt_state.detect_count == 0);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_set_bandwidth(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_command_decoder_t decoder = {0};

    reset_roundtrip_state();
    rt_state.expected_interface = 1;
    rt_state.expected_bandwidth = 125000UL;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_command_set_bandwidth(&encoder, 1, 125000UL) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 5);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.bandwidth_count == 1);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_set_txpower(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_command_decoder_t decoder = {0};

    reset_roundtrip_state();
    rt_state.expected_interface = 0;
    rt_state.expected_txpower = -3;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_command_set_txpower(&encoder, 0, -3) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 2);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.txpower_count == 1);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_set_spreading_factor(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_command_decoder_t decoder = {0};

    reset_roundtrip_state();
    rt_state.expected_interface = 1;
    rt_state.expected_spreading_factor = 7;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_command_set_spreading_factor(&encoder, 1, 7) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 2);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.spreading_factor_count == 1);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_set_coding_rate(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_command_decoder_t decoder = {0};

    reset_roundtrip_state();
    rt_state.expected_interface = 0;
    rt_state.expected_coding_rate = 5;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_command_set_coding_rate(&encoder, 0, 5) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 2);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.coding_rate_count == 1);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_set_radio_state(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_command_decoder_t decoder = {0};

    reset_roundtrip_state();
    rt_state.expected_interface = 2;
    rt_state.expected_radio_state = RNODE_RADIO_STATE_ON;

    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_command_set_radio_state(&encoder, 2, RNODE_RADIO_STATE_ON) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 2);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.radio_state_count == 1);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_detect(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_command_decoder_t decoder = {0};

    reset_roundtrip_state();
    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_command_detect(&encoder) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 2);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.detect_count == 1);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_ready(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_command_decoder_t decoder = {0};

    reset_roundtrip_state();
    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_command_ready(&encoder) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 1);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.ready_count == 1);
    assert(rt_state.error_count == 0);
}

static void test_round_trip_leave(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_command_decoder_t decoder = {0};

    reset_roundtrip_state();
    init_roundtrip_encoder(&encoder, &rt_state);
    init_roundtrip_decoder(&decoder, &rt_state);

    assert(pico_rnode_proto_command_leave(&encoder) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(rt_buffer_len == 2);

    roundtrip_decode_buffer(&decoder);
    assert(rt_state.leave_count == 1);
    assert(rt_state.error_count == 0);
}

static void run_test(const char *name, void (*fn)(void)) {
    printf("[ RUN ] %s\n", name);
    reset_roundtrip_state();
    fn();
    printf("[ PASS ] %s\n", name);
}

void test_command_round_trip(void) {
    run_test("round_trip_set_frequency", test_round_trip_set_frequency);
    run_test("round_trip_set_bandwidth", test_round_trip_set_bandwidth);
    run_test("round_trip_set_txpower", test_round_trip_set_txpower);
    run_test("round_trip_set_spreading_factor", test_round_trip_set_spreading_factor);
    run_test("round_trip_set_coding_rate", test_round_trip_set_coding_rate);
    run_test("round_trip_set_radio_state", test_round_trip_set_radio_state);
    run_test("round_trip_detect", test_round_trip_detect);
    run_test("round_trip_ready", test_round_trip_ready);
    run_test("round_trip_leave", test_round_trip_leave);

    printf("All command round-trip tests passed.\n");
}
