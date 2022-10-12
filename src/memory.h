inline void zero_memory(size_t size, void *memory) {
    u8 *byte = (u8 *)memory;

    while (size--) {
        *byte++ = 0;
    }
}

inline void *copy(memory_size size, void *_source, void *_destiny) {
    u8 *source = (u8 *)_source;
    u8 *destiny = (u8 *)_destiny;

    while (size--) {
        *destiny++ = *source++;
    }

    return _destiny;
}

#define zero_struct(instance) zero_memory(sizeof(instance), &(instance))
#define zero_array(count, pointer) zero_size(count * sizeof((pointer)[0]), pointer)
