
#include "../include/allocator.hpp"
#include "../include/common.hpp"


Allocator allocator;


int main()
{
    allocator.initialize();
    static_assert(sizeof(Block) == Constant::BlockSizeInBytes,
                  "Size of Block doesn't match expected size");

    int* ptr = (int*)allocator.malloc(sizeof(int));
    *ptr     = 521;
    std::cout << *ptr << "\n";
    allocator.release();
    return 0;
}