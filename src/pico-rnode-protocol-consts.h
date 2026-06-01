// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RNODE_LOCK_STATE_UNLOCKED = 0x00,
    RNODE_LOCK_STATE_LOCKED   = 0x01,
} pico_rnode_proto_lock_state_t;

typedef enum {
    RNODE_RADIO_STATE_OFF = 0x00,
    RNODE_RADIO_STATE_ON  = 0x01,
    RNODE_RADIO_STATE_ASK = 0xFF
} pico_rnode_proto_radio_state_t;

typedef enum {
    RNODE_DETECT_REQ = 0x73,
    RNODE_DETECT_RESP = 0x46
} pico_rnode_proto_detect_constants_t;

typedef enum {
    RNODE_OPCODE_DATA              = 0x00,
    RNODE_OPCODE_FREQUENCY         = 0x01,
    RNODE_OPCODE_BANDWIDTH         = 0x02,
    RNODE_OPCODE_TXPOWER           = 0x03,
    RNODE_OPCODE_SF                = 0x04,
    RNODE_OPCODE_CR                = 0x05,
    RNODE_OPCODE_RADIO_STATE       = 0x06,
    RNODE_OPCODE_RADIO_LOCK        = 0x07,
    RNODE_OPCODE_DETECT            = 0x08,
    RNODE_OPCODE_LEAVE             = 0x0A,
    RNODE_OPCODE_READY             = 0x0F,
    RNODE_OPCODE_STAT_RX           = 0x21,
    RNODE_OPCODE_STAT_TX           = 0x22,
    RNODE_OPCODE_STAT_RSSI         = 0x23,
    RNODE_OPCODE_STAT_SNR          = 0x24,
    RNODE_OPCODE_BLINK             = 0x30,
    RNODE_OPCODE_RANDOM            = 0x40,
    RNODE_OPCODE_PLATFORM          = 0x48,
    RNODE_OPCODE_MCU               = 0x49,
    RNODE_OPCODE_FW_VERSION        = 0x50,
    RNODE_OPCODE_ROM_READ          = 0x51,
    RNODE_OPCODE_RESET             = 0x55,
    RNODE_OPCODE_INTERFACES        = 0x71,
} rnode_opcode_t;

#ifdef __cplusplus
}
#endif 
