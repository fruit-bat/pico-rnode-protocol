// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "pico-rnode-protocol-command-decoder.h"
#include "pico-rnode-protocol-command-decoder-text.h"
#include "test_utils.h"

typedef struct {
    char buffer[4096];
    size_t pos;
} text_capture_t;

static text_capture_t capture;

static void capture_reset(void) {
    memset(capture.buffer, 0, sizeof(capture.buffer));
    capture.pos = 0;
}

static int capture_contains(const char *substr) {
    return strstr(capture.buffer, substr) != NULL;
}

static void test_decoder_text_init_default_stdout(void) {
    fprintf(stderr, "TEST: test_decoder_text_init_default_stdout\n");
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, NULL);
    
    assert(text_decoder.out == stdout);
    fprintf(stderr, "PASS\n");
}

static void test_decoder_text_init_custom_stream(void) {
    fprintf(stderr, "TEST: test_decoder_text_init_custom_stream\n");
    
    FILE *custom_stream = fopen("/tmp/test_output.txt", "w");
    assert(custom_stream != NULL);
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, custom_stream);
    
    assert(text_decoder.out == custom_stream);
    
    fclose(custom_stream);
    fprintf(stderr, "PASS\n");
}

static void test_decoder_text_ready_command(void) {
    fprintf(stderr, "TEST: test_decoder_text_ready_command\n");
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    FILE *out = tmpfile();
    assert(out != NULL);
    
    capture_reset();
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, out);
    
    uint8_t ready_frame[] = {0x0F};
    
    pico_rnode_proto_command_decoder_start(&decoder);
    for (size_t i = 0; i < sizeof(ready_frame); i++) {
        pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_put(&decoder, ready_frame[i]);
        assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    }
    pico_rnode_proto_decoder_status_t final_status = pico_rnode_proto_command_decoder_end(&decoder);
    assert(final_status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    
    rewind(out);
    size_t bytes_read = fread(capture.buffer, 1, sizeof(capture.buffer) - 1, out);
    capture.buffer[bytes_read] = '\0';
    fclose(out);
    
    fprintf(stderr, "Output: %s\n", capture.buffer);
    assert(capture_contains("READY"));
    
    fprintf(stderr, "PASS\n");
}

static void test_decoder_text_detect_command(void) {
    fprintf(stderr, "TEST: test_decoder_text_detect_command\n");
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    FILE *out = tmpfile();
    assert(out != NULL);
    
    capture_reset();
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, out);
    
    uint8_t detect_frame[] = {0x18, 0x73};
    
    pico_rnode_proto_command_decoder_start(&decoder);
    for (size_t i = 0; i < sizeof(detect_frame); i++) {
        pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_put(&decoder, detect_frame[i]);
        assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    }
    pico_rnode_proto_decoder_status_t final_status = pico_rnode_proto_command_decoder_end(&decoder);
    assert(final_status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    
    rewind(out);
    size_t bytes_read = fread(capture.buffer, 1, sizeof(capture.buffer) - 1, out);
    capture.buffer[bytes_read] = '\0';
    fclose(out);
    
    fprintf(stderr, "Output: %s\n", capture.buffer);
    assert(capture_contains("DETECT"));
    
    fprintf(stderr, "PASS\n");
}

static void test_decoder_text_frequency_command(void) {
    fprintf(stderr, "TEST: test_decoder_text_frequency_command\n");
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    FILE *out = tmpfile();
    assert(out != NULL);
    
    capture_reset();
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, out);
    
    uint8_t freq_frame[] = {0x11, 0x33, 0xC0, 0x02, 0x00};
    
    pico_rnode_proto_command_decoder_start(&decoder);
    for (size_t i = 0; i < sizeof(freq_frame); i++) {
        pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_put(&decoder, freq_frame[i]);
        assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    }
    pico_rnode_proto_decoder_status_t final_status = pico_rnode_proto_command_decoder_end(&decoder);
    assert(final_status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    
    rewind(out);
    size_t bytes_read = fread(capture.buffer, 1, sizeof(capture.buffer) - 1, out);
    capture.buffer[bytes_read] = '\0';
    fclose(out);
    
    fprintf(stderr, "Output: %s\n", capture.buffer);
    assert(capture_contains("FREQUENCY"));
    assert(capture_contains("interface=1"));
    assert(capture_contains("hz=868221440"));
    
    fprintf(stderr, "PASS\n");
}

static void test_decoder_text_tx_data_stream(void) {
    fprintf(stderr, "TEST: test_decoder_text_tx_data_stream\n");
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    FILE *out = tmpfile();
    assert(out != NULL);
    
    capture_reset();
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, out);
    
    uint8_t tx_frame[] = {0x20, 'H', 'i'};
    
    pico_rnode_proto_command_decoder_start(&decoder);
    for (size_t i = 0; i < sizeof(tx_frame); i++) {
        pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_put(&decoder, tx_frame[i]);
        assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    }
    pico_rnode_proto_decoder_status_t final_status = pico_rnode_proto_command_decoder_end(&decoder);
    assert(final_status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    
    rewind(out);
    size_t bytes_read = fread(capture.buffer, 1, sizeof(capture.buffer) - 1, out);
    capture.buffer[bytes_read] = '\0';
    fclose(out);
    
    fprintf(stderr, "Output: %s\n", capture.buffer);
    assert(capture_contains("TX START"));
    assert(capture_contains("interface=2"));
    assert(capture_contains("TX DATA"));
    assert(capture_contains("TX END"));
    
    fprintf(stderr, "PASS\n");
}

