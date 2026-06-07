// SPDX-License-Identifier: MIT
// Copyright (c) 2026 fruit-bat

// Enable GNU miscellaneous extensions so cfmakeraw() is declared.
// cfmakeraw() is a glibc extension guarded by __USE_MISC.
// _XOPEN_SOURCE 700 is also required so POSIX/X/Open PTY helpers
// like posix_openpt(), grantpt(), unlockpt(), and ptsname() are exposed.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#include <errno.h>
#include <fcntl.h>
#include <pty.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "pico-kiss-protocol.h"
#include "pico-serial-proxy.h"
#include "pico-rnode-protocol-command-decoder-text.h"

#define BUFFER_SIZE 1024
#define MAX_FRAME_BYTES 2048

// The serial configuration and PTY management is handled by
// the pico-serial-proxy library. The monitor app provides a
// callback to receive bytes forwarded in either direction.

typedef struct monitor_context monitor_context_t;

typedef pico_rnode_proto_decoder_status_t (*pico_rnode_proto_decoder_mon_put_t)(
    monitor_context_t *monitor_context,
    uint8_t byte
);

typedef pico_rnode_proto_decoder_status_t (*pico_rnode_proto_decoder_mon_write_t)(
    monitor_context_t *monitor_context,
    const uint8_t* bytes,
    size_t len
);

typedef void (*pico_rnode_proto_decoder_mon_start_t)(
    monitor_context_t *monitor_context
);

typedef pico_rnode_proto_decoder_status_t (*pico_rnode_proto_decoder_mon_end_t)(
    monitor_context_t *monitor_context
);

typedef struct monitor_context {
    const char *direction;
    uint8_t frame[MAX_FRAME_BYTES];
    size_t frame_len;
    void *rnode_decoder;
    pico_rnode_proto_decoder_mon_start_t start;
    pico_rnode_proto_decoder_mon_put_t put;
    pico_rnode_proto_decoder_mon_write_t write;
    pico_rnode_proto_decoder_mon_end_t end;
} monitor_context_t;

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_mon_put(
    monitor_context_t *monitor_context,
    uint8_t byte
) {
    return pico_rnode_proto_command_decoder_put(
        (pico_rnode_proto_command_decoder_t *)monitor_context->rnode_decoder,
        byte
    );
}

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_mon_write(
    monitor_context_t *monitor_context,
    const uint8_t* bytes,
    size_t len
) {
    return pico_rnode_proto_command_decoder_write(
        (pico_rnode_proto_command_decoder_t *)monitor_context->rnode_decoder,
        bytes,
        len
    );
}

void pico_rnode_proto_command_decoder_mon_start(
    monitor_context_t *monitor_context
) {
    pico_rnode_proto_command_decoder_start(
        (pico_rnode_proto_command_decoder_t *)monitor_context->rnode_decoder
    );
}

pico_rnode_proto_decoder_status_t pico_rnode_proto_command_decoder_mon_end(
    monitor_context_t *monitor_context
) {
    return pico_rnode_proto_command_decoder_end(
        (pico_rnode_proto_command_decoder_t *)monitor_context->rnode_decoder
    );
}

static void print_hex_frame(const char *prefix, const uint8_t *data, size_t len) {
    printf("%s [%zu]:", prefix, len);
    for (size_t i = 0; i < len; i++) {
        printf(" %02X", data[i]);
    }
    printf("\n");
}

static void decoder_start(void *data) {
    struct monitor_context *ctx = (struct monitor_context *)data;
    ctx->frame_len = 0;
}

static pico_kiss_proto_decoder_data_cb_status_t decoder_data(
    void *data,
    uint8_t byte,
    uint32_t byte_index
) {
    (void)byte_index;
    struct monitor_context *ctx = (struct monitor_context *)data;
    if (ctx->frame_len < sizeof(ctx->frame)) {
        ctx->frame[ctx->frame_len++] = byte;
    }
    return PICO_KISS_PROTO_DECODER_DATA_CB_STATUS_OK;
}

static void decoder_end(void *data, pico_kiss_proto_frame_info_t *info) {
    struct monitor_context *ctx = (struct monitor_context *)data;
    if (info->status == PICO_KISS_PROTO_DECODER_STATUS_FRAME_COMPLETE) {
        print_hex_frame(ctx->direction, ctx->frame, ctx->frame_len);
    } else {
        printf("%s [error=%s, len=%u]\n",
               ctx->direction,
               pico_kiss_proto_decoder_status_to_string(info->status),
               info->len);
    }
}

