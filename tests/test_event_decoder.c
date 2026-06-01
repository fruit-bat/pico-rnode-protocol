#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "pico-rnode-protocol-event-decoder.h"
#include "pico-rnode-protocol-event-encoder.h"
#include "test_utils.h"

typedef struct {
    uint32_t rssi_count;
    int8_t last_rssi;
    uint32_t stat_rx_start_count;
    uint32_t stat_rx_data_count;
    uint32_t stat_rx_end_count;
    uint32_t stat_rx_length;
    uint32_t stat_tx_start_count;
    uint32_t stat_tx_data_count;
    uint32_t stat_tx_end_count;
    uint32_t stat_tx_length;
    uint32_t error_count;
} event_decoder_state_t;

static event_decoder_state_t event_state;
static uint8_t event_buffer[64];
static size_t event_buffer_len;

static void reset_event_state(void) {
    memset(&event_state, 0, sizeof(event_state));
    event_buffer_len = 0;
}

static pico_rnode_proto_frame_cb_status_t event_encoder_start_cb(void *context) {
    (void)context;
    event_buffer_len = 0;
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static pico_rnode_proto_frame_cb_status_t event_encoder_put_cb(
    void *context,
    uint8_t byte
) {
    (void)context;
    assert(event_buffer_len < sizeof(event_buffer));
    event_buffer[event_buffer_len++] = byte;
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static pico_rnode_proto_frame_cb_status_t event_encoder_end_cb(void *context) {
    (void)context;
    return PICO_RNODE_PROTO_FRAME_CB_STATUS_OK;
}

static void event_decoder_error_cb(
    void *context,
    uint8_t interface,
    uint8_t opcode,
    uint32_t index,
    pico_rnode_proto_event_decoder_status_t status
) {
    (void)context;
    (void)interface;
    (void)opcode;
    (void)index;
    (void)status;
    event_state.error_count++;
}

static void event_decoder_rssi_cb(
    void *context,
    uint8_t interface,
    int8_t rssi
) {
    (void)context;
    assert(interface == 1);
    assert(rssi == -42);
    event_state.rssi_count++;
    event_state.last_rssi = rssi;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_rx_start_cb(
    void *context,
    uint8_t interface
) {
    (void)context;
    assert(interface == 2);
    event_state.stat_rx_start_count++;
    event_state.stat_rx_length = 0;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_rx_data_cb(
    void *context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
) {
    (void)context;
    assert(interface == 2);
    assert(byte_index == event_state.stat_rx_length);
    event_state.stat_rx_data_count++;
    event_state.stat_rx_length++;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_rx_end_cb(
    void *context,
    uint8_t interface,
    uint32_t length
) {
    (void)context;
    assert(interface == 2);
    assert(length == event_state.stat_rx_length);
    event_state.stat_rx_end_count++;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_tx_start_cb(
    void *context,
    uint8_t interface
) {
    (void)context;
    assert(interface == 3);
    event_state.stat_tx_start_count++;
    event_state.stat_tx_length = 0;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_tx_data_cb(
    void *context,
    uint8_t interface,
    uint8_t byte,
    uint32_t byte_index
) {
    (void)context;
    assert(interface == 3);
    assert(byte_index == event_state.stat_tx_length);
    event_state.stat_tx_data_count++;
    event_state.stat_tx_length++;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static pico_rnode_proto_stream_cb_status_t event_decoder_stat_tx_end_cb(
    void *context,
    uint8_t interface,
    uint32_t length
) {
    (void)context;
    assert(interface == 3);
    assert(length == event_state.stat_tx_length);
    event_state.stat_tx_end_count++;
    return PICO_RNODE_PROTO_STREAM_CB_STATUS_OK;
}

static void init_event_decoder(
    pico_rnode_proto_event_decoder_t *decoder
) {
    pico_rnode_proto_event_decoder_init(
        decoder,
        &event_state,
        event_decoder_rssi_cb,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        event_decoder_stat_rx_start_cb,
        event_decoder_stat_rx_data_cb,
        event_decoder_stat_rx_end_cb,
        event_decoder_stat_tx_start_cb,
        event_decoder_stat_tx_data_cb,
        event_decoder_stat_tx_end_cb,
        event_decoder_error_cb
    );
}

static void init_event_encoder(
    pico_rnode_proto_event_encoder_t *encoder
) {
    pico_rnode_proto_event_encoder_init(
        encoder,
        &event_state,
        event_encoder_start_cb,
        event_encoder_put_cb,
        event_encoder_end_cb
    );
}

static void event_decode_buffer(
    pico_rnode_proto_event_decoder_t *decoder
) {
    pico_rnode_proto_event_decoder_status_t status;
    pico_rnode_proto_event_decoder_start(decoder);
    status = pico_rnode_proto_event_decoder_write(decoder, event_buffer, event_buffer_len);
    assert(status == PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK);
    status = pico_rnode_proto_event_decoder_end(decoder);
    assert(status == PICO_RNODE_PROTO_EVENT_DECODER_STATUS_OK);
}

static void test_event_decoder_rssi(void) {
    pico_rnode_proto_event_encoder_t encoder = {0};
    pico_rnode_proto_event_decoder_t decoder = {0};

    reset_event_state();
    init_event_encoder(&encoder);
    init_event_decoder(&decoder);

    assert(pico_rnode_proto_event_rssi(&encoder, 1, -42) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(event_buffer_len == 2);

    event_decode_buffer(&decoder);
    assert(event_state.rssi_count == 1);
    assert(event_state.last_rssi == -42);
    assert(event_state.error_count == 0);
}

static void test_event_decoder_stat_rx(void) {
    pico_rnode_proto_event_encoder_t encoder = {0};
    pico_rnode_proto_event_decoder_t decoder = {0};
    const uint8_t payload[] = {0x0A, 0x0B, 0x0C};

    reset_event_state();
    init_event_encoder(&encoder);
    init_event_decoder(&decoder);

    assert(pico_rnode_proto_event_stat_rx(&encoder, 2, payload, sizeof(payload)) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(event_buffer_len == 4);

    event_decode_buffer(&decoder);
    assert(event_state.stat_rx_start_count == 1);
    assert(event_state.stat_rx_data_count == sizeof(payload));
    assert(event_state.stat_rx_end_count == 1);
    assert(event_state.stat_rx_length == sizeof(payload));
    assert(event_state.error_count == 0);
}

static void test_event_decoder_stat_tx(void) {
    pico_rnode_proto_event_encoder_t encoder = {0};
    pico_rnode_proto_event_decoder_t decoder = {0};
    const uint8_t payload[] = {0x10, 0x20};

    reset_event_state();
    init_event_encoder(&encoder);
    init_event_decoder(&decoder);

    assert(pico_rnode_proto_event_stat_tx(&encoder, 3, payload, sizeof(payload)) == PICO_RNODE_PROTO_ENCODER_STATUS_OK);
    assert(event_buffer_len == 3);

    event_decode_buffer(&decoder);
    assert(event_state.stat_tx_start_count == 1);
    assert(event_state.stat_tx_data_count == sizeof(payload));
    assert(event_state.stat_tx_end_count == 1);
    assert(event_state.stat_tx_length == sizeof(payload));
    assert(event_state.error_count == 0);
}

static void run_test(const char *name, void (*fn)(void)) {
    printf("[ RUN ] %s\n", name);
    reset_event_state();
    fn();
    printf("[ PASS ] %s\n", name);
}

void test_event_decoder(void) {
    run_test("event_decoder_rssi", test_event_decoder_rssi);
    run_test("event_decoder_stat_rx", test_event_decoder_stat_rx);
    run_test("event_decoder_stat_tx", test_event_decoder_stat_tx);
    printf("All event decoder tests passed.\n");
}
