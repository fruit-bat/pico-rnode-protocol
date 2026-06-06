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
    pico_rnode_proto_command_decoder_t decoder; /**< Underlying command decoder. */
} pico_rnode_proto_command_decoder_text_t;

/**
 * Initialize a text-mode command decoder.
 *
 * The provided output stream is used for all decoded message output. If
 * `out` is NULL, `stdout` is used.
 */
void pico_rnode_proto_command_decoder_text_init(
    pico_rnode_proto_command_decoder_text_t *text_decoder,
    FILE *out
);

/**
 * Feed a single command byte into the text decoder.
 */
pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_text_put(
    pico_rnode_proto_command_decoder_text_t *text_decoder,
    uint8_t byte
);

/**
 * Feed a buffer of command bytes into the text decoder.
 */
pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_text_write(
    pico_rnode_proto_command_decoder_text_t *text_decoder,
    const uint8_t *bytes,
    size_t len
);

/**
 * Notify the text decoder that a new command frame has started.
 */
void pico_rnode_proto_command_decoder_text_start(
    pico_rnode_proto_command_decoder_text_t *text_decoder
);

/**
 * Notify the text decoder that the current command frame has ended.
 */
pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_text_end(
    pico_rnode_proto_command_decoder_text_t *text_decoder
);

#ifdef __cplusplus
}
#endif
