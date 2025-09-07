// #pragma once
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum
{
    MARK = 0x01,
    ROOT = 0x02,
    LEAF = 0x04
};

struct GCPtr
{
public:
    void*  ptr   = nullptr;
    size_t size  = 0;
    size_t hash  = 0;   // 存储原始哈希位置 + 1 (0表示空)
    int    flags = 0;

    GCPtr() = default;
    GCPtr(void* p, size_t sz, size_t h)
        : ptr(p)
        , size(sz)
        , hash(h)
    {
    }

    bool isEmpty() const { return hash == 0; }
    bool isOccupied() const { return hash != 0; }
    void clear()
    {
        ptr   = nullptr;
        size  = 0;
        hash  = 0;
        flags = 0;
    }
};

enum
{
    PRIMES_COUNT = 23
};

static const size_t PRIMES[PRIMES_COUNT] = {
    1,    5,     11,    23,    53,     101,    197,    389,     683,     1259,    2417,   4733,
    9371, 18617, 37097, 74093, 148073, 296099, 592019, 1100009, 2200013, 4400021, 8800019};

struct GC
{
private:
    GCPtr *   _items, *_frees;
    size_t    _nslots      = 0;
    size_t    _nitems      = 0;   // size
    size_t    _nfrees      = 0;
    size_t    _mitems      = 0;   // capacity
    uintptr_t _minptr      = UINTPTR_MAX;
    uintptr_t _maxptr      = 0;
    bool      _paused      = false;
    void*     _bottom      = nullptr;
    double    _sweepfactor = 0.5;

    size_t hash(void* ptr);
    size_t getPrimeSize(size_t size);
    size_t probeDistance(size_t current_index, size_t original_hash);


    GCPtr* searchPtrImpl(void* ptr);
    void   addPtrImpl(void* ptr, size_t size);
    void   removePtrImpl(void* ptr);

    int  rehash(size_t size);
    bool resizeMore();
    bool resizeLess();

    void markPtr(void* ptr);
    void markStack();
    void mark();
    void sweep();
    void run();

    void addPtr(void* ptr, size_t size);
    void removePtr(void* ptr);

public:
    void  start(void* stk);
    void  stop();
    void* alloc(size_t size);
};
