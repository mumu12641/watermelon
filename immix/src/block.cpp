#include "../include/block.hpp"

void Block::initialize()
{
    memset(lines, 0, sizeof(lines));
    header.info.blockFlags = BlockFlags::Free;
    header.info.cursor     = (uint8_t*)(this + Constant::BlockHeaderSizeInBytes);
    header.info.limit      = (uint8_t*)(this + Constant::BlockSizeInBytes);
    for (uint32_t i = 0; i < Constant::LineCountInBlock; i++) {
        header.lineFlags[i] = LineFlags::FREE;
    }
}