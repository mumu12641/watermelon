#pragma once
#include "block.hpp"
#include "constant.hpp"

struct ChunkHeader
{
    uint32_t freeBlocksNum;
    uint32_t s;
};

struct Chunk
{
    // ChunkHeader header;
    Block blocks[Constant::BlockCountInChunk];
};