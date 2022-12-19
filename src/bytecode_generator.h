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

internal BYTECODE_INSTRUCTION ast_type_to_arithmetic_bytecode_instruction(AST_EXPRESSION_TYPE type) {
    switch(type) {
        case AST_EXPRESSION_BINARY_ADD: {
            return ADDI;
        } break;

        case AST_EXPRESSION_BINARY_SUB: {
            return SUBI;
        } break;

        case AST_EXPRESSION_BINARY_MUL: {
            return MULI;
        } break;

        case AST_EXPRESSION_BINARY_DIV: {
            return DIVI;
        } break;

        case AST_EXPRESSION_BINARY_MOD: {
            return MODI;
        } break;

        invalid_default_case_msg("invalid ast_expression_type for arithmetic bytecode generation");
    }

    return BYTECODE_NULL;
}

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

// TODO: when executing the bytecode instead of using the register number use something like this register = R0 + (op.r % REGISTER_COUNT) (because there is only 20 registers)
// TODO: later maybe do a register allocator
enum _REGISTER :u32 {
    REGISTER_NULL = 0,

    RBP = 1,

    R0  = 2,
    R1  = 3,
    R2  = 4,
    R3  = 5,
    R4  = 6,
    R5  = 7,
    R6  = 8,
    R7  = 9,
    R8  = 10,
    R9  = 11,
    R10 = 12,
    R11 = 13,
    R12 = 14,
    R13 = 15,
    R14 = 16,
    R15 = 17,
    R16 = 18,
    R17 = 19,
    R18 = 20,
    R19 = 21,

    REGISTER_COUNT,
};

internal char *to_string(_REGISTER r) {
    switch (r) {
        case REGISTER_NULL: {return "REGISTER_NULL";} break;

        case R0: {return "R0";} break;
        case R1: {return "R1";} break;
        case R2: {return "R2";} break;
        case R3: {return "R3";} break;
        case R4: {return "R4";} break;
        case R5: {return "R5";} break;
        case R6: {return "R6";} break;
        case R7: {return "R7";} break;
        case R8: {return "R8";} break;
        case R9: {return "R9";} break;
        case R10: {return "R10";} break;
        case R11: {return "R11";} break;
        case R12: {return "R12";} break;
        case R13: {return "R13";} break;
        case R14: {return "R14";} break;
        case R15: {return "R15";} break;
        case R16: {return "R16";} break;
        case R17: {return "R17";} break;
        case R18: {return "R18";} break;
        case R19: {return "R19";} break;
        case RBP: {return "RBP";} break;

        case REGISTER_COUNT: {return "REGISTER_COUNT";} break;
        invalid_default_case_msg("unhandled register in to_string");
    }

    return 0;
}

enum OPERAND_TYPE {
    OPERAND_NULL,

    TAG_ID,
    REGISTER_ID,

    // The _ is because windows already has defined these names
    _BYTE,
    _WORD,
    _DWORD,
    _QWORD,

    ADDRESS,

    OPERAND_COUNT,
};

struct Address {
    u32 r;
    s32 offset;
};

internal Address new_address(u32 r, s32 offset) {
    Address result;
    result.r = r;
    result.offset = offset;
    return result;
}

struct Operand {
    OPERAND_TYPE type;

    union {
        // Tag is just a number but in the dissasembly is like tag_<tag>
        u32 tag;
        // REGISTER r;
        u32 r;

        u8  byte;
        u16 word;
        u32 dword;
        u64 qword;

        f32 _f32;
        f64 _f64;

        Address _address;
    };
};

// internal Operand new_operand(OPERAND_TYPE type, REGISTER r) {
//     Operand result;
//     result.type = type;
//     result.r = r;
//     return result;
// }

internal Operand new_operand(OPERAND_TYPE type, u8  byte) {
    Operand result;
    result.type = type;
    result.byte = byte;
    return result;
}

internal Operand new_operand(OPERAND_TYPE type, u16 word) {
    Operand result;
    result.type = type;
    result.word = word;
    return result;
}

internal Operand new_operand(OPERAND_TYPE type, u32 tag_or_dword_or_register) {
    Operand result;
    result.type = type;
    result.dword = tag_or_dword_or_register;
    return result;
}

internal Operand new_operand(OPERAND_TYPE type, u64 qword) {
    Operand result;
    result.type = type;
    result.qword = qword;
    return result;
}

