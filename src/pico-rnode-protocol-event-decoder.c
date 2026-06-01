#include "pico-rnode-protocol-event-decoder.h"

static int32_t rnode_event_opcode_length(uint8_t opcode) {
    switch (opcode) {
        case 0x01: // STAT_RX
        case 0x02: // STAT_TX
            return -1;
        case 0x03: // STAT_RSSI
        case 0x04: // STAT_SNR
            return 1;
        case 0x00: // BLINK or DATA
            return 0;
        default:
            return -2;
    }
}

static pico_rnode_proto_event_decoder_status_t pico_rnode_proto_event_decoder_fixed_length_event(
    pico_rnode_proto_event_decoder_t *decoder
) {
    pico_rnode_proto_event_decoder_status_t status = PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK;
    uint32_t value = 0;

    for (uint32_t i = 0; i < decoder->opcode_length; i++) {
        value <<= 8;
        value |= (uint32_t)decoder->smallbuf[i];
    }

    switch (decoder->opcode) {
        case 0x03: // STAT_RSSI
            if (decoder->rssi_cb) {
                decoder->rssi_cb(decoder->context, decoder->interface, (int8_t)value);
            }
            break;
        case 0x04: // STAT_SNR
            if (decoder->snr_cb) {
                decoder->snr_cb(decoder->context, decoder->interface, (int8_t)value);
            }
            break;
        case 0x00: // BLINK or DATA with no payload
            if (decoder->blink_cb) {
                decoder->blink_cb(decoder->context);
            }
            break;
        default:
            status = PICO_RNODE_PROTO_EVENT_DECODER_STATUS_UNKNOWN_OPCODE;
            break;
    }

    if (status != PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK && decoder->error_cb) {
        decoder->error_cb(
            decoder->context,
            decoder->interface,
            decoder->opcode,
            decoder->payload_index,
            status
        );
    }
    return status;
}

static void pico_rnode_proto_event_decoder_select_stream_callbacks(
    pico_rnode_proto_event_decoder_t *decoder
) {
    switch (decoder->opcode) {
        case 0x01: // STAT_RX
            decoder->stream.start_cb = decoder->stat_rx_start_cb;
            decoder->stream.data_cb = decoder->stat_rx_data_cb;
            decoder->stream.end_cb = decoder->stat_rx_end_cb;
            break;
        case 0x02: // STAT_TX
            decoder->stream.start_cb = decoder->stat_tx_start_cb;
            decoder->stream.data_cb = decoder->stat_tx_data_cb;
            decoder->stream.end_cb = decoder->stat_tx_end_cb;
            break;
        default:
            decoder->stream.start_cb = NULL;
            decoder->stream.data_cb = NULL;
            decoder->stream.end_cb = NULL;
            break;
    }
}

