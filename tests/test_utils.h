#pragma once

#include <stdint.h>
#include <stdio.h>

void assert_equal_bytes(const uint8_t *actual, const uint8_t *expected, size_t len);
