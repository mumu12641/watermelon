#pragma once
#include <stdint.h>

namespace Constant {

static const uint32_t BlockBits         = 16;
static const uint32_t BlockSizeInBytes  = 1 << BlockBits;
static const uint32_t BlockCountInChunk = 8;

static const uint32_t LineBits         = 8;
static const uint32_t LineSizeInBytes  = 1 << LineBits;
static const uint32_t LineCountInBlock = BlockSizeInBytes / LineSizeInBytes;   // 256

static const uint32_t BlockHeaderLineCount   = 2;
static const uint32_t BlockHeaderSizeInBytes = BlockHeaderLineCount * LineSizeInBytes;   // 2 * 256

static const uint32_t ObjectHeaderWithoutPtrSizeInBytes = 8;                      // 8 bytes
static const uint32_t ObjectHeaderSizeInBytes           = 8 + sizeof(uint8_t*);   // 16 bytes

}   // namespace Constant