// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "pico-rnode-protocol-event-decoder.h"

/**
 * Text-mode event decoder context.
 *
 * This wrapper holds a FILE stream and an optional prefix string used when
 * printing decoded event output.
 */
typedef struct {
    FILE *out; /**< Output stream for human-readable decoded messages. */
    const char *prefix; /**< Optional prefix printed before each message. */
} pico_rnode_proto_event_decoder_text_t;

/**
 * Initialize a text-mode event decoder.
 *
 * @param text_decoder text decoder context to initialize.
 * @param decoder event decoder instance to attach.
 * @param out output stream for decoded text messages, or NULL for stdout.
 * @param prefix optional prefix string printed before each decoded line.
 */
void pico_rnode_proto_event_decoder_text_init(
    pico_rnode_proto_event_decoder_text_t *text_decoder,
    pico_rnode_proto_event_decoder_t *decoder,
    FILE *out,
    const char *prefix
);

#ifdef __cplusplus
}
#endif
