// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef enum {
    RNODE_OPCODE_DATA              = 0x00,
    RNODE_OPCODE_FREQUENCY         = 0x01,
    RNODE_OPCODE_BANDWIDTH         = 0x02,
    RNODE_OPCODE_TXPOWER           = 0x03,
    RNODE_OPCODE_SF                = 0x04,
    RNODE_OPCODE_CR                = 0x05,
    RNODE_OPCODE_RADIO_STATE       = 0x06,
    RNODE_OPCODE_DETECT            = 0x08,
    RNODE_OPCODE_LEAVE             = 0x0A,
    RNODE_OPCODE_READY             = 0x0F,
} rnode_opcode_t;

typedef enum {
    RNODE_EVENT_DATA,
    RNODE_EVENT_SET_FREQUENCY,
    RNODE_EVENT_SET_BANDWIDTH,
    RNODE_EVENT_SET_TXPOWER,
    RNODE_EVENT_SET_SF,
    RNODE_EVENT_SET_CR,
    RNODE_EVENT_READY,
    RNODE_EVENT_UNKNOWN
} rnode_event_type_t;

typedef struct {
    rnode_event_type_t type;
    uint8_t interface;

    union {
        struct {
            const uint8_t *data;
            size_t len;
        } data;

        struct {
            uint32_t hz;
        } frequency;

        struct {
            uint32_t bandwidth;
        } bandwidth;

        struct {
            int8_t dbm;
        } txpower;

        struct {
            uint8_t sf;
        } sf;

        struct {
            uint8_t cr;
        } cr;
    };
} rnode_event_t;


#ifdef __cplusplus
}
#endif
