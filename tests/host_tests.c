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

static uint32_t detect_cb_count = 0;
static uint32_t frequency_cb_count = 0;
static uint32_t bandwidth_cb_count = 0;
static uint32_t txpower_cb_count = 0;
static uint32_t coding_rate_cb_count = 0;
static uint32_t spreading_factor_cb_count = 0;
static uint32_t radio_state_cb_count = 0;
static uint32_t ready_cb_count = 0;
static uint32_t leave_cb_count = 0;
static uint32_t lock_cb_count = 0;
static uint32_t tx_start_cb_count = 0;
static uint32_t tx_data_cb_count = 0;
static uint32_t tx_end_cb_count = 0;
static uint32_t tx_error_cb_count = 0;

static uint8_t data[9] = {0};
static void* error_context = NULL;
static uint8_t error_interface = 0;
static uint8_t error_opcode = 0;
static uint32_t error_index = 0;
static pico_rnode_proto_decoder_status_t error_status = 0;

void reset_callback_counts(void) {
    detect_cb_count = 0;
    frequency_cb_count = 0;
    bandwidth_cb_count = 0;
    txpower_cb_count = 0;
    coding_rate_cb_count = 0;
    spreading_factor_cb_count = 0;
    radio_state_cb_count = 0;
    ready_cb_count = 0;
    leave_cb_count = 0;
    lock_cb_count = 0;
    tx_start_cb_count = 0;
    tx_data_cb_count = 0;
    tx_end_cb_count = 0;
    tx_error_cb_count = 0;
    memset(data, 0, sizeof(data));
}

void pico_rnode_proto_decoder_error_cb_test(
    void * context,
    uint8_t interface,
    uint8_t opcode,
    uint32_t index,
    pico_rnode_proto_decoder_status_t status
) {
    error_context = context;
    error_interface = interface;
    error_opcode = opcode;
    error_index = index;
    error_status = status;
    fprintf(stderr, "error_cb called with interface=%u, opcode=%u, index=%u, status=%d\n", interface, opcode, index, status);
    tx_error_cb_count++;
}

void pico_rnode_proto_command_set_frequency_cb_test(
    void * context,
    uint8_t interface,
    uint32_t frequency_hz
) {
    fprintf(stderr, "set_frequency_cb called with interface=%u, frequency_hz=%u\n", interface, frequency_hz);
    assert(interface == 1);
    assert(frequency_hz == 867252736);
    frequency_cb_count++;
}

void pico_rnode_proto_command_set_txpower_cb_test(
    void * context,
    uint8_t interface,
    int8_t dbm
) {
    fprintf(stderr, "set_txpower_cb called with interface=%u, dbm=%d\n", interface, dbm);
    assert(interface == 1);
    assert(dbm == 14);
    txpower_cb_count++;
}

void pico_rnode_proto_command_detect_cb_test(
    void * context
) {
    fprintf(stderr, "detect_cb called\n");
    detect_cb_count++;
}

void pico_rnode_proto_command_set_spreading_factor_cb_test(
    void * context,
    uint8_t interface,
    uint8_t spreading_factor
) {
    fprintf(stderr, "set_spreading_factor_cb called with interface=%u, spreading_factor=%u\n", interface, spreading_factor);
    assert(interface == 1);
    assert(spreading_factor == 7);
    spreading_factor_cb_count++;
}

void pico_rnode_proto_command_set_coding_rate_cb_test(
    void * context,
    uint8_t interface,
    uint8_t coding_rate
) {
    fprintf(stderr, "set_coding_rate_cb called with interface=%u, coding_rate=%u\n", interface, coding_rate);
    assert(interface == 1);
    assert(coding_rate == 5);
    coding_rate_cb_count++;
}

void pico_rnode_proto_command_set_radio_state_cb_test(
    void * context,
    uint8_t interface,
    pico_rnode_proto_radio_state_t state // radio state, for LoRa radios (typically 0-2)
) {
    fprintf(stderr, "set_radio_state_cb called with interface=%u, state=%u\n", interface, state);
    assert(interface == 1);
    assert(state == RNODE_RADIO_STATE_ON);
    radio_state_cb_count++;
}

void pico_rnode_proto_command_ready_cb_test(
    void * context
) {
    fprintf(stderr, "ready_cb called\n");
    ready_cb_count++;
}

void pico_rnode_proto_command_leave_cb_test(
    void * context
) {
    fprintf(stderr, "leave_cb called\n");
    leave_cb_count++;
}

void pico_rnode_proto_command_lock_cb_test(
    void * context
) {
    fprintf(stderr, "lock_cb called\n");
    lock_cb_count++;
}