static void test_decoder_text_invalid_opcode(void) {
    fprintf(stderr, "TEST: test_decoder_text_invalid_opcode\n");
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    FILE *out = tmpfile();
    assert(out != NULL);
    
    capture_reset();
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, out);
    
    uint8_t invalid_frame[] = {0x1E};
    
    pico_rnode_proto_command_decoder_start(&decoder);
    for (size_t i = 0; i < sizeof(invalid_frame); i++) {
        pico_rnode_proto_command_decoder_put(&decoder, invalid_frame[i]);
    }
    pico_rnode_proto_command_decoder_end(&decoder);
    
    rewind(out);
    size_t bytes_read = fread(capture.buffer, 1, sizeof(capture.buffer) - 1, out);
    capture.buffer[bytes_read] = '\0';
    fclose(out);
    
    fprintf(stderr, "Output: %s\n", capture.buffer);
    assert(capture_contains("ERROR"));
    
    fprintf(stderr, "PASS\n");
}

static void test_decoder_text_radio_state_command(void) {
    fprintf(stderr, "TEST: test_decoder_text_radio_state_command\n");
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    FILE *out = tmpfile();
    assert(out != NULL);
    
    capture_reset();
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, out);
    
    uint8_t radio_state_frame[] = {0x36, RNODE_RADIO_STATE_ON};
    
    pico_rnode_proto_command_decoder_start(&decoder);
    for (size_t i = 0; i < sizeof(radio_state_frame); i++) {
        pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_put(&decoder, radio_state_frame[i]);
        assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    }
    pico_rnode_proto_decoder_status_t final_status = pico_rnode_proto_command_decoder_end(&decoder);
    assert(final_status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    
    rewind(out);
    size_t bytes_read = fread(capture.buffer, 1, sizeof(capture.buffer) - 1, out);
    capture.buffer[bytes_read] = '\0';
    fclose(out);
    
    fprintf(stderr, "Output: %s\n", capture.buffer);
    assert(capture_contains("RADIO_STATE"));
    assert(capture_contains("interface=3"));
    assert(capture_contains("state=ON"));
    
    fprintf(stderr, "PASS\n");
}

static void test_decoder_text_spreading_factor_command(void) {
    fprintf(stderr, "TEST: test_decoder_text_spreading_factor_command\n");
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    FILE *out = tmpfile();
    assert(out != NULL);
    
    capture_reset();
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, out);
    
    uint8_t sf_frame[] = {0x14, 9};
    
    pico_rnode_proto_command_decoder_start(&decoder);
    for (size_t i = 0; i < sizeof(sf_frame); i++) {
        pico_rnode_proto_decoder_status_t status = pico_rnode_proto_command_decoder_put(&decoder, sf_frame[i]);
        assert(status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    }
    pico_rnode_proto_decoder_status_t final_status = pico_rnode_proto_command_decoder_end(&decoder);
    assert(final_status == PICO_RNODE_PROTO_DECODER_STATUS_OK);
    
    rewind(out);
    size_t bytes_read = fread(capture.buffer, 1, sizeof(capture.buffer) - 1, out);
    capture.buffer[bytes_read] = '\0';
    fclose(out);
    
    fprintf(stderr, "Output: %s\n", capture.buffer);
    assert(capture_contains("SF"));
    assert(capture_contains("interface=1"));
    assert(capture_contains("sf=9"));
    
    fprintf(stderr, "PASS\n");
}

static void test_decoder_text_multiple_commands(void) {
    fprintf(stderr, "TEST: test_decoder_text_multiple_commands\n");
    
    pico_rnode_proto_command_decoder_t decoder = {0};
    pico_rnode_proto_command_decoder_text_t text_decoder = {0};
    
    FILE *out = tmpfile();
    assert(out != NULL);
    
    capture_reset();
    
    pico_rnode_proto_command_decoder_text_init(&text_decoder, &decoder, out);
    
    uint8_t ready_frame[] = {0x0F};
    pico_rnode_proto_command_decoder_start(&decoder);
    for (size_t i = 0; i < sizeof(ready_frame); i++) {
        pico_rnode_proto_command_decoder_put(&decoder, ready_frame[i]);
    }
    pico_rnode_proto_command_decoder_end(&decoder);
    
    uint8_t detect_frame[] = {0x18, 0x73};
    pico_rnode_proto_command_decoder_start(&decoder);
    for (size_t i = 0; i < sizeof(detect_frame); i++) {
        pico_rnode_proto_command_decoder_put(&decoder, detect_frame[i]);
    }
    pico_rnode_proto_command_decoder_end(&decoder);
    
    rewind(out);
    size_t bytes_read = fread(capture.buffer, 1, sizeof(capture.buffer) - 1, out);
    capture.buffer[bytes_read] = '\0';
    fclose(out);
    
    fprintf(stderr, "Output: %s\n", capture.buffer);
    assert(capture_contains("READY"));
    assert(capture_contains("DETECT"));
    
    fprintf(stderr, "PASS\n");
}

void test_command_decoder_text(void) {
    fprintf(stderr, "\n=== Text Decoder Tests ===\n");
    
    test_decoder_text_init_default_stdout();
    test_decoder_text_init_custom_stream();
    test_decoder_text_ready_command();
    test_decoder_text_detect_command();
    test_decoder_text_frequency_command();
    test_decoder_text_tx_data_stream();
    test_decoder_text_invalid_opcode();
    test_decoder_text_radio_state_command();
    test_decoder_text_spreading_factor_command();
    test_decoder_text_multiple_commands();
    
    fprintf(stderr, "=== Text Decoder Tests Passed ===\n");
}
