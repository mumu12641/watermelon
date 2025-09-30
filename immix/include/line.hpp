#pragma once
#include "constant.hpp"
#include "line_flags.hpp"

struct Line
{
    uint8_t data[Constant::LineSizeInBytes];
};

