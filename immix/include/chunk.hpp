#pragma once
#include "block.hpp"
#include "constant.hpp"

struct Chunk
{
    Block blocks[Constant::BlockCountInChunk];
};