#include "../include/gc.hpp"

#include <stdio.h>
size_t GC::hash(void* ptr)
{
    uintptr_t ad = (uintptr_t)ptr;
    return (size_t)((13 * ad) ^ (ad >> 15));
}

size_t GC::getPrimeSize(size_t size)
{
    for (size_t i = 0; i < PRIMES_COUNT; ++i) {
        if (PRIMES[i] > size) {
            return PRIMES[i];
        }
    }
    return size * 2;
}

size_t GC::probeDistance(size_t current_index, size_t original_hash)
{
    const long distance = current_index - (original_hash - 1);
    return distance < 0 ? _nslots + distance : distance;
}

GCPtr* GC::searchPtrImpl(void* ptr)
{
    size_t currPos  = hash(ptr) % this->_nslots;
    size_t distance = 0;
    while (1) {
        auto curr = this->_items[currPos];
        if (curr.hash == 0 || distance > probeDistance(currPos, curr.hash)) {
            return nullptr;
        }
        if (curr.ptr == ptr) {
            return &this->_items[currPos];
        }
        currPos = (currPos + 1) % this->_nslots;
        distance++;
    }
    return nullptr;
}

void GC::addPtrImpl(void* ptr, size_t size)
{
    size_t currPos  = hash(ptr) % this->_nslots;
    size_t distance = 0;

    GCPtr insertEntry(ptr, size, currPos + 1), tmp;
    while (1) {
        auto curr = this->_items[currPos];
        if (curr.isEmpty()) {
            printf("add ptr %p\n", ptr);
            this->_items[currPos] = insertEntry;
            ++this->_nitems;
            return;
        }
        if (curr.ptr == ptr) return;

        const size_t currDistance = probeDistance(currPos, curr.hash);
        if (distance > currDistance) {
            tmp         = insertEntry;
            insertEntry = curr;
            curr        = tmp;
            distance    = currDistance;
        }

        currPos = (currPos + 1) % _nslots;
        ++distance;
    }
}

void GC::removePtrImpl(void* ptr)
{
    if (this->_nitems == 0) return;
    for (size_t i = 0; i < this->_nfrees; i++) {
        if (this->_frees[i].ptr == ptr) {
            this->_frees[i].ptr = nullptr;
        }
    }

    size_t currPos  = hash(ptr) % _nslots;
    size_t distance = 0;

    while (_items[currPos].isOccupied()) {
        auto& current = this->_items[currPos];
        if (current.ptr == ptr) break;
        if (distance > probeDistance(currPos, current.hash)) {
            return;
        }
        currPos = (currPos + 1) % this->_nslots;
        ++distance;
    }

    if (this->_items[currPos].isEmpty() || this->_items[currPos].ptr != ptr) return;

    this->_items[currPos].clear();
    --this->_nitems;

    size_t next_pos = (currPos + 1) % this->_nslots;
    while (this->_items[next_pos].isOccupied() &&
           probeDistance(next_pos, this->_items[next_pos].hash) > 0) {
        this->_items[currPos] = this->_items[next_pos];
        this->_items[next_pos].clear();
        currPos  = next_pos;
        next_pos = (next_pos + 1) % this->_nslots;
    }
}

int GC::rehash(size_t size)
{
    GCPtr* old_items = this->_items;
    size_t old_size  = this->_nslots;

    this->_nslots = size;
    this->_items  = (GCPtr*)calloc(this->_nslots, sizeof(GCPtr));

    if (this->_items == nullptr) {
        this->_nslots = old_size;
        this->_items  = old_items;
        return 0;
    }

    for (size_t i = 0; i < old_size; i++) {
        if (old_items[i].hash != 0) {
            addPtr(old_items[i].ptr, old_items[i].size);
        }
    }

    free(old_items);

    return 1;
}

bool GC::resizeMore()
{
    size_t new_size = getPrimeSize(this->_nitems);
    size_t old_size = this->_nslots;
    return (new_size > old_size) ? rehash(new_size) : 1;
}

bool GC::resizeLess()
{
    size_t new_size = getPrimeSize(this->_nitems);
    size_t old_size = this->_nslots;
    return (new_size < old_size) ? rehash(new_size) : 1;
}

void GC::markPtr(void* ptr)
{
    size_t currPos, distance;

    if ((uintptr_t)ptr < this->_minptr || (uintptr_t)ptr > this->_maxptr) return;

    currPos  = hash(ptr) % this->_nslots;
    distance = 0;

    while (1) {
        ;
        if (this->_items[currPos].isEmpty() ||
            distance > probeDistance(currPos, this->_items[currPos].hash)) {
            return;
        }
        if (ptr == this->_items[currPos].ptr) {

            if (this->_items[currPos].flags & MARK) return;
            this->_items[currPos].flags |= MARK;
            printf("mark ptr %p\n", ptr);
            if (this->_items[currPos].flags & LEAF) return;

            for (size_t k = 0; k < this->_items[currPos].size / sizeof(void*); k++) {
                markPtr(((void**)this->_items[currPos].ptr)[k]);
            }
            return;
        }
        currPos = (currPos + 1) % _nslots;
        distance++;
    }
}

