struct Memory_block_chain {
    u8 *base_address;
    umm size;
    umm used;
    umm pad;
};

struct Memory_pool {
    size_t size;
    u8 *base_address;
    size_t used;

    size_t minimum_block_size;

    u32 block_count;
    s32 temporary_count;
};

struct Temporary_memory {
    Memory_pool *pool;
    u8 *base_address;
    size_t used;
};

enum ARENA_PUSH_FLAG {
    // the default behavior is to clear to zero
    ARENA_FLAG_CLEAR_TO_ZERO = 1<<0,
};

struct Arena_push_params {
    u32 flags;
    u32 alignment;
};

inline Arena_push_params default_pool_parans(void) {
    Arena_push_params result;

    result.flags = ARENA_FLAG_CLEAR_TO_ZERO;
    result.alignment = 4;

    return result;
}

inline Arena_push_params align_clear(u32 alignment) {
    Arena_push_params result;

    result.flags = ARENA_FLAG_CLEAR_TO_ZERO;
    result.alignment = alignment;

    return result;
}

inline Arena_push_params no_clear() {
    Arena_push_params result = default_pool_parans();

    result.flags &= ~ARENA_FLAG_CLEAR_TO_ZERO;

    return result;
}

inline Arena_push_params align_no_clear(u32 alignment) {
    Arena_push_params result = no_clear();

    result.alignment = alignment;

    return result;
}

inline void set_minimum_block_size(Memory_pool *pool, size_t minimum_block_size) {
    pool->minimum_block_size = minimum_block_size;
}

inline Memory_block_chain *get_footer(Memory_pool *pool) {
    Memory_block_chain *result = (Memory_block_chain *)(pool->base_address + pool->size);
    return result;
}

inline void free_last_block(Memory_pool *pool) {
    void *free = pool->base_address;

    Memory_block_chain *footer = get_footer(pool);

    pool->base_address = footer->base_address;
    pool->size = footer->size;
    pool->used = footer->used;

    free_memory(free);

    pool->block_count--;
}

inline void clear(Memory_pool *pool) {
    while (pool->block_count > 0) {
        free_last_block(pool);
    }
}

inline address get_alignment_offset(Memory_pool *pool, address alignment) {
    address result = 0;
    address result_address = (address)(pool->base_address + pool->used);
    address alignment_mask = alignment - 1;

    if (result_address & alignment_mask) {
        result = alignment - (result_address & alignment_mask);
    }

    return result;
}

#define push_struct(pool, type, ...) (type *) _push_size(pool, sizeof(type), ##__VA_ARGS__)
#define push_array(pool, count, type, ...) (type *) _push_size(pool, (count) * sizeof(type), ##__VA_ARGS__)
#define push_size(pool, size, ...) _push_size(pool, size, ## __VA_ARGS__)
#define push_copy(pool, size, source, ...) copy(size, source, _push_size(pool, size, ##__VA_ARGS__))

inline address get_remaining_size(Memory_pool *pool, Arena_push_params params=default_pool_parans()) {
    address result = pool->size - (pool->used + get_alignment_offset(pool, params.alignment));
    return result;
}

inline memory_size get_effective_size_for(Memory_pool *pool, memory_size _size, memory_size alignment) {
    memory_size size = _size;
    memory_size alignment_offset = get_alignment_offset(pool, alignment);
    size += alignment_offset;

    return size;
}

inline void *_push_size(Memory_pool *pool, size_t _size, Arena_push_params params=default_pool_parans()) {
    void *result = 0;

    memory_size size = get_effective_size_for(pool, _size, params.alignment);

    if ((pool->used + size) > pool->size) {
        if (!pool->minimum_block_size) {
            pool->minimum_block_size = MEGABYTES(1);
        }

        Memory_block_chain save;
        save.base_address = pool->base_address;
        save.size = pool->size;
        save.used = pool->used;

        size = _size;
        memory_size block_size = al_max(size + sizeof(Memory_block_chain), pool->minimum_block_size);
        pool->size = block_size - sizeof(Memory_block_chain);
        pool->base_address = (u8 *)allocate_memory(block_size);
        pool->used = 0;
        pool->block_count++;

        Memory_block_chain *footer = get_footer(pool);
        *footer = save;
    }

    assert((pool->used + size) <= pool->size, "memory_pool overflow");

    memory_size alignment_offset = get_alignment_offset(pool, params.alignment);

    result = (pool->base_address + pool->used + alignment_offset);
    pool->used += size;

    if (params.flags & ARENA_FLAG_CLEAR_TO_ZERO) {
        zero_memory(_size, result);
    }

    return result;
}

inline Temporary_memory begin_temporary_memory(Memory_pool *pool) {
    Temporary_memory result;

    result.pool = pool;
    result.base_address = pool->base_address;
    result.used = pool->used;

    pool->temporary_count++;

    return result;
}

inline void end_temporary_memory(Temporary_memory tm) {
    Memory_pool *pool = tm.pool;

    while (pool->base_address != tm.base_address) {
        free_last_block(pool);
    }

    assert(pool->used >= tm.used, "temporary memory requested is greater than the pool capacity");
    assert(pool->temporary_count > 0, "temporary memory count less or equal to zero");
    pool->used = tm.used;
    pool->temporary_count--;
}

inline void check_pool(Memory_pool *pool) {
    assert(pool->temporary_count == 0, "temporary memory begin/end missmatch");
}
