
#include "../include/allocator.hpp"
#include "../include/common.hpp"


Allocator allocator;


int main()
{
    allocator.init();
    std::cout << allocator.chunk << "\n";
    static_assert(sizeof(Block) == Constant::BlockSizeInBytes,
                  "Size of Block doesn't match expected size");
    std::cout << sizeof(*allocator.chunk) << "\n";
    std::cout << Constant::ObjectHeaderSizeInBytes << "\n";
    // block->initialize();
    int* ptr = (int*)allocator.malloc(sizeof(int));
    if (ptr != nullptr) {
        *ptr = 521;
        std::cout << *ptr << "\n";
    }
    else {
        std::cout << "Failed to allocate memory\n";
    }
    allocator.release();
    return 0;
}