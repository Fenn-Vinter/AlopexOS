#include <new.hpp>

// Define a static heap buffer inside the high-half kernel image
static constexpr u64 heap_size = 16 * 1024 * 1024;
alignas(16) static u8 heap_buffer[heap_size];
static u8* heap_start = heap_buffer;

struct BlockHeader {
    u64 size;           
    bool is_free;       
    BlockHeader* next;  
    BlockHeader* prev;  
};

static constexpr u64 HEADER_SIZE = sizeof(BlockHeader);
static constexpr u64 ALIGNMENT = 16;

static BlockHeader* free_list_head = nullptr;
static bool heap_initialized = false;

static u64 align_up(u64 size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

static void init_heap() {
    if (heap_initialized) return;
    
    BlockHeader* initial_block = reinterpret_cast<BlockHeader*>(heap_start);
    initial_block->size = heap_size - HEADER_SIZE;
    initial_block->is_free = true;
    initial_block->next = nullptr;
    initial_block->prev = nullptr;
    
    free_list_head = initial_block;
    heap_initialized = true;
}

static void remove_from_free_list(BlockHeader* block) {
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        free_list_head = block->next;
    }
    
    if (block->next) {
        block->next->prev = block->prev;
    }
    
    block->next = nullptr;
    block->prev = nullptr;
}

static void add_to_free_list(BlockHeader* block) {
    block->is_free = true;
    block->next = free_list_head;
    block->prev = nullptr;
    
    if (free_list_head) {
        free_list_head->prev = block;
    }
    free_list_head = block;
}

static void coalesce(BlockHeader* block) {
    u8* next_block_addr = reinterpret_cast<u8*>(block) + HEADER_SIZE + block->size;
    u8* heap_end = heap_start + heap_size;

    if (next_block_addr < heap_end) {
        BlockHeader* next_block = reinterpret_cast<BlockHeader*>(next_block_addr);
        if (next_block->is_free) {
            block->size += HEADER_SIZE + next_block->size;
            remove_from_free_list(next_block);
        }
    }
}

static void* allocate_memory(u64 size) {
    if (!heap_initialized) init_heap();

    u64 aligned_size = align_up(size);
    if (aligned_size < 1) aligned_size = 1;

    BlockHeader* current = free_list_head;

    while (current) {
        if (current->is_free && current->size >= aligned_size) {
            u64 remaining = current->size - aligned_size;
            if (remaining > HEADER_SIZE + ALIGNMENT) {
                BlockHeader* new_block = reinterpret_cast<BlockHeader*>(
                    reinterpret_cast<u8*>(current) + HEADER_SIZE + aligned_size
                );
                
                new_block->size = remaining - HEADER_SIZE;
                new_block->is_free = false;
                
                current->size = aligned_size;
                
                add_to_free_list(new_block);
            }

            remove_from_free_list(current);
            current->is_free = false;
            return reinterpret_cast<u8*>(current) + HEADER_SIZE;
        }
        current = current->next;
    }

    return nullptr;
}

static void free_memory(void* ptr) {
    if (!ptr) return;

    BlockHeader* block = reinterpret_cast<BlockHeader*>(
        reinterpret_cast<u8*>(ptr) - HEADER_SIZE
    );

    if (block->is_free) return;

    add_to_free_list(block);
    coalesce(block);
}

void* operator new(size_t size) {
    void* ptr = allocate_memory(size);
    if (!ptr) {
        asm volatile("hlt");
        __builtin_unreachable();
    }
    return ptr;
}

void* operator new[](size_t size) {
    return operator new(size);
}

void* operator new(size_t, void* ptr) noexcept {
    return ptr;
}

void* operator new[](size_t, void* ptr) noexcept {
    return ptr;
}

void operator delete(void* ptr) noexcept {
    free_memory(ptr);
}

void operator delete[](void* ptr) noexcept {
    free_memory(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    free_memory(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    free_memory(ptr);
}