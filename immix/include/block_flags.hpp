#pragma once
#include <stdint.h>

enum class BlockFlags : uint8_t
{
    Free        = 0x00,
    Unavailable = 0x01,
    Recyclable  = 0x02,
};
