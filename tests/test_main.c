// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

extern void test_command_decoder(void);
extern void test_command_encoder(void);

int main(void) {
//    test_command_decoder();
    test_command_encoder();
    return 0;
}
