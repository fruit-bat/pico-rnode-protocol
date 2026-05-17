// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pico-rnode-protocol.h"


typedef struct {
    uint8_t *buffer;
    size_t len;
} encoder_capture_t;

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

static void test_encoder_simple(void) {
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