void pico_rnode_proto_event_decoder_init(
    pico_rnode_proto_event_decoder_t *decoder,
    void * context,
    pico_rnode_proto_event_rssi_cb_t rssi_cb,
    pico_rnode_proto_event_snr_cb_t snr_cb,
    pico_rnode_proto_event_blink_cb_t blink_cb,
    pico_rnode_proto_event_random_cb_t random_cb,
    pico_rnode_proto_event_platform_cb_t platform_cb,
    pico_rnode_proto_event_mcu_cb_t mcu_cb,
    pico_rnode_proto_event_fw_version_cb_t fw_version_cb,
    pico_rnode_proto_stream_start_cb_t stat_rx_start_cb,
    pico_rnode_proto_stream_data_cb_t stat_rx_data_cb,
    pico_rnode_proto_stream_end_cb_t stat_rx_end_cb,
    pico_rnode_proto_stream_start_cb_t stat_tx_start_cb,
    pico_rnode_proto_stream_data_cb_t stat_tx_data_cb,
    pico_rnode_proto_stream_end_cb_t stat_tx_end_cb,
    pico_rnode_proto_event_decoder_error_cb_t error_cb
) {
    decoder->context = context;
    decoder->payload_index = 0;
    decoder->opcode_length = 0;
    decoder->state = PICO_RNODE_PROTO_EVENT_DECODER_STATE_WAIT_EVENT;

    decoder->rssi_cb = rssi_cb;
    decoder->snr_cb = snr_cb;
    decoder->blink_cb = blink_cb;
    decoder->random_cb = random_cb;
    decoder->platform_cb = platform_cb;
    decoder->mcu_cb = mcu_cb;
    decoder->fw_version_cb = fw_version_cb;

    decoder->stat_rx_start_cb = stat_rx_start_cb;
    decoder->stat_rx_data_cb = stat_rx_data_cb;
    decoder->stat_rx_end_cb = stat_rx_end_cb;
    decoder->stat_tx_start_cb = stat_tx_start_cb;
    decoder->stat_tx_data_cb = stat_tx_data_cb;
    decoder->stat_tx_end_cb = stat_tx_end_cb;

    pico_rnode_proto_stream_init(&decoder->stream, NULL, NULL, NULL);
    decoder->error_cb = error_cb;
}

void pico_rnode_proto_event_decoder_start(
    pico_rnode_proto_event_decoder_t *decoder
) {
    decoder->state = PICO_RNODE_PROTO_EVENT_DECODER_STATE_WAIT_EVENT;
    decoder->payload_index = 0;
}

pico_rnode_proto_event_decoder_status_t pico_rnode_proto_event_decoder_put(
    pico_rnode_proto_event_decoder_t *decoder,
    uint8_t byte
) {
    switch (decoder->state) {
        case PICO_RNODE_PROTO_EVENT_DECODER_STATE_WAIT_EVENT: {
            decoder->interface = (byte >> 4) & 0x0F;
            decoder->opcode = byte & 0x0F;
            decoder->payload_index = 0;

            int32_t opcode_length = rnode_event_opcode_length(decoder->opcode);
            switch (opcode_length) {
                case -2:
                    decoder->state = PICO_RNODE_PROTO_EVENT_DECODER_STATE_ABORT;
                    if (decoder->error_cb) {
                        decoder->error_cb(
                            decoder->context,
                            decoder->interface,
                            decoder->opcode,
                            decoder->payload_index,
                            PICO_RNODE_PROTO_EVENT_DECODER_STATUS_UNKNOWN_OPCODE
                        );
                    }
                    return PICO_RNODE_PROTO_EVENT_DECODER_STATUS_UNKNOWN_OPCODE;
                case -1:
                    decoder->opcode_length = 0;
                    decoder->state = PICO_RNODE_PROTO_EVENT_DECODER_STATE_STREAM_DATA;
                    pico_rnode_proto_event_decoder_select_stream_callbacks(decoder);
                    if (decoder->stream.start_cb) {
                        pico_rnode_proto_stream_start(&decoder->stream, decoder->context, decoder->interface);
                    }
                    break;
                default:
                    decoder->opcode_length = (uint8_t)opcode_length;
                    decoder->state = PICO_RNODE_PROTO_EVENT_DECODER_STATE_READ_FIXED;
                    break;
            }
            break;
        }
        case PICO_RNODE_PROTO_EVENT_DECODER_STATE_READ_FIXED: {
            if (decoder->payload_index < decoder->opcode_length) {
                decoder->smallbuf[decoder->payload_index] = byte;
            } else {
                decoder->state = PICO_RNODE_PROTO_EVENT_DECODER_STATE_ABORT;
                if (decoder->error_cb) {
                    decoder->error_cb(
                        decoder->context,
                        decoder->interface,
                        decoder->opcode,
                        decoder->payload_index,
                        PICO_RNODE_PROTO_EVENT_DECODER_STATUS_INVALID_LENGTH
                    );
                }
                return PICO_RNODE_PROTO_EVENT_DECODER_STATUS_INVALID_LENGTH;
            }
            decoder->payload_index++;
            break;
        }
        case PICO_RNODE_PROTO_EVENT_DECODER_STATE_STREAM_DATA: {
            if (decoder->stream.data_cb) {
                pico_rnode_proto_stream_cb_status_t cb_status = pico_rnode_proto_stream_data(
                    &decoder->stream,
                    decoder->context,
                    decoder->interface,
                    byte
                );
                if (cb_status != PICO_RNODE_PROTO_STREAM_CB_STATUS_OK) {
                    decoder->state = PICO_RNODE_PROTO_EVENT_DECODER_STATE_ABORT;
                    if (decoder->error_cb) {
                        decoder->error_cb(
                            decoder->context,
                            decoder->interface,
                            decoder->opcode,
                            decoder->payload_index,
                            PICO_RNODE_PROTO_EVENT_DECODER_STATUS_ABORTED
                        );
                    }
                    return PICO_RNODE_PROTO_EVENT_DECODER_STATUS_ABORTED;
                }
            }
            decoder->payload_index++;
            break;
        }
        default:
            return PICO_RNODE_PROTO_EVENT_DECODER_STATUS_ABORTED;
    }
    return PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK;
}

