#pragma once
#include "block.hpp"
#include "block_flags.hpp"
#include "chunk.hpp"
#include "common.hpp"
#include "constant.hpp"
#include "line.hpp"
#include "line_flags.hpp"
#include "object.hpp"

struct Allocator
{
    void*  mmapAddr;
    Chunk* chunk;
    // Block* currBlock;

    void  init();
    void* malloc(uint32_t size);
    void  release();
};
