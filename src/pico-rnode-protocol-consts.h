// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RNODE_RADIO_STATE_OFF = 0x00,
    RNODE_RADIO_STATE_ON  = 0x01,
    RNODE_RADIO_STATE_ASK = 0xFF
} pico_rnode_proto_radio_state_t;

typedef enum {
    PICO_RNODE_PROTO_FRAME_CB_STATUS_OK = 0,
    PICO_RNODE_PROTO_FRAME_CB_STATUS_ABORT = 1,
} pico_rnode_proto_frame_cb_status_t;

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
} rnode_opcode_t;

#ifdef __cplusplus
}
#endif 