pico_rnode_proto_event_decoder_status_t pico_rnode_proto_event_decoder_write(
    pico_rnode_proto_event_decoder_t *decoder,
    const uint8_t * bytes,
    size_t len
) {
    for (size_t i = 0; i < len; i++) {
        pico_rnode_proto_event_decoder_status_t status = pico_rnode_proto_event_decoder_put(
            decoder,
            bytes[i]
        );
        if (status != PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK) {
            return status;
        }
    }
    return PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK;
}

pico_rnode_proto_event_decoder_status_t pico_rnode_proto_event_decoder_end(
    pico_rnode_proto_event_decoder_t *decoder
) {
    pico_rnode_proto_event_decoder_status_t status = PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK;
    switch (decoder->state) {
        case PICO_RNODE_PROTO_EVENT_DECODER_STATE_READ_FIXED: {
            if (decoder->payload_index == decoder->opcode_length) {
                status = pico_rnode_proto_event_decoder_fixed_length_event(decoder);
            } else {
                status = PICO_RNODE_PROTO_EVENT_DECODER_STATUS_INVALID_LENGTH;
                if (decoder->error_cb) {
                    decoder->error_cb(
                        decoder->context,
                        decoder->interface,
                        decoder->opcode,
                        decoder->payload_index,
                        PICO_RNODE_PROTO_EVENT_DECODER_STATUS_INVALID_LENGTH
                    );
                }
            }
            break;
        }
        case PICO_RNODE_PROTO_EVENT_DECODER_STATE_STREAM_DATA: {
            if (decoder->stream.end_cb) {
                pico_rnode_proto_stream_cb_status_t cb_status = pico_rnode_proto_stream_end(
                    &decoder->stream,
                    decoder->context,
                    decoder->interface
                );
                if (cb_status != PICO_RNODE_PROTO_STREAM_CB_STATUS_OK) {
                    decoder->state = PICO_RNODE_PROTO_EVENT_DECODER_STATE_ABORT;
                    if (decoder->error_cb) {
                        decoder->error_cb(
                            decoder->context,
                            decoder->interface,
                            decoder->opcode,
                            decoder->payload_index,
                            PICO_RNODE_PROTO_EVENT_DECODER_STATUS_ABORTED
                        );
                    }
                    return PICO_RNODE_PROTO_EVENT_DECODER_STATUS_ABORTED;
                }
            }
            break;
        }
        default:
            status = PICO_RNODE_PROTO_EVENT_DECODER_STATUS_ABORTED;
            break;
    }
    pico_rnode_proto_event_decoder_start(decoder);
    return status;
}

