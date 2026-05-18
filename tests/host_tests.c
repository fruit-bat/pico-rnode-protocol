// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pico-rnode-protocol.h"

static void assert_equal_bytes(const uint8_t *actual, const uint8_t *expected, size_t len) {
    if (memcmp(actual, expected, len) != 0) {
        fprintf(stderr, "Byte arrays differ:\n");
        fprintf(stderr, "  expected:");
        for (size_t i = 0; i < len; i++) {
            fprintf(stderr, " %02X", expected[i]);
        }
        fprintf(stderr, "\n  actual:  ");
        for (size_t i = 0; i < len; i++) {
            fprintf(stderr, " %02X", actual[i]);
        }
        fprintf(stderr, "\n");
        assert(0);
    }
}

void pico_rnode_proto_cmd_set_bandwidth_cb_test(
    void * context,
    uint8_t interface,
    uint32_t bandwidth
) {
    fprintf(stderr, "set_bandwidth_cb called with interface=%u, bandwidth=%lu\n", interface, bandwidth);
    assert(interface == 1);
    assert(bandwidth == 867252736);
}

static void test_encoder_simple(void) {
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_init(
        &decoder,
        NULL, // context
        NULL, // set_frequency_cb
        pico_rnode_proto_cmd_set_bandwidth_cb_test, // set_bandwidth_cb
        NULL, // set_txpower_cb
        NULL, // tx_start_cb
        NULL, // tx_data_cb
        NULL, // tx_end_cb
        NULL  // tx_error_cb
    );

    // Set bandwidth command on interface 1 with bandwidth 867252736 Hz
    // Values are big-endian on the wire, so the payload bytes are reversed from the uint32_t literal
    const uint8_t frame[] = { 
        0x12, // Interface 1, opcode 2 (set bandwidth)
        0x33, // Payload byte 0 (MSB)
        0xB1, // Payload byte 1
        0x3A, // Payload byte 2
        0x00, // Payload byte 3 (LSB)
    };

    pico_rnode_proto_command_decoder_start(&decoder);

    for (size_t i = 0; i < sizeof(frame); i++) {
        pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_put(
            &decoder,
            frame[i]
        );
        assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    }

    pico_rnode_proto_command_decoder_end(&decoder);
}


static void run_test(const char *name, void (*fn)(void)) {
    printf("[ RUN ] %s\n", name);
    fn();
    printf("[ PASS ] %s\n", name);
}

int main(void) {
    run_test("encoder_simple", test_encoder_simple);
    printf("All tests passed.\n");
    return 0;
}
