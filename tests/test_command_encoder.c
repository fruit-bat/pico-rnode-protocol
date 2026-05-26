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


    const uint8_t frame[] = { 
        0x21, // Interface 2, opcode 1 (set frequency)
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

static void run_test(const char *name, void (*fn)(void)) {
    printf("[ RUN ] %s\n", name);
    fn();
    printf("[ PASS ] %s\n", name);
}

void test_command_encoder(void) {
    run_test("command_set_frequency", test_pico_rnode_proto_command_set_frequency); 

    fprintf(stderr, "All command encoder tests passed!\n");
}