void GC::markStack()
{
    void *stk, *bot, *top, *p;
    bot = this->_bottom;
    top = &stk;

    if (bot == top) {
        return;
    }

    if (bot < top) {
        for (p = top; p >= bot; p = ((char*)p) - sizeof(void*)) {
            markPtr(*((void**)p));
        }
    }

    if (bot > top) {
        for (p = top; p <= bot; p = ((char*)p) + sizeof(void*)) {
            markPtr(*((void**)p));
        }
    }
}

void GC::mark()
{
    jmp_buf env;
    void (GC::* volatile markStackFunc)() = &GC::markStack;

    if (this->_nitems == 0) return;

    for (size_t i = 0; i < this->_nslots; i++) {
        if (this->_items[i].hash == 0) continue;
        if (this->_items[i].flags & MARK) {
            continue;
        }
        if (this->_items[i].flags & ROOT) {
            this->_items[i].flags |= MARK;
            if (this->_items[i].flags & LEAF) {
                continue;
            }
            for (size_t k = 0; k < this->_items[i].size / sizeof(void*); k++) {
                markPtr(((void**)this->_items[i].ptr)[k]);
            }
            continue;
        }
    }
    memset(&env, 0, sizeof(jmp_buf));
    setjmp(env);
    (this->*markStackFunc)();
}

void GC::sweep()
{
    if (this->_nitems == 0) return;

    this->_nfrees = 0;
    for (size_t i = 0; i < this->_nslots; i++) {
        if (this->_items[i].hash == 0) continue;
        if (this->_items[i].flags & MARK) continue;
        if (this->_items[i].flags & ROOT) continue;
        this->_nfrees++;
    }

    this->_frees = (GCPtr*)realloc(this->_frees, sizeof(GCPtr) * this->_nfrees);

    for (size_t k = 0, i = 0; i < this->_nslots; i++) {
        if (this->_items[i].hash == 0) continue;
        if (this->_items[i].flags & MARK) continue;
        if (this->_items[i].flags & ROOT) continue;

        this->_frees[k] = this->_items[i];
        k++;

        size_t currPos = i;
        while (true) {
            size_t nextPos = (currPos + 1) % this->_nslots;
            auto   curr    = this->_items[nextPos];
            if (curr.isOccupied() && probeDistance(nextPos, curr.hash) > 0) {
                this->_items[currPos] = this->_items[nextPos];
                currPos               = nextPos;
            }
            else {
                break;
            }
        }
        this->_nitems--;
    }

    for (size_t i = 0; i < this->_nslots; i++) {
        if (this->_items[i].hash == 0) continue;
        if (this->_items[i].flags & MARK) this->_items[i].flags &= ~MARK;
    }

    resizeLess();
    this->_mitems = this->_nitems + (size_t)(this->_nitems * this->_sweepfactor) + 1;

    for (size_t i = 0; i < this->_nfrees; i++) {
        if (this->_frees[i].ptr) {
            printf("sweep ptr %p\n", this->_frees[i].ptr);
            free(this->_frees[i].ptr);
        }
    }

    free(this->_frees);
    this->_frees  = NULL;
    this->_nfrees = 0;
}

void GC::run()
{
    mark();
    sweep();
}

void GC::addPtr(void* ptr, size_t size)
{
    this->_maxptr =
        ((uintptr_t)ptr) + size > this->_maxptr ? ((uintptr_t)ptr) + size : this->_maxptr;
    this->_minptr = ((uintptr_t)ptr) < this->_minptr ? ((uintptr_t)ptr) : this->_minptr;

    if (resizeMore()) {
        addPtrImpl(ptr, size);
        if (!this->_paused && this->_nitems > this->_mitems) {
            run();
        }
    }
    else {
        this->_nitems--;
        free(ptr);
    }
    return;
}

void GC::removePtr(void* ptr)
{
    removePtrImpl(ptr);
    resizeLess();
    this->_mitems = this->_nitems + this->_nitems / 2 + 1;
}

void GC::start(void* stk)
{
    this->_bottom = stk;
}

void GC::stop()
{
    sweep();
    free(this->_items);
    free(this->_frees);
}

void* GC::alloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr != nullptr) addPtr(ptr, size);
    return ptr;
}
static GC gc;
extern "C" {
void gc_start(void* stk)
{
    gc.start(stk);
}

void gc_stop()
{
    gc.stop();
}

void* gc_alloc(size_t size)
{
    return gc.alloc(size);
}
}
struct A
{
    struct B* b;
};

struct B
{
    struct A* a;
};

void example_function()
{
    struct A* a = (struct A*)gc_alloc(sizeof(struct A));
    struct B* b = (struct B*)gc_alloc(sizeof(struct B));

    // struct A* a = (struct A*)malloc(sizeof(struct A));
    // struct B* b = (struct B*)malloc(sizeof(struct B));
    a->b        = b;
    b->a        = a;
    // free(a);
    // free(b);
    return;
}

int main(int argc, char** argv)
{
    gc_start(&argc);
    example_function();
    gc_stop();
    return 0;
}