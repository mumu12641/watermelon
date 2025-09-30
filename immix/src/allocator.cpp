#include "../include/allocator.hpp"

void Allocator::initialize()
{
    // 计算所需内存大小，确保有额外空间用于对齐
    size_t alignmentSize = sizeof(Block);
    size_t totalSize = sizeof(Chunk) + alignmentSize;
    
    // 分配内存
    mmapAddr = mmap(nullptr,
                    totalSize,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1,
                    0);
        
    if (mmapAddr == MAP_FAILED) {
        std::cerr << "mmap failed: " << strerror(errno) << std::endl;
        return;
    }

    std::cout << "mmap addr: " << mmapAddr << std::endl;

    // 计算对齐后的地址
    uintptr_t addr = reinterpret_cast<uintptr_t>(mmapAddr);
    uintptr_t alignedAddr = (addr + alignmentSize - 1) & ~(alignmentSize - 1);
    void* alignedPtr = reinterpret_cast<void*>(alignedAddr);

    std::cout << "Original addr: " << mmapAddr << ", aligned addr: " << alignedPtr << std::endl;
    std::cout << "Alignment offset: " << (alignedAddr - addr) << " bytes" << std::endl;

    // 先清零内存
    memset(alignedPtr, 0, sizeof(Chunk));

    // 使用 placement new 在对齐的地址上构造 Chunk
    this->chunk = new (alignedPtr) Chunk();
    std::cout << "sizeof chunk " << sizeof(Chunk) << std::endl;
    std::cout << "chunk addr: " << this->chunk << std::endl;
    std::cout << "chunk end addr: " << reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this->chunk) + sizeof(Chunk)) << std::endl;

    // 检查地址对齐
    std::cout << "chunk alignment: " << (reinterpret_cast<uintptr_t>(this->chunk) % alignof(Chunk))
              << std::endl;
    std::cout << "block alignment: " << (reinterpret_cast<uintptr_t>(this->chunk) % sizeof(Block))
              << std::endl;

    std::cout << "blocks array addr: " << this->chunk->blocks << std::endl;
    std::cout << "first block addr: " << &this->chunk->blocks[0] << std::endl;

    // 逐步检查每个 block 的地址
    for (int i = 0; i < Constant::BlockCountInChunk; i++) {
        Block* currentBlock = &this->chunk->blocks[i];
        std::cout << "Block[" << i << "] addr: " << currentBlock << std::endl;
        std::cout << "Block[" << i << "] alignment: " << (reinterpret_cast<uintptr_t>(currentBlock) % sizeof(Block)) << std::endl;
        std::cout << "Block[" << i << "] header addr: " << &currentBlock->header << std::endl;
        std::cout << "Block[" << i << "] info addr: " << &currentBlock->header.info << std::endl;

        // 检查是否可以安全访问
        try {
            currentBlock->initialize();
            std::cout << "Block[" << i
                      << "] cursor: " << static_cast<void*>(currentBlock->header.info.cursor)
                      << ", limit: " << static_cast<void*>(currentBlock->header.info.limit)
                      << std::endl;
        }
        catch (...) {
            std::cerr << "Exception accessing Block[" << i << "]" << std::endl;
            break;
        }
    }
}

void Allocator::release()
{
    size_t alignmentSize = sizeof(Block);
    size_t totalSize = sizeof(Chunk) + alignmentSize;
    munmap(mmapAddr, totalSize);
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