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

pico_rnode_proto_frame_cb_status_t pico_rnode_proto_cmd_start_cb_test(
    void * context
) {
    start_cb_count++;
    last_callback_context = context;
    data_index = 0;
    fprintf(stderr, "cmd_start_cb called\n");
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

pico_rnode_proto_frame_cb_status_t pico_rnode_proto_cmd_put_cb_test(
    void * context,
    uint8_t byte
) {
    put_cb_count++;
    last_callback_context = context;
    data[data_index++] = byte;
    fprintf(stderr, "cmd_put_cb called with byte: 0x%02X\n", byte);
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

pico_rnode_proto_frame_cb_status_t pico_rnode_proto_cmd_end_cb_test(
    void * context
) {
    end_cb_count++;
    last_callback_context = context;
    fprintf(stderr, "cmd_end_cb called\n");
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

void pico_rnode_proto_command_encoder_init_test(
    pico_rnode_proto_command_encoder_t *encoder
) {
    pico_rnode_proto_command_encoder_init(
        encoder,
        &test_context,
        pico_rnode_proto_cmd_start_cb_test,
        pico_rnode_proto_cmd_put_cb_test,
        pico_rnode_proto_cmd_end_cb_test
    );
}

// Test encoding a set frequency command and verify the output frame
void test_pico_rnode_proto_command_set_frequency(void) {

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
void test_pico_rnode_proto_command_set_bandwidth(void) {

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

static void run_test(const char *name, void (*fn)(void)) {
    printf("[ RUN ] %s\n", name);
    clear_test_state();
    fn();
    printf("[ PASS ] %s\n", name);
}

void test_command_encoder(void) {
    run_test("command_set_frequency", test_pico_rnode_proto_command_set_frequency); 
    run_test("command_set_bandwidth", test_pico_rnode_proto_command_set_bandwidth);

    fprintf(stderr, "All command encoder tests passed!\n");
}