static void decoder_error(
    void *data,
    pico_kiss_proto_decoder_status_t status,
    uint32_t index
) {
    struct monitor_context *ctx = (struct monitor_context *)data;
    printf("%s decode error: %s at byte index %u\n",
           ctx->direction,
           pico_kiss_proto_decoder_status_to_string(status),
           index);
}

static ssize_t write_all(int fd, const void *buffer, size_t len) {
    const uint8_t *ptr = (const uint8_t *)buffer;
    size_t remaining = len;

    while (remaining > 0) {
        ssize_t written = write(fd, ptr, remaining);
        if (written <= 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        ptr += written;
        remaining -= (size_t)written;
    }
    return (ssize_t)len;
}

static pico_serial_proxy_t monitor_proxy;

static void proxy_signal_handler(int signum) {
    const char *signal_name = "unknown";
    switch (signum) {
        case SIGINT: signal_name = "SIGINT"; break;
        case SIGTERM: signal_name = "SIGTERM"; break;
        case SIGHUP: signal_name = "SIGHUP"; break;
    }
    fprintf(stderr, "Received %s, shutting down proxy...\n", signal_name);
    pico_serial_proxy_request_stop(&monitor_proxy);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <real_tty_device> [baud_rate]\n", argv[0]);
        return 1;
    }

    const char *real_tty_path = argv[1];
    int baud_rate = 0;
    if (argc >= 3) {
        baud_rate = atoi(argv[2]);
    }

    signal(SIGINT, proxy_signal_handler);
    signal(SIGTERM, proxy_signal_handler);
    signal(SIGHUP, proxy_signal_handler);

    // The pico-serial-proxy library will open and configure the real
    // device and create the virtual PTY; we just initialize it below.

    // prepare decoders and contexts as globals so the proxy callback
    // can feed bytes into them.
    static struct monitor_context incoming_ctx = {
        .direction = "H<-D",
        .frame_len = 0,
    };
    static struct monitor_context outgoing_ctx = {
        .direction = "H->D",
        .frame_len = 0,
    };

    static pico_kiss_proto_decoder_t incoming_decoder;
    static pico_kiss_proto_decoder_t outgoing_decoder;
    static pico_rnode_proto_command_decoder_t command_decoder;
    static pico_rnode_proto_command_decoder_text_t command_text_decoder;
    // TODO event_decoder
    // TOOD event_decoder_text

    pico_rnode_proto_command_decoder_text_init(
        &command_text_decoder, 
        &command_decoder, 
        stdout);

    pico_kiss_proto_decoder_init(&incoming_decoder,
                                 &incoming_ctx,
                                 decoder_start,
                                 decoder_data,
                                 decoder_end,
                                 decoder_error);

    pico_kiss_proto_decoder_init(&outgoing_decoder,
                                 &outgoing_ctx,
                                 decoder_start,
                                 decoder_data,
                                 decoder_end,
                                 decoder_error);

    // callback invoked by the proxy when bytes are forwarded; host_to_device
    // is true for data coming from the virtual PTY towards the real device.
    void proxy_data_cb(void *context, bool host_to_device, const uint8_t *data, size_t len) {
        (void)context;
        if (host_to_device) {
            for (size_t i = 0; i < len; i++) {
                pico_kiss_proto_decoder_put(&outgoing_decoder, data[i]);
            }
        } else {
            for (size_t i = 0; i < len; i++) {
                pico_kiss_proto_decoder_put(&incoming_decoder, data[i]);
            }
        }
    }

    // lifecycle callback receives STARTED/ERROR/STOPPED events
    void proxy_lifecycle_cb(void *context, pico_serial_proxy_event_t event, const char *message) {
        (void)context;
        switch (event) {
            case PICO_SERIAL_PROXY_EVENT_STARTED:
                printf("[proxy] started, virt_tty=%s\n", message ? message : "(unknown)");
                break;
            case PICO_SERIAL_PROXY_EVENT_ERROR:
                printf("[proxy] error: %s\n", message ? message : "(no message)");
                break;
            case PICO_SERIAL_PROXY_EVENT_STOPPED:
                printf("[proxy] stopped: %s\n", message ? message : "");
                break;
        }
    }

    // Initialize and run the proxy library.
    if (pico_serial_proxy_init(&monitor_proxy,
                               real_tty_path,
                               baud_rate,
                               NULL,
                               proxy_data_cb,
                               proxy_lifecycle_cb) != 0) {
        perror("Error initializing serial proxy");
        return 1;
    }

    int rv = pico_serial_proxy_run(&monitor_proxy);
    (void)rv;
    return 0;
}
