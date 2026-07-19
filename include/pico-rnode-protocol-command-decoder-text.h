// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "pico-rnode-protocol-command-decoder.h"

/**
 * Text-mode command decoder context.
 *
 * This wrapper holds a FILE stream and an internal command decoder instance.
 */
typedef struct {
    FILE *out; /**< Output stream for human-readable decoded messages. */
    const char *prefix; /**< Optional prefix printed before each message. */
} pico_rnode_proto_command_decoder_text_t;

/**
 * Initialize a text-mode command decoder.
 *
 * The provided output stream is used for all decoded message output. If
 * `out` is NULL, `stdout` is used.
 */
void pico_rnode_proto_command_decoder_text_init(
    pico_rnode_proto_command_decoder_text_t *text_decoder,
    pico_rnode_proto_command_decoder_t* decoder,
    FILE *out,
    const char *prefix
);

#ifdef __cplusplus
}
#endif
