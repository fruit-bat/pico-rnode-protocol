// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

extern void test_command_decoder(void);
extern void test_command_encoder(void);
extern void test_command_round_trip(void);

static void usage(const char *program_name) {
    printf("Usage: %s [all|encoder|decoder|round_trip]\n", program_name);
}

int main(int argc, char **argv) {
    const char *mode = "all";
    if (argc > 2) {
        usage(argv[0]);
        return 1;
    }
    if (argc == 2) {
        mode = argv[1];
    }

    if (strcmp(mode, "all") == 0) {
        test_command_encoder();
        test_command_decoder();
        test_command_round_trip();
    } else if (strcmp(mode, "encoder") == 0) {
        test_command_encoder();
    } else if (strcmp(mode, "decoder") == 0) {
        test_command_decoder();
    } else if (strcmp(mode, "round_trip") == 0) {
        test_command_round_trip();
    } else {
        usage(argv[0]);
        return 1;
    }

    return 0;
}
