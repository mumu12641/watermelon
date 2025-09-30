#include "../include/allocator.hpp"

void Allocator::init()
{
    mmapAddr = malloc(sizeof(Chunk));
    // mmapAddr =
    //     mmap(nullptr, sizeof(Chunk), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    this->chunk = new (mmapAddr) Chunk();

    for (int i = 0; i < Constant::BlockCountInChunk; i++) {
        this->chunk->blocks[i].initialize();
    }
    // this->currBlock = &this->chunk->blocks[0];
}

void Allocator::release()
{
    // munmap(mmapAddr, sizeof(Chunk));
    free(mmapAddr);
}

void* Allocator::malloc(uint32_t size)
{
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

        // ObjectHeader header(size, ObjectType::STANDARD, cursor +
        // Constant::ObjectHeaderSizeInBytes);
        // *(ObjectHeader*)cursor = header;
        // currFreeBlock->header.info.cursor += totalSizeInBytes;
        // return header.field;

        ObjectHeader* objHeader = (ObjectHeader*)cursor;
        objHeader->size         = size;
        objHeader->type         = ObjectType::STANDARD;

        uint8_t* objData = cursor + Constant::ObjectHeaderSizeInBytes;
        objHeader->field = objData;

        currFreeBlock->header.info.cursor += totalSizeInBytes;

        currFreeBlock->header.info.blockFlags = BlockFlags::Recyclable;


        memset(objData, 0, size);

        return objData;
        // if(totalSizeInBytes + cursor > limit){

        // }
    }

    return nullptr;
}