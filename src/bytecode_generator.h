enum BYTECODE_INSTRUCTION {
    BYTECODE_NULL,

    // Load inmediate
    MOVI,
    // Load from register
    MOVR,
    // Load from memory
    MOVM,

    ADDI,
    SUBI,
    MULI,
    DIVI,
    MODI,
    GTI,
    GTOEI,
    LTI,
    LTOEI,

    ADDF,
    SUBF,
    MULF,
    DIVF,
    GTF,
    GTOEF,
    LTF,
    LTOEF,

    EQ,
    NEQ,
    // Can these logical/bitwise be merged?
    LNOT,
    LOR,
    LAND,
    BNOT,
    BOR,
    BAND,

    // Jump if equals and if not equals
    JEQ,
    JNEQ,

    BYTECODE_COUNT,
};

enum REGISTER {
    REGISTER_NULL = 0,

    R0  = 1,
    R1  = 2,
    R2  = 3,
    R3  = 4,
    R4  = 5,
    R5  = 6,
    R6  = 7,
    R7  = 8,
    R8  = 9,
    R9  = 10,
    R10 = 11,
    R11 = 12,
    R12 = 13,
    R13 = 14,
    R14 = 15,
    R15 = 16,
    R16 = 17,
    R17 = 18,
    R18 = 19,
    R19 = 20,

    REGISTER_COUNT,
};

union Operand {
    str tag;
    REGISTER r;

    u8  _u8;
    u16 _u16;
    u32 _u32;
    u64 _u64;

    s8  _s8;
    s16 _s16;
    s32 _s32;
    s64 _s64;

    f32 _f32;
    f64 _f64;

    u64 _address;
};

struct Bytecode_instruction {
    BYTECODE_INSTRUCTION intruction;
    Operand a;
    Operand b;
};

#define BYTECODE_FIRST_SIZE KILOBYTES(1)

struct Bytecode_generator {
    Memory_pool *allocator;
    u64 bytecode_size;
    u64 bytecode_head;
    Bytecode_instruction *bytecode;
};

internal Bytecode_generator *new_bytecode_generator(Memory_pool *allocator) {
    Bytecode_generator *result = push_struct(allocator, Bytecode_generator);

    result->allocator = allocator;
    result->bytecode_size = BYTECODE_FIRST_SIZE;
    result->bytecode_head = 0;
    result->bytecode = push_array(result->allocator, Bytecode_instruction, result->bytecode_size);

    return result;
}

// TODO: have a realloc for when the bytecode has to grow, allocate the double of the current size and copy the current content into the new buffer
