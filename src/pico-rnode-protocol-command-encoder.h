// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "pico-rnode-protocol-consts.h"

// -----------------------------------------------------------
// Encoder for outgoing protocol commands.
// -----------------------------------------------------------

typedef enum {
    PICO_RNODE_PROTO_ENCODER_STATUS_OK = 0,
    PICO_RNODE_PROTO_ENCODER_STATUS_ABORTED,

    // A transmission is in progress but an attempt was made to send a command 
    PICO_RNODE_PROTO_ENCODER_STATUS_FRAME_ERROR,

} pico_rnode_proto_encoder_status_t;

typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_cmd_start_cb_t)(
    void * context
);

typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_cmd_put_cb_t)(
    void * context,
    uint8_t byte
);

typedef pico_rnode_proto_frame_cb_status_t (*pico_rnode_proto_cmd_end_cb_t)(
    void * context
);

typedef enum {
    PICO_RNODE_PROTO_ENCODER_STATE_IDLE = 0,
    PICO_RNODE_PROTO_ENCODER_STATE_TRANSMITTING,
} pico_rnode_proto_encoder_state_t;

typedef struct {
    void * context;
    pico_rnode_proto_encoder_state_t state;
    pico_rnode_proto_cmd_start_cb_t start_cb;
    pico_rnode_proto_cmd_put_cb_t put_cb;
    pico_rnode_proto_cmd_end_cb_t end_cb;
} pico_rnode_proto_command_encoder_t;

void pico_rnode_proto_command_encoder_init(
    pico_rnode_proto_command_encoder_t *encoder,
    void * context,
    pico_rnode_proto_cmd_start_cb_t start_cb,
    pico_rnode_proto_cmd_put_cb_t put_cb,
    pico_rnode_proto_cmd_end_cb_t end_cb
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_frequency(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t hz
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_bandwidth(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint32_t bandwidth
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_txpower(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    int8_t dbm
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_spreading_factor(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t sf // spreading factor, for LoRa radios (typically 6-12)
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_coding_rate(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    uint8_t cr // coding rate, for LoRa radios (typically 5-8)
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_set_radio_state(
    pico_rnode_proto_command_encoder_t *encoder,
    uint8_t interface,
    pico_rnode_proto_radio_state_t state // radio state, for LoRa radios (typically 0-2)
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_detect(
    pico_rnode_proto_command_encoder_t *encoder
);

pico_rnode_proto_encoder_status_t pico_rnode_proto_command_leave(
    pico_rnode_proto_command_encoder_t *encoder
);

#ifdef __cplusplus
}
#endif
