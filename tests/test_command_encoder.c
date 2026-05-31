// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pico-rnode-protocol-command-encoder.h"
#include "test_utils.h"

static uint8_t data[9] = {0};
static uint32_t data_index = 0;
static uint32_t start_cb_count = 0;
static uint32_t put_cb_count = 0;
static uint32_t end_cb_count = 0;
static void* last_callback_context = NULL;
static uint32_t test_context = 0xDEADBEEF;

static void clear_test_state(void) {
    data_index = 0;
    start_cb_count = 0;
    put_cb_count = 0;
    end_cb_count = 0;
    last_callback_context = NULL;
}

static pico_rnode_proto_frame_cb_status_t pico_rnode_proto_frame_start_cb_test(
    void * context
) {
    start_cb_count++;
    last_callback_context = context;
    data_index = 0;
    fprintf(stderr, "cmd_start_cb called\n");
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static pico_rnode_proto_frame_cb_status_t pico_rnode_proto_frame_data_cb_test(
    void * context,
    uint8_t byte
) {
    put_cb_count++;
    last_callback_context = context;
    data[data_index++] = byte;
    fprintf(stderr, "cmd_put_cb called with byte: 0x%02X\n", byte);
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static pico_rnode_proto_frame_cb_status_t pico_rnode_proto_frame_end_cb_test(
    void * context
) {
    end_cb_count++;
    last_callback_context = context;
    fprintf(stderr, "cmd_end_cb called\n");
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static void pico_rnode_proto_command_encoder_init_test(
    pico_rnode_proto_command_encoder_t *encoder
) {
    pico_rnode_proto_command_encoder_init(
        encoder,
        &test_context,
        pico_rnode_proto_frame_start_cb_test,
        pico_rnode_proto_frame_data_cb_test,
        pico_rnode_proto_frame_end_cb_test
    );
}

// Test encoding a set frequency command and verify the output frame
static void test_pico_rnode_proto_command_set_frequency(void) {

    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    status = pico_rnode_proto_command_set_frequency(
        &encoder,
        2, // Interface 2 
        867252736
    );
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for setting frequency on interface 2 to 867252736 Hz is:
    // Byte 0: 0x21 (Interface 2, opcode 1 - set frequency)
    // Byte 1-4: 0x33B13A00 (867252736 in big-endian format)
    const uint8_t frame[] = { 
        0x21, // Interface 2, opcode 1 (set frequency - RNODE_OPCODE_FREQUENCY)
        0x33, // Payload byte 0 (MSB)
        0xB1, // Payload byte 1
        0x3A, // Payload byte 2
        0x00, // Payload byte 3 (LSB)
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);

    assert(last_callback_context == &test_context);

    assert_equal_bytes(frame, data, sizeof(frame));
}

// Test encoding a set bandwidth command and verify the output frame
static void test_pico_rnode_proto_command_set_bandwidth(void) {

    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    status = pico_rnode_proto_command_set_bandwidth(
        &encoder,
        1, // Interface 1 
        125000UL
    );
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for setting bandwidth on interface 1 to 125000 Hz is:
    // Byte 0: 0x12 (Interface 1, opcode 2 - set bandwidth)
    // Byte 1-4: 0x0001E848 (125000 in big-endian format)
    const uint8_t frame[] = { 
        0x12, // Interface 1, opcode 2 (set bandwidth - RNODE_OPCODE_BANDWIDTH)
        0x00, // Payload byte 0 (MSB)
        0x01, // Payload byte 1
        0xE8, // Payload byte 2
        0x48, // Payload byte 3 (LSB)
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);

    assert(last_callback_context == &test_context);

    assert_equal_bytes(frame, data, sizeof(frame));
}

// Test encoding a set txpower command and verify the output frame
static void test_pico_rnode_proto_command_set_txpower(void) {

    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    status = pico_rnode_proto_command_set_txpower(
        &encoder,
        0, // Interface 0 
        -3 // Tx power of -3 dBm
    );
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for setting txpower on interface 0 to -3 dBm is:
    // Byte 0: 0x03 (Interface 0, opcode 3 - set txpower)
    // Byte 1: 0xFD (-3 in two's complement)
    const uint8_t frame[] = { 
        0x03, // Interface 0, opcode 3 (set txpower - RNODE_OPCODE_TXPOWER)
        0xFD, // Payload byte 0 (-3 in two's complement)
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);

    assert(last_callback_context == &test_context);

    assert_equal_bytes(frame, data, sizeof(frame));
}

// Test encoding a set spreading factor command and verify the output frame
static void test_pico_rnode_proto_command_set_spreading_factor(void) {

    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    status = pico_rnode_proto_command_set_spreading_factor(
        &encoder,
        1, // Interface 1 
        7 // Spreading factor of 7
    );
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for setting spreading factor on interface 1 to 7 is:
    // Byte 0: 0x14 (Interface 1, opcode 4 - set spreading factor)
    // Byte 1: 0x07 (Spreading factor value)
    const uint8_t frame[] = { 
        0x14, // Interface 1, opcode 4 (set spreading factor - RNODE_OPCODE_SPREADING_FACTOR)
        0x07, // Payload byte 0 (spreading factor value)
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);

    assert(last_callback_context == &test_context);

    assert_equal_bytes(frame, data, sizeof(frame));
}

// Test encoding a set coding rate command and verify the output frame
static void test_pico_rnode_proto_command_set_coding_rate(void) {

    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    status = pico_rnode_proto_command_set_coding_rate(
        &encoder,
        0, // Interface 0 
        5 // Coding rate of 4/5
    );
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for setting coding rate on interface 0 to 5 (4/5) is:
    // Byte 0: 0x05 (Interface 0, opcode 5 - set coding rate)
    // Byte 1: 0x05 (Coding rate value)
    const uint8_t frame[] = { 
        0x05, // Interface 0, opcode 5 (set coding rate - RNODE_OPCODE_CODING_RATE)
        0x05, // Payload byte 0 (coding rate value)
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);  
    assert(last_callback_context == &test_context);

    assert_equal_bytes(frame, data, sizeof(frame));
}

// Test encoding a set radio state command and verify the output frame
static void test_pico_rnode_proto_command_set_radio_state(void) {

    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    status = pico_rnode_proto_command_set_radio_state(
        &encoder,
        2, // Interface 2 
        1 // Radio state on
    );
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for setting radio state on interface 2 to on is:
    // Byte 0: 0x26 (Interface 2, opcode 6 - set radio state)
    // Byte 1: 0x01 (Radio state on)
    const uint8_t frame[] = { 
        0x26, // Interface 2, opcode 6 (set radio state - RNODE_OPCODE_RADIO_STATE)
        0x01, // Payload byte 0 (radio state on)
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);
    assert(last_callback_context == &test_context);
    assert_equal_bytes(frame, data, sizeof(frame));
}

// Test encoding a detect command and verify the output frame
static void test_pico_rnode_proto_command_detect(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    status = pico_rnode_proto_command_detect(&encoder);
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for the detect command on interface 0 is:
    // Byte 0: 0x08 (Interface 0, opcode 8 - detect)
    // Byte 1: 0x73 (Payload byte 0, detect request - RNODE_DETECT_REQ)
    const uint8_t frame[] = { 
        0x08, // Interface 0, opcode 8 (detect - RNODE_OPCODE_DETECT)
        0x73, // Detect request (RNODE_DETECT_REQ)
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);
    assert(last_callback_context == &test_context);
    assert_equal_bytes(frame, data, sizeof(frame));
}

// Test encoding a ready command and verify the output frame
static void test_pico_rnode_proto_command_ready(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    status = pico_rnode_proto_command_ready(&encoder);
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for the ready command on interface 0 is:
    // Byte 0: 0x0f (Interface 0, opcode 15 - ready)
    const uint8_t frame[] = { 
        0x0F, // Interface 0, opcode 15 (ready - RNODE_OPCODE_READY)
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);
    assert(last_callback_context == &test_context);
    assert_equal_bytes(frame, data, sizeof(frame));
}

static void test_pico_rnode_proto_command_leave(void) {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    status = pico_rnode_proto_command_leave(&encoder);
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for the leave command on interface 0 is:
    // Byte 0: 0x0a (Interface 0, opcode 10 - leave)
    const uint8_t frame[] = { 
        0x0A, // Interface 0, opcode 10 (leave - RNODE_OPCODE_LEAVE)
        0x00, // Payload byte 0 (reserved, set to 0)
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);
    assert(last_callback_context == &test_context);
    assert_equal_bytes(frame, data, sizeof(frame));
}

static void test_pico_rnode_proto_transmit() {
    pico_rnode_proto_command_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_command_encoder_init_test(&encoder);

    // Start a transmit command on interface 1
    status = pico_rnode_proto_command_start(&encoder, 1);
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // Send some data bytes
    uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    for (size_t i = 0; i < sizeof(payload); i++) {
        status = pico_rnode_proto_command_data(&encoder, payload[i]);
        assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    }

    // End the transmit command
    status = pico_rnode_proto_command_end(&encoder);
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for the transmit command on interface 1 with payload 0xDEADBEEF is:
    // Byte 0: 0x1D (Interface 1, opcode 0 - DATA)
    // Byte 1-4: 0xDEADBEEF (payload data)
    const uint8_t frame[] = { 
        0x10, // Interface 1, opcode 0 (DATA - RNODE_OPCODE_DATA)
        0xDE, // Payload byte 0
        0xAD, // Payload byte 1
        0xBE, // Payload byte 2
        0xEF, // Payload byte 3
    };

    assert(start_cb_count == 1);
    assert(put_cb_count == sizeof(frame));
    assert(end_cb_count == 1);
    assert(last_callback_context == &test_context);
    assert_equal_bytes(frame, data, sizeof(frame));
}

static void run_test(const char *name, void (*fn)(void)) {
    printf("[ RUN ] %s\n", name);
    clear_test_state();
    fn();
    printf("[ PASS ] %s\n", name);
}

void test_command_encoder(void) {
    run_test("command_set_frequency", test_pico_rnode_proto_command_set_frequency); 
    run_test("command_set_bandwidth", test_pico_rnode_proto_command_set_bandwidth);
    run_test("command_set_txpower", test_pico_rnode_proto_command_set_txpower);
    run_test("command_set_spreading_factor", test_pico_rnode_proto_command_set_spreading_factor);
    run_test("command_set_coding_rate", test_pico_rnode_proto_command_set_coding_rate);
    run_test("command_set_radio_state", test_pico_rnode_proto_command_set_radio_state);
    run_test("command_detect", test_pico_rnode_proto_command_detect);
    run_test("command_ready", test_pico_rnode_proto_command_ready);
    run_test("command_leave", test_pico_rnode_proto_command_leave);
    run_test("command_transmit", test_pico_rnode_proto_transmit);
    
    fprintf(stderr, "All command encoder tests passed!\n");
}