void pico_rnode_proto_command_tx_start_cb_test(
    void * context,
    uint8_t interface
) {
    fprintf(stderr, "tx_start_cb called with interface=%u\n", interface);
    assert(interface == 1);
    tx_start_cb_count++;
}

pico_rnode_proto_frame_cb_status_t pico_rnode_proto_command_tx_data_cb_test(
    void * context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
) {
    fprintf(stderr, "tx_data_cb called with interface=%u, byte=%u, byte_index=%u\n", interface, byte, byte_index);
    assert(interface == 1);
    if (byte == '#') {
        fprintf(stderr, "Simulating error condition on byte '#'\n");
        return PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT;
    }
    if (byte_index >= sizeof(data)) {
        fprintf(stderr, "Error: byte_index %u out of bounds\n", byte_index);
        return PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT;
    }
    data[byte_index] = byte;
    tx_data_cb_count++;
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

void pico_rnode_proto_command_tx_end_cb_test(
    void * context,
    uint8_t interface,
    uint32_t len
) {
    fprintf(stderr, "tx_end_cb called with interface=%u, len=%u\n", interface, len);
    assert(interface == 1);
    tx_end_cb_count++;
}

void pico_rnode_proto_command_set_bandwidth_cb_test(
    void * context,
    uint8_t interface,
    uint32_t bandwidth
) {
    fprintf(stderr, "set_bandwidth_cb called with interface=%u, bandwidth=%u\n", interface, bandwidth);
    assert(interface == 1);
    assert(bandwidth == 867252736);
    bandwidth_cb_count++;
}

static void init_test_decoder(
    pico_rnode_proto_command_decoder_t *decoder,
    void * context
) {
    pico_rnode_proto_command_decoder_init(
        decoder,
        context,
        pico_rnode_proto_command_detect_cb_test, // detect_cb
        pico_rnode_proto_command_set_frequency_cb_test, // set_frequency_cb
        pico_rnode_proto_command_set_bandwidth_cb_test, // set_bandwidth_cb
        pico_rnode_proto_command_set_txpower_cb_test, // set_txpower_cb
        pico_rnode_proto_command_set_spreading_factor_cb_test, // set_spreading_factor_cb
        pico_rnode_proto_command_set_coding_rate_cb_test, // set_coding_rate_cb
        pico_rnode_proto_command_set_radio_state_cb_test, // set_radio_state_cb
        pico_rnode_proto_command_ready_cb_test, // ready_cb
        pico_rnode_proto_command_lock_cb_test, // lock_cb
        pico_rnode_proto_command_leave_cb_test, // leave_cb
        pico_rnode_proto_command_tx_start_cb_test, // tx_start_cb
        pico_rnode_proto_command_tx_data_cb_test, // tx_data_cb
        pico_rnode_proto_command_tx_end_cb_test, // tx_end_cb
        pico_rnode_proto_decoder_error_cb_test  // error_cb
    );
}



static void test_decoder_invalid_opcode(void) {
    pico_rnode_proto_command_decoder_t decoder = {0};
    init_test_decoder(&decoder, NULL);

    // Invalid opcode 0x0F on interface 1
    const uint8_t frame[] = { 
        0x19, // Interface 1, opcode 9 (invalid)
        0x00, // Payload byte 0
    };

    pico_rnode_proto_command_decoder_start(&decoder);

    pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_write(
        &decoder,
        frame,
        sizeof(frame)
    );

    assert(status == PICO_RNODE_PROTO_DECODER_STATUS_UNKNOWN_OPCODE);

    pico_rnode_proto_command_decoder_end(&decoder);

    assert(detect_cb_count == 0);
    assert(frequency_cb_count == 0);
    assert(bandwidth_cb_count == 0);
    assert(txpower_cb_count == 0);
    assert(coding_rate_cb_count == 0);
    assert(spreading_factor_cb_count == 0);
    assert(radio_state_cb_count == 0);
    assert(ready_cb_count == 0);
    assert(leave_cb_count == 0);
    assert(lock_cb_count == 0);
    assert(tx_start_cb_count == 0);
    assert(tx_data_cb_count == 0);
    assert(tx_end_cb_count == 0);
    assert(tx_error_cb_count == 1); 
}

static void test_decoder_set_bandwidth(void) {
    pico_rnode_proto_command_decoder_t decoder = {0};
    init_test_decoder(&decoder, NULL);

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

    assert(detect_cb_count == 0);
    assert(frequency_cb_count == 0);
    assert(bandwidth_cb_count == 1);
    assert(txpower_cb_count == 0);
    assert(coding_rate_cb_count == 0);
    assert(spreading_factor_cb_count == 0);
    assert(radio_state_cb_count == 0);
    assert(ready_cb_count == 0);
    assert(leave_cb_count == 0);
    assert(lock_cb_count == 0);
    assert(tx_start_cb_count == 0);
    assert(tx_data_cb_count == 0);
    assert(tx_end_cb_count == 0);
    assert(tx_error_cb_count == 0); 
}

static void test_decoder_set_frequency(void) {
    pico_rnode_proto_command_decoder_t decoder = {0};
    init_test_decoder(&decoder, NULL);

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

    assert(detect_cb_count == 0);
    assert(frequency_cb_count == 1);
    assert(bandwidth_cb_count == 0);
    assert(txpower_cb_count == 0);
    assert(coding_rate_cb_count == 0);
    assert(spreading_factor_cb_count == 0);
    assert(radio_state_cb_count == 0);
    assert(ready_cb_count == 0);
    assert(leave_cb_count == 0);
    assert(lock_cb_count == 0);
    assert(tx_start_cb_count == 0);
    assert(tx_data_cb_count == 0);
    assert(tx_end_cb_count == 0);
    assert(tx_error_cb_count == 0); 
}

static void test_decoder_set_txpower(void) {
    pico_rnode_proto_command_decoder_t decoder = {0};
    init_test_decoder(&decoder, NULL);

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

    assert(detect_cb_count == 0);
    assert(frequency_cb_count == 0);
    assert(bandwidth_cb_count == 0);
    assert(txpower_cb_count == 1);
    assert(coding_rate_cb_count == 0);
    assert(spreading_factor_cb_count == 0);
    assert(radio_state_cb_count == 0);
    assert(ready_cb_count == 0);
    assert(leave_cb_count == 0);
    assert(lock_cb_count == 0);
    assert(tx_start_cb_count == 0);
    assert(tx_data_cb_count == 0);
    assert(tx_end_cb_count == 0);
    assert(tx_error_cb_count == 0); 

}

static void test_decoder_transmit(void) {
    pico_rnode_proto_command_decoder_t decoder = {0};
    init_test_decoder(&decoder, NULL);

    // Set frequency command on interface 1 with frequency 867252736 Hz
    // Values are big-endian on the wire, so the payload bytes are reversed from the uint32_t literal
    const uint8_t frame_data[] = { 
        'h', // Payload byte 0
        'e', // Payload byte 1
        'l', // Payload byte 2
        'l', // Payload byte 3
        'o', // Payload byte 4
    };

    pico_rnode_proto_command_decoder_start(&decoder);

    pico_rnode_proto_decoder_status_t status1 = pico_rnode_proto_command_decoder_put(
        &decoder,
        0x10 // Interface 1, opcode 0 (transmit)
    );
    assert(status1 == PICO_RNODE_PROTO_DECODER_STATUS_OK);

    assert(tx_start_cb_count == 1);
    assert(tx_data_cb_count == 0);
    assert(tx_end_cb_count == 0);
    assert(tx_error_cb_count == 0);

    pico_rnode_proto_decoder_status_t status2 = pico_rnode_proto_command_decoder_write(
        &decoder,
        frame_data,
        sizeof(frame_data)
    );
    assert(status2 == PICO_RNODE_PROTO_DECODER_STATUS_OK);

    assert(tx_start_cb_count == 1);
    assert(tx_data_cb_count == 5);
    assert(tx_end_cb_count == 0);
    assert(tx_error_cb_count == 0);

    pico_rnode_proto_decoder_status_t status3 = pico_rnode_proto_command_decoder_end(&decoder);
    assert(status3 == PICO_RNODE_PROTO_DECODER_STATUS_OK);

    assert(detect_cb_count == 0);
    assert(frequency_cb_count == 0);
    assert(bandwidth_cb_count == 0);
    assert(txpower_cb_count == 0);
    assert(coding_rate_cb_count == 0);
    assert(spreading_factor_cb_count == 0);
    assert(radio_state_cb_count == 0);
    assert(ready_cb_count == 0);
    assert(leave_cb_count == 0);
    assert(lock_cb_count == 0);
    assert(tx_start_cb_count == 1);
    assert(tx_data_cb_count == 5);
    assert(tx_end_cb_count == 1);
    assert(tx_error_cb_count == 0); 

    assert_equal_bytes(data, (const uint8_t*)"hello", 5);
}

static void test_decoder_transmit_abort(void) {
    static uint32_t test_context = 0xDEADBEEF;
    pico_rnode_proto_command_decoder_t decoder = {0};
    init_test_decoder(&decoder, &test_context);
    
    // Set frequency command on interface 1 with frequency 867252736 Hz
    // Values are big-endian on the wire, so the payload bytes are reversed from the uint32_t literal
    const uint8_t data_head[] = {
        0x10, // Interface 1, opcode 0 (transmit) 
        'h', // Payload byte 0
        'e', // Payload byte 1
    };
    const uint8_t data_tail[] = {
        '#', // Payload byte 2
        'l', // Payload byte 3
        'o', // Payload byte 4
    };

    pico_rnode_proto_command_decoder_start(&decoder);

    pico_rnode_proto_decoder_status_t status1 = pico_rnode_proto_command_decoder_write(
        &decoder,
        data_head,
        sizeof(data_head)
    );
    assert(status1 == PICO_RNODE_PROTO_DECODER_STATUS_OK);

    assert(tx_start_cb_count == 1);
    assert(tx_data_cb_count == 2);
    assert(tx_end_cb_count == 0);
    assert(tx_error_cb_count == 0);

    pico_rnode_proto_decoder_status_t status2 = pico_rnode_proto_command_decoder_write(
        &decoder,
        data_tail,
        sizeof(data_tail)
    );
    assert(status2 == PICO_RNODE_PROTO_DECODER_STATUS_ABORTED);

    assert(tx_start_cb_count == 1);
    assert(tx_data_cb_count == 2); // Data callback should not be called for bytes after the error
    assert(tx_end_cb_count == 0);
    assert(tx_error_cb_count == 1);

    pico_rnode_proto_decoder_status_t status3 = pico_rnode_proto_command_decoder_end(&decoder);
    assert(status3 == PICO_RNODE_PROTO_DECODER_STATUS_ABORTED);

    assert(detect_cb_count == 0);
    assert(frequency_cb_count == 0);
    assert(bandwidth_cb_count == 0);
    assert(txpower_cb_count == 0);
    assert(coding_rate_cb_count == 0);
    assert(spreading_factor_cb_count == 0);
    assert(radio_state_cb_count == 0);
    assert(ready_cb_count == 0);
    assert(leave_cb_count == 0);
    assert(lock_cb_count == 0);
    assert(tx_start_cb_count == 1);
    assert(tx_data_cb_count == 2); // Data callback should not be called for bytes after the error  
    assert(tx_end_cb_count == 0); // End callback should not be called since transmission was aborted
    assert(tx_error_cb_count == 1);
    assert(error_context == &test_context);
    assert(error_interface == 1);
    assert(error_opcode == 0);
    assert(error_index == 2); // Error should occur on the byte with value '#'


    assert(error_status == PICO_RNODE_PROTO_DECODER_STATUS_ABORTED);
}

static void test_decoder_set_frequency_error(void) {
    static uint32_t test_context = 0xDEADBEEF;
    pico_rnode_proto_command_decoder_t decoder = {0};
    init_test_decoder(&decoder, &test_context);

    // Set frequency command on interface 1 with frequency 867252736 Hz
    // Values are big-endian on the wire, so the payload bytes are reversed from the uint32_t literal
    const uint8_t frame[] = { 
        0x11, // Interface 1, opcode 1 (set frequency)
        0x33, // Payload byte 0 (MSB)
        0xB1, // Payload byte 1
        0x3A, // Payload byte 2
    }; // Missing payload byte 3 (LSB)

    pico_rnode_proto_command_decoder_start(&decoder);

    pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_write(
        &decoder,
        frame,
        sizeof(frame)
    );

    assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);

    pico_rnode_proto_command_decoder_end(&decoder);

    assert(detect_cb_count == 0);
    assert(frequency_cb_count == 0); // Callback should not be called due to error
    assert(bandwidth_cb_count == 0);
    assert(txpower_cb_count == 0);
    assert(coding_rate_cb_count == 0);
    assert(spreading_factor_cb_count == 0);
    assert(radio_state_cb_count == 0);
    assert(ready_cb_count == 0);
    assert(leave_cb_count == 0);
    assert(lock_cb_count == 0);
    assert(tx_start_cb_count == 0);
    assert(tx_data_cb_count == 0);
    assert(tx_end_cb_count == 0);
    assert(tx_error_cb_count == 1);
    assert(error_interface == 1);
    assert(error_opcode == 1);
    assert(error_index == 3); // Error should occur on the missing byte index
    assert(error_status == PICO_RNODE_PROTO_DECODER_STATUS_INVALID_LENGTH);
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
    run_test("decoder_transmit", test_decoder_transmit);
    run_test("decoder_transmit_abort", test_decoder_transmit_abort);
    run_test("decoder_set_frequency_error", test_decoder_set_frequency_error);
    run_test("decoder_invalid_opcode", test_decoder_invalid_opcode);

    printf("All tests passed.\n");
    return 0;
}
