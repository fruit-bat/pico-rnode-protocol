// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pico-rnode-protocol-event-encoder.h"
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

static void pico_rnode_proto_event_encoder_init_test(
    pico_rnode_proto_event_encoder_t *encoder
) {
    pico_rnode_proto_event_encoder_init(
        encoder,
        &test_context,
        pico_rnode_proto_frame_start_cb_test,
        pico_rnode_proto_frame_data_cb_test,
        pico_rnode_proto_frame_end_cb_test
    );
}

static void test_pico_rnode_proto_receive() {
    pico_rnode_proto_event_encoder_t encoder = {0};
    pico_rnode_proto_encoder_status_t status;

    pico_rnode_proto_event_encoder_init_test(&encoder);

    // Start a data command on interface 1
    status = pico_rnode_proto_event_start(&encoder, 1);
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // Send some data bytes
    uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    for (size_t i = 0; i < sizeof(payload); i++) {
        status = pico_rnode_proto_event_data(&encoder, payload[i]);
        assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    }

    // End the data command
    status = pico_rnode_proto_event_end(&encoder);
    assert(status == PICO_RNODE_PROTO_ENCODER_STATUS_OK);

    // The expected frame for the receive command on interface 1 with payload 0xDEADBEEF is:
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

void test_event_encoder(void) {
    run_test("event_receive", test_pico_rnode_proto_receive);
    
    fprintf(stderr, "All event encoder tests passed!\n");
}
