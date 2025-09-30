#pragma once
#include <stdint.h>

enum class LineFlags : uint8_t
{
    FREE         = 0x00,
    MARKER       = 0x01,
    ALLOCATED    = 0x02,
    CONSERVATIVE = 0x03
};
