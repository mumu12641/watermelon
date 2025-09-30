#pragma once
#include <stdint.h>

enum class ObjectType : uint8_t
{
    STANDARD = 0x1,
    LARGE    = 0x2
};

struct ObjectHeader
{
    uint32_t   size;
    ObjectType type;
    uint8_t    _padding[3];
    uint8_t*   field;

};

// struct Object
// {
//     ObjectHeader header;
//     uint8_t*     field;
// };