internal Operand new_operand(OPERAND_TYPE type, f32 _f32) {
    Operand result;
    result.type = type;
    result._f32 = _f32;
    return result;
}

internal Operand new_operand(OPERAND_TYPE type, f64 _f64) {
    Operand result;
    result.type = type;
    result._f64 = _f64;
    return result;
}

internal Operand new_operand(OPERAND_TYPE type, Address _address) {
    Operand result;
    result.type = type;
    result._address = _address;
    return result;
}


internal Operand new_register_operand(u32 r) {
    Operand result = new_operand(REGISTER_ID, r);
    return result;
}

internal void print(Operand o) {
    Address a = o._address;
    char sign = a.offset >= 0 ? '+' : ' ';

    switch (o.type) {
        case OPERAND_NULL: {PEYOT_ERROR("this should not be printed");} break;

        case      TAG_ID: {printf("tag_%u",     o.tag);} break;
        case REGISTER_ID: {printf("r%u",        o.r);} break;
        case       _BYTE: {printf("%u",         o.byte);} break;
        case       _WORD: {printf("%u",         o.word);} break;
        case      _DWORD: {printf("%u",         o.dword);} break;
        case      _QWORD: {printf("%llu",       o.qword);} break;
        case     ADDRESS: {
            switch (a.r) {
                case RBP: {
                    printf("[RBP %c %d]", sign, a.offset);
                } break;
                default: {
                    printf("[R%u %c %d]", a.r, sign, a.offset);
                } break;
            }
        } break;

        case OPERAND_COUNT: {PEYOT_ERROR("this should not be printed");} break;

        invalid_default_case_msg("print operand unhandled type");
    }
}

struct Bytecode_instruction {
    BYTECODE_INSTRUCTION instruction;
    Operand destination;
    Operand source;
};

#if DEVELOPMENT
#define BYTECODE_FIRST_SIZE 1
#else
#define BYTECODE_FIRST_SIZE KILOBYTES(1)
#endif

struct Bytecode_generator {
    Memory_pool *allocator;

    Type_spec_table *type_table;
    Operator_table *operator_table;
    Symbol_table *current_scope;

    u32 current_register;

    u32 stack_head;
    u64 bytecode_size;
    u64 bytecode_head;
    Bytecode_instruction *bytecode;
};

internal u64 push_stack(Bytecode_generator *generator, u64 size) {
    generator->stack_head += size;
    return generator->stack_head - size;
}

internal Bytecode_generator *new_bytecode_generator(Memory_pool *allocator, Type_spec_table *type_table, Operator_table *operator_table) {
    Bytecode_generator *result = push_struct(allocator, Bytecode_generator);

    result->allocator = allocator;
    result->type_table = type_table;
    result->operator_table = operator_table;
    result->current_register = R0;
    result->stack_head = 0;
    result->bytecode_size = BYTECODE_FIRST_SIZE;
    result->bytecode_head = 0;
    result->bytecode = push_array(result->allocator, Bytecode_instruction, result->bytecode_size);

    return result;
}

internal u32 new_register(Bytecode_generator *generator) {
    u32 result = generator->current_register++;
    return result;
}

internal Bytecode_instruction *next(Bytecode_generator *generator) {
    Bytecode_instruction *result = 0;

    if (generator->bytecode_head >= generator->bytecode_size) {
        u64 new_size = generator->bytecode_size * 2;
        debug("resizing bytecode buffer")
        debug(new_size)
        Bytecode_instruction *new_bytecode = push_array_copy(generator->allocator, Bytecode_instruction, new_size, generator->bytecode);

        generator->bytecode_size = new_size;
        generator->bytecode = new_bytecode;
    }

    result = generator->bytecode + generator->bytecode_head++;

    return result;
}

internal void print_bytecode(Bytecode_generator *generator) {
    u32 max_length = 0;
    u32 head = generator->bytecode_head;

    while (head) {
        max_length++;
        head /= 10;
    }

    debug(max_length)

    sfor_count(generator->bytecode, generator->bytecode_head) {
        u32 line_length = 0;
        u32 line = i;

        while (line) {
            line_length++;
            line /= 10;
        }

        line_length = al_max(line_length, 1);

        print_indent(max_length - line_length);
        printf("%d:  ", i);
        print(it->instruction);
        putchar(' ');
        print(it->destination);
        putchar(',');
        putchar(' ');
        print(it->source);
        putchar('\n');
    }
}


