// =============================================================================
// kernel/memory/heap.cpp -- Kernel heap allocator implementation
// =============================================================================
// Design: a single contiguous arena (reserved statically in .bss, since this
// kernel does not yet enable paging -- see docs/ARCHITECTURE.md "Memory
// Management" for the roadmap to a paged, dynamically-growing heap) managed
// as a doubly-linked list of blocks, each either free or allocated. This is
// the classic "boundary tag" free-list allocator design.
// =============================================================================
#include "heap.h"
#include "libc/stdio.h"

namespace {

constexpr size_t HEAP_SIZE = 8 * 1024 * 1024; // 8 MiB kernel heap
alignas(16) uint8_t heap_arena[HEAP_SIZE];

struct BlockHeader {
    size_t size;         // size of the usable region that follows this header
    bool   free;
    BlockHeader* next;
    BlockHeader* prev;
};

BlockHeader* heap_head = nullptr;

constexpr size_t HEADER_SIZE = sizeof(BlockHeader);

void split_block(BlockHeader* block, size_t size) {
    if (block->size <= size + HEADER_SIZE + 16) return; // not worth splitting

    BlockHeader* new_block = (BlockHeader*)((uint8_t*)(block + 1) + size);
    new_block->size = block->size - size - HEADER_SIZE;
    new_block->free = true;
    new_block->next = block->next;
    new_block->prev = block;

    if (block->next) block->next->prev = new_block;
    block->next = new_block;
    block->size = size;
}

void coalesce(BlockHeader* block) {
    if (block->next && block->next->free) {
        block->size += HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    if (block->prev && block->prev->free) {
        coalesce(block->prev);
    }
}

} // namespace

namespace heap {

void init() {
    heap_head = (BlockHeader*)heap_arena;
    heap_head->size = HEAP_SIZE - HEADER_SIZE;
    heap_head->free = true;
    heap_head->next = nullptr;
    heap_head->prev = nullptr;
    kprintf("[heap] initialized, %u KiB arena\n", (unsigned)(HEAP_SIZE / 1024));
}

void* kmalloc(size_t size) {
    // Align allocations to 16 bytes -- keeps things simple and avoids
    // unaligned-access issues for any struct stored on the heap.
    size = (size + 15) & ~15u;

    for (BlockHeader* block = heap_head; block; block = block->next) {
        if (block->free && block->size >= size) {
            split_block(block, size);
            block->free = false;
            return (void*)(block + 1);
        }
    }

    kprintf("[heap] OUT OF MEMORY requesting %u bytes\n", (unsigned)size);
    return nullptr;
}

void kfree(void* ptr) {
    if (!ptr) return;
    BlockHeader* block = (BlockHeader*)ptr - 1;
    block->free = true;
    coalesce(block);
}

} // namespace heap

void* operator new(size_t size)           { return heap::kmalloc(size); }
void* operator new[](size_t size)         { return heap::kmalloc(size); }
void  operator delete(void* ptr) noexcept   { heap::kfree(ptr); }
void  operator delete[](void* ptr) noexcept { heap::kfree(ptr); }
void  operator delete(void* ptr, size_t) noexcept   { heap::kfree(ptr); }
void  operator delete[](void* ptr, size_t) noexcept { heap::kfree(ptr); }
