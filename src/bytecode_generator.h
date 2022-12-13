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
    TAG,

    // Stack handling
    PUSH,
    POP,

    BYTECODE_COUNT,
};

internal void print(BYTECODE_INSTRUCTION instruction) {
    switch (instruction) {
        case BYTECODE_NULL: {printf("BYTECODE_NULL");} break;
        case MOVI: {printf("MOVI");} break;
        case MOVR: {printf("MOVR");} break;
        case MOVM: {printf("MOVM");} break;
        case ADDI: {printf("ADDI");} break;
        case SUBI: {printf("SUBI");} break;
        case MULI: {printf("MULI");} break;
        case DIVI: {printf("DIVI");} break;
        case MODI: {printf("MODI");} break;
        case GTI: {printf("GTI");} break;
        case GTOEI: {printf("GTOEI");} break;
        case LTI: {printf("LTI");} break;
        case LTOEI: {printf("LTOEI");} break;
        case ADDF: {printf("ADDF");} break;
        case SUBF: {printf("SUBF");} break;
        case MULF: {printf("MULF");} break;
        case DIVF: {printf("DIVF");} break;
        case GTF: {printf("GTF");} break;
        case GTOEF: {printf("GTOEF");} break;
        case LTF: {printf("LTF");} break;
        case LTOEF: {printf("LTOEF");} break;
        case EQ: {printf("EQ");} break;
        case NEQ: {printf("NEQ");} break;
        case LNOT: {printf("LNOT");} break;
        case LOR: {printf("LOR");} break;
        case LAND: {printf("LAND");} break;
        case BNOT: {printf("BNOT");} break;
        case BOR: {printf("BOR");} break;
        case BAND: {printf("BAND");} break;
        case JEQ: {printf("JEQ");} break;
        case JNEQ: {printf("JNEQ");} break;
        case TAG: {printf("TAG");} break;
        case PUSH: {printf("PUSH");} break;
        case POP: {printf("POP");} break;
        case BYTECODE_COUNT: {printf("BYTECODE_COUNT");} break;
        invalid_default_case_msg("missing instruction type in print");
    }
}

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
    // Tag is just a number but in the dissasembly is like tag_<tag>
    u32 tag;
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

internal void print(Operand o) {
    printf("%lld", o._u64);
}

struct Bytecode_instruction {
    BYTECODE_INSTRUCTION instruction;
    Operand a;
    Operand b;
};

// #define BYTECODE_FIRST_SIZE KILOBYTES(1)
#define BYTECODE_FIRST_SIZE 1

struct Bytecode_generator {
    Memory_pool *allocator;

    Type_spec_table *type_table;
    Operator_table *operator_table;

    u32 stack_head;
    u64 bytecode_size;
    u64 bytecode_head;
    Bytecode_instruction *bytecode;
};

internal Bytecode_generator *new_bytecode_generator(Memory_pool *allocator, Type_spec_table *type_table, Operator_table *operator_table) {
    Bytecode_generator *result = push_struct(allocator, Bytecode_generator);

    result->allocator = allocator;
    result->type_table = type_table;
    result->operator_table = operator_table;
    result->stack_head = 0;
    result->bytecode_size = BYTECODE_FIRST_SIZE;
    result->bytecode_head = 0;
    result->bytecode = push_array(result->allocator, Bytecode_instruction, result->bytecode_size);

    return result;
}

internal Bytecode_instruction *next(Bytecode_generator *generator) {
    Bytecode_instruction *result = 0;

    if (generator->bytecode_head >= generator->bytecode_size) {
        u64 new_size = generator->bytecode_size * 2;
        Bytecode_instruction *new_bytecode = push_array_copy(generator->allocator, Bytecode_instruction, new_size, generator->bytecode);

        generator->bytecode_size = new_size;
        generator->bytecode = new_bytecode;
    }

    result = generator->bytecode + generator->bytecode_head++;

    return result;
}

internal void print_bytecode(Bytecode_generator *generator) {
    sfor_count(generator->bytecode, generator->bytecode_head) {
        putchar(' ');
        putchar(' ');
        print(it->instruction);
        putchar(' ');
        print(it->a);
        putchar(',');
        putchar(' ');
        print(it->b);
        putchar('\n');
    }
}


