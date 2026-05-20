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

static uint32_t frequency_cb_count = 0;
static uint32_t bandwidth_cb_count = 0;
static uint32_t txpower_cb_count = 0;

void reset_callback_counts(void) {
    frequency_cb_count = 0;
    bandwidth_cb_count = 0;
    txpower_cb_count = 0;
}

void pico_rnode_proto_cmd_set_frequency_cb_test(
    void * context,
    uint8_t interface,
    uint32_t frequency_hz
) {
    fprintf(stderr, "set_frequency_cb called with interface=%u, frequency_hz=%u\n", interface, frequency_hz);
    assert(interface == 1);
    assert(frequency_hz == 867252736);
    frequency_cb_count++;
}

void pico_rnode_proto_cmd_set_txpower_cb_test(
    void * context,
    uint8_t interface,
    int8_t dbm
) {
    fprintf(stderr, "set_txpower_cb called with interface=%u, dbm=%d\n", interface, dbm);
    assert(interface == 1);
    assert(dbm == 14);
    txpower_cb_count++;
}

void pico_rnode_proto_cmd_set_bandwidth_cb_test(
    void * context,
    uint8_t interface,
    uint32_t bandwidth
) {
    fprintf(stderr, "set_bandwidth_cb called with interface=%u, bandwidth=%u\n", interface, bandwidth);
    assert(interface == 1);
    assert(bandwidth == 867252736);
    bandwidth_cb_count++;
}

static void test_decoder_set_bandwidth(void) {
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_init(
        &decoder,
        NULL, // context
        pico_rnode_proto_cmd_set_frequency_cb_test, // set_frequency_cb
        pico_rnode_proto_cmd_set_bandwidth_cb_test, // set_bandwidth_cb
        pico_rnode_proto_cmd_set_txpower_cb_test, // set_txpower_cb
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

    pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_write(
        &decoder,
        frame,
        sizeof(frame)
    );

    assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);

    pico_rnode_proto_command_decoder_end(&decoder);

    assert(bandwidth_cb_count == 1);
    assert(frequency_cb_count == 0);
    assert(txpower_cb_count == 0);
}

static void test_decoder_set_frequency(void) {
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_init(
        &decoder,
        NULL, // context
        pico_rnode_proto_cmd_set_frequency_cb_test, // set_frequency_cb
        pico_rnode_proto_cmd_set_bandwidth_cb_test, // set_bandwidth_cb
        pico_rnode_proto_cmd_set_txpower_cb_test, // set_txpower_cb
        NULL, // tx_start_cb
        NULL, // tx_data_cb
        NULL, // tx_end_cb
        NULL  // tx_error_cb
    );

    // Set frequency command on interface 1 with frequency 867252736 Hz
    // Values are big-endian on the wire, so the payload bytes are reversed from the uint32_t literal
    const uint8_t frame[] = { 
        0x11, // Interface 1, opcode 1 (set frequency)
        0x33, // Payload byte 0 (MSB)
        0xB1, // Payload byte 1
        0x3A, // Payload byte 2
        0x00, // Payload byte 3 (LSB)
    };

    pico_rnode_proto_command_decoder_start(&decoder);

    pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_write(
        &decoder,
        frame,
        sizeof(frame)
    );

    assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);

    pico_rnode_proto_command_decoder_end(&decoder);

    assert(bandwidth_cb_count == 0);
    assert(frequency_cb_count == 1);
    assert(txpower_cb_count == 0);
}

static void test_decoder_set_txpower(void) {
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_init(
        &decoder,
        NULL, // context
        pico_rnode_proto_cmd_set_frequency_cb_test, // set_frequency_cb
        pico_rnode_proto_cmd_set_bandwidth_cb_test, // set_bandwidth_cb
        pico_rnode_proto_cmd_set_txpower_cb_test, // set_txpower_cb
        NULL, // tx_start_cb
        NULL, // tx_data_cb
        NULL, // tx_end_cb
        NULL  // tx_error_cb
    );

    // Set txpower command on interface 1 with txpower 14 dBm
    // Values are big-endian on the wire, so the payload bytes are reversed from the int8_t literal
    const uint8_t frame[] = { 
        0x13, // Interface 1, opcode 3 (set txpower)
        0x0E, // Payload byte 0 (MSB)
    };

    pico_rnode_proto_command_decoder_start(&decoder);

    pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_write(
        &decoder,
        frame,
        sizeof(frame)
    );

    assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);

    pico_rnode_proto_command_decoder_end(&decoder);

    assert(bandwidth_cb_count == 0);
    assert(frequency_cb_count == 0);
    assert(txpower_cb_count == 1);
}

static void run_test(const char *name, void (*fn)(void)) {
    printf("[ RUN ] %s\n", name);
    reset_callback_counts();
    fn();
    printf("[ PASS ] %s\n", name);
}

int main(void) {
    run_test("decoder_set_bandwidth", test_decoder_set_bandwidth);
    run_test("decoder_set_frequency", test_decoder_set_frequency);
    run_test("decoder_set_txpower", test_decoder_set_txpower);

    printf("All tests passed.\n");
    return 0;
}
