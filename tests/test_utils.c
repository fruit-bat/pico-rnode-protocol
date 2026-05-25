// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "test_utils.h"

void assert_equal_bytes(const uint8_t *actual, const uint8_t *expected, size_t len) {
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