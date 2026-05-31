#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * Event decoder placeholder.
 *
 * This module is reserved for decoding incoming RNODE events such as telemetry,
 * status updates, or other device-originated packets. It is intentionally
 * separated from command decoding so that event handling remains independent
 * from command parsing.
 *
 * The API surface for event decoding is not yet defined.
 */

// -----------------------------------------------------------
// Decoder for incoming protocol events (e.g. from radio to host).
// -----------------------------------------------------------



#ifdef __cplusplus
}
#endif 
