#pragma once
#include "block_flags.hpp"
#include "common.hpp"
#include "constant.hpp"
#include "line.hpp"
#include "line_flags.hpp"

struct Block
{
    struct BlockHeader
    {
        struct Info
        {
            BlockFlags blockFlags;

            uint8_t* cursor;
            uint8_t* limit;

            uint32_t holeCount;
        } info;
        uint8_t padding[Constant::LineCountInBlock - sizeof(Info)];

        // one line
        LineFlags lineFlags[Constant::LineCountInBlock];
    } header;

    Line lines[Constant::LineCountInBlock - Constant::BlockHeaderLineCount];
    
    inline bool isRecyclable() { return header.info.blockFlags == BlockFlags::Recyclable; }
    inline bool isFree() { return header.info.blockFlags == BlockFlags::Free; }
    inline bool IsUnavailable() { return header.info.blockFlags == BlockFlags::Unavailable; }

    void initialize();
};
