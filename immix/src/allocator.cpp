#include "../include/allocator.hpp"

void Allocator::initialize()
{
    size_t alignmentSize = sizeof(Block);
    size_t totalSize     = sizeof(Chunk) + alignmentSize;

    mmapPtr = mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    uintptr_t alignedAddr =
        (reinterpret_cast<uintptr_t>(mmapPtr) + alignmentSize - 1) & ~(alignmentSize - 1);
    alignedPtr = reinterpret_cast<void*>(alignedAddr);

    memset(alignedPtr, 0, sizeof(Chunk));
    this->chunk = new (alignedPtr) Chunk();

    for (int i = 0; i < Constant::BlockCountInChunk; i++) {
        this->chunk->blocks[i].initialize();
    }
}

void Allocator::release()
{
    size_t alignmentSize = sizeof(Block);
    size_t totalSize     = sizeof(Chunk) + alignmentSize;
    munmap(mmapPtr, totalSize);
}

void* Allocator::malloc(uint32_t size)
{
    // find a recyclable block, get cursor & limit, alloca a object
    // no recyclable block Or no fit hole, find free block
    // otherwise, run gc
    // if medium object & current cursor & limit do not fit it -> find a free block

    /* GC */
    // select from block
    // in from block, if

    // find a free block?
    // find a free line
    // alloca object and modify cursor & limit
    // set block recyclable

    Block* currBlock     = nullptr;
    Block* currFreeBlock = nullptr;
    for (int i = 0; i < Constant::BlockCountInChunk; i++) {
        Block* block = &this->chunk->blocks[i];
        if (block->isRecyclable()) {
            currBlock = block;
            break;
        }
        else if (block->isFree()) {
            currFreeBlock = block;
        }
    }

    if (currBlock == nullptr && currFreeBlock == nullptr) {
        // TODO: alloca new block
    }

    if (currFreeBlock != nullptr) {
        // there is a recycleale block
        uint32_t totalSizeInBytes = size + Constant::ObjectHeaderSizeInBytes;
        // auto     isMediumSizedObject = totalSizeInBytes > Constant::LineSizeInBytes;

        uint8_t* cursor = currFreeBlock->header.info.cursor;
        uint8_t* limit  = currFreeBlock->header.info.limit;

        if (cursor + totalSizeInBytes > limit) {
            return nullptr;
        }

        ObjectHeader* objHeader = (ObjectHeader*)cursor;
        objHeader->size         = size;
        objHeader->type         = ObjectType::STANDARD;

        uint8_t* objData = cursor + Constant::ObjectHeaderSizeInBytes;
        objHeader->field = objData;

        currFreeBlock->header.info.cursor += totalSizeInBytes;

        currFreeBlock->header.info.blockFlags = BlockFlags::Recyclable;


        memset(objData, 0, size);

        return objData;
    }

    return nullptr;
}