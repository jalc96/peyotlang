enum BYTECODE_INSTRUCTION {
    NOP,

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
    JUMP,
    JEQ,
    JNEQ,
    TAG,
    FTAG,
    CALL,
    RET,
    LEAVE,

    // Stack handling
    PUSH,
    POP,

    BYTECODE_COUNT,
};

internal BYTECODE_INSTRUCTION ast_binary_type_to_bytecode_instruction(AST_EXPRESSION_TYPE type, bool is_float) {
    switch(type) {
        case AST_EXPRESSION_BINARY_ADD: {return is_float ? ADDF : ADDI;} break;
        case AST_EXPRESSION_BINARY_SUB: {return is_float ? SUBF : SUBI;} break;
        case AST_EXPRESSION_BINARY_MUL: {return is_float ? MULF : MULI;} break;
        case AST_EXPRESSION_BINARY_DIV: {return is_float ? DIVF : DIVI;} break;
        case AST_EXPRESSION_BINARY_MOD: {return MODI;} break;
        case AST_EXPRESSION_BINARY_EQUALS: {return EQ;} break;
        case AST_EXPRESSION_BINARY_NOT_EQUALS: {return NEQ;} break;
        case AST_EXPRESSION_BINARY_GREATER_THAN: {return is_float ? GTF : GTI;} break;
        case AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS: {return is_float ? GTOEF : GTOEI;} break;
        case AST_EXPRESSION_BINARY_LESS_THAN: {return is_float ? LTF : LTI;} break;
        case AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS: {return is_float ? LTOEF : LTOEI;} break;
        case AST_EXPRESSION_BINARY_LOGICAL_OR: {return LOR;} break;
        case AST_EXPRESSION_BINARY_LOGICAL_AND: {return LAND;} break;
        case AST_EXPRESSION_BINARY_BITWISE_OR: {return BOR;} break;
        case AST_EXPRESSION_BINARY_BITWISE_AND: {return BAND;} break;

        invalid_default_case_msg("invalid ast_expression_type for arithmetic bytecode generation");
    }

    return NOP;
}

internal void print(BYTECODE_INSTRUCTION instruction) {
    switch (instruction) {
        case NOP: {printf("NOP");} break;
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
        case JUMP: {printf("JUMP");} break;
        case JEQ: {printf("JEQ");} break;
        case JNEQ: {printf("JNEQ");} break;
        case TAG: {printf("TAG");} break;
        case FTAG: {printf("FTAG");} break;
        case PUSH: {printf("PUSH");} break;
        case POP: {printf("POP");} break;
        case BYTECODE_COUNT: {printf("BYTECODE_COUNT");} break;
        case CALL: {printf("CALL");} break;
        case RET: {printf("RET");} break;
        case LEAVE: {printf("LEAVE");} break;
        invalid_default_case_msg("missing instruction type in print");
    }
}

// The R0 register mus not be used never for things other than return values
// TODO: when executing the bytecode instead of using the register number use something like this register = R1 + (op.r % REGISTER_COUNT) (because there is only 20 registers)
// TODO: later maybe do a register allocator
enum _REGISTER :u32 {
    REGISTER_NULL = 0,

    RBP = 1,
    RSP,
    // String memory pool pointer
    SMP,

    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    R16,
    R17,
    R18,
    R19,

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

    FUNCTION_NAME,

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

struct Tag {
    u32 id;
};

struct Operand {
    OPERAND_TYPE type;

    union {
        // Tag is just a number but in the dissasembly is like tag_<tag>
        Tag tag;
        // REGISTER r;
        u32 r;

        u8  byte;
        u16 word;
        u32 dword;
        u64 qword;

        f32 _f32;
        f64 _f64;

        Address _address;

        str *function_name;
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

internal Operand new_operand(OPERAND_TYPE type, Tag tag) {
    Operand result;
    result.type = type;
    result.tag = tag;
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

internal Operand new_operand(OPERAND_TYPE type, str *function_name) {
    Operand result;
    result.type = type;
    result.function_name = function_name;
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

        case      TAG_ID: {printf("tag_%u",     o.tag.id);} break;
        case REGISTER_ID: {
            switch (a.r) {
                case RBP: {
                    printf("rbp");
                } break;
                case RSP: {
                    printf("rsp");
                } break;
                case SMP: {
                    printf("smp");
                } break;
                default: {
                    printf("r%u", o.r);
                } break;
            }
        } break;
        case       _BYTE: {printf("%u",         o.byte);} break;
        case       _WORD: {printf("%u",         o.word);} break;
        case      _DWORD: {printf("%u",         o.dword);} break;
        case      _QWORD: {printf("%llu",       o.qword);} break;
        case     ADDRESS: {
            switch (a.r) {
                case RBP: {
                    printf("[RBP %c %d]", sign, a.offset);
                } break;
                case RSP: {
                    printf("[RSP %c %d]", sign, a.offset);
                } break;
                case SMP: {
                    printf("[SMP %c %d]", sign, a.offset);
                } break;
                default: {
                    printf("[R%u %c %d]", a.r, sign, a.offset);
                } break;
            }
        } break;

        case FUNCTION_NAME: {
            str n = *(o.function_name);
            printf("%s", n.buffer);
        } break;

        case OPERAND_COUNT: {PEYOT_ERROR("this should not be printed");} break;

        invalid_default_case_msg("print operand unhandled type");
    }
}

#if DEVELOPMENT
#define TAG_OFFSET_HASH_TABLE_SIZE 8
#else
#define TAG_OFFSET_HASH_TABLE_SIZE KILOBYTES(1)
#endif

struct Tag_offset {
    u32 id;
    u32 bytecode_offset;
    Tag_offset *next;
};

internal Tag_offset *new_tag_offset(Memory_pool *allocator, u32 id, u32 bytecode_offset) {
    Tag_offset *result = push_struct(allocator, Tag_offset);

    result->id = id;
    result->bytecode_offset = bytecode_offset;

    return result;
}

internal void print(Tag_offset *tag) {
    u32 bo = tag->bytecode_offset;
    printf("tag_%u: %u\n", tag->id, bo);
}

internal void print_entire_list(Tag_offset *tag, u32 indent=0) {
    lfor(tag) {
        print_indent(indent);
        print(it);
        indent += 2;
    }
}

struct Tag_offset_hash_table {
    Tag_offset *table[TAG_OFFSET_HASH_TABLE_SIZE];
    Memory_pool *allocator;
};

internal Tag_offset_hash_table *new_tag_offset_hash_table(Memory_pool *allocator) {
    Tag_offset_hash_table *result = push_struct(allocator, Tag_offset_hash_table);
    result->allocator = allocator;
    return result;
}

internal void print(Tag_offset_hash_table *table) {
    printf("---TAGS OFFSET---\n");

    sfor(table->table) {
        print_entire_list(*it, 4);
    }
}

internal u32 get_tag_offset_index(u32 id) {
    u32 result = id & (TAG_OFFSET_HASH_TABLE_SIZE - 1);
    return result;
}

internal void put(Tag_offset_hash_table *table, Tag_offset *tag) {
    u32 index = get_tag_offset_index(tag->id);
    tag->next = table->table[index];
    table->table[index] = tag;
}

internal Tag_offset *get(Tag_offset_hash_table *table, u32 id) {
    Tag_offset *result = 0;

    u32 index = get_tag_offset_index(id);

    lfor (table->table[index]) {
        if (it->id == id) {
            result = it;
            break;
        }
    }

    return result;
}

#if DEVELOPMENT
#define FUNCTION_OFFSET_HASH_TABLE_SIZE 8
#else
#define FUNCTION_OFFSET_HASH_TABLE_SIZE KILOBYTES(1)
#endif

struct Function_offset {
    str *name;
    u32 bytecode_offset;
    Function_offset *next;
};

internal Function_offset *new_function_offset(Memory_pool *allocator, str *name, u32 bytecode_offset) {
    Function_offset *result = push_struct(allocator, Function_offset);

    result->name = name;
    result->bytecode_offset = bytecode_offset;

    return result;
}

internal void print(Function_offset *function) {
    u32 bo = function->bytecode_offset;
    printf("%.*s: %u\n", STR_PRINT(*function->name), bo);
}

internal void print_entire_list(Function_offset *function, u32 indent=0) {
    lfor(function) {
        print_indent(indent);
        print(it);
        indent += 2;
    }
}

struct Function_offset_hash_table {
    Function_offset *table[FUNCTION_OFFSET_HASH_TABLE_SIZE];
    Memory_pool *allocator;
};

internal Function_offset_hash_table *new_function_offset_hash_table(Memory_pool *allocator) {
    Function_offset_hash_table *result = push_struct(allocator, Function_offset_hash_table);
    result->allocator = allocator;
    return result;
}

internal void print(Function_offset_hash_table *table) {
    printf("---FUNCTIONS OFFSET---\n");

    sfor(table->table) {
        print_entire_list(*it, 4);
    }
}

internal u32 get_function_offset_index(str *name) {
    u32 i = hash(*name);
    u32 result = i & (FUNCTION_OFFSET_HASH_TABLE_SIZE - 1);
    return result;
}

internal void put(Function_offset_hash_table *table, Function_offset *function) {
    u32 index = get_function_offset_index(function->name);
    function->next = table->table[index];
    table->table[index] = function;
}

internal Function_offset *get(Function_offset_hash_table *table, str *name) {
    Function_offset *result = 0;

    u32 index = get_function_offset_index(name);

    lfor (table->table[index]) {
        if (equals(it->name, name)) {
            result = it;
            break;
        }
    }

    return result;
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

template<class T> struct Regrowable_linear_buffer {
    Memory_pool *allocator;
    T *buffer;
    u32 size;
    u32 head;
    T *get_count(u32 count);
    T *next();
};

template<class T> T *Regrowable_linear_buffer<T>::get_count(u32 count) {
    T *result = 0;

    if ((head + count) >= size) {
        u64 new_size = al_max(size + count, size * 2);
        debug("resizing buffer")
        debug(new_size)
        T *new_buffer = push_array_copy(allocator, T, new_size, buffer);

        size = new_size;
        buffer = new_buffer;
    }

    result = buffer + head;
    head += count;

    return result;
}

template<class T> T *Regrowable_linear_buffer<T>::next() {
    T *result = get_count(1);
    return result;
}

template <class T> internal Regrowable_linear_buffer<T> *new_regrowable_linear_buffer(Memory_pool *allocator) {
    Regrowable_linear_buffer<T> *result = push_struct(allocator, Regrowable_linear_buffer<T>);

    result->allocator = allocator;
    result->size = BYTECODE_FIRST_SIZE;
    result->buffer = push_array(result->allocator, T, result->size);

    return result;
}

template<class T> struct Regrowable_linear_buffer_iterator {
    u32 index;
    Regrowable_linear_buffer<T> *buffer;
};

template<class T> internal Regrowable_linear_buffer_iterator<T> iterate(Regrowable_linear_buffer<T> *buffer) {
    Regrowable_linear_buffer_iterator<T> result;

    result.index = 0;
    result.buffer = buffer;

    return result;
}

template<class T> internal bool valid(Regrowable_linear_buffer_iterator<T> it) {
    bool result = it.index < it.buffer->head;
    return result;
}

template<class T> internal T *next(Regrowable_linear_buffer_iterator<T> *it, u32 size) {
    T *result = it->buffer->buffer + it->index;
    it->index += size;
    return result;
}

struct Memory_stack {
    u32 offset;
    Memory_stack *next;
};

struct Bytecode_generator {
    Memory_pool *allocator;

    Type_spec_table *type_table;
    Operator_table *operator_table;
    Symbol_table *current_scope;
    Native_operations_table *native_operations_table;
    Tag_offset_hash_table *tag_offset_table;
    Function_offset_hash_table *function_offset_table;

    u32 current_register;
    u32 current_tag;

// TODO: this is not necessary
    bool call_params_stack_check;
    u32 call_params_stack_head;

    u32 stack_head;
    Memory_stack *stack;
    Memory_stack *first_free;

    // u64 bytecode_size;
    // u64 bytecode_head;
    // Bytecode_instruction *bytecode;
    Regrowable_linear_buffer<u8> *string_pool;
    Regrowable_linear_buffer<Bytecode_instruction> *bytecode;
};

internal Tag new_tag(Bytecode_generator *generator) {
    Tag result;

    result.id = generator->current_tag++;

    return result;
}

internal Tag *new_tag_alloc(Bytecode_generator *generator) {
    Tag *result = push_struct(generator->allocator, Tag);

    result->id = generator->current_tag++;

    return result;
}

internal u64 push_stack(Bytecode_generator *generator, u64 size) {
    generator->stack_head += size;
    return generator->stack_head - size;
}

internal void begin_function_call_stack(Bytecode_generator *generator) {
    assert(generator->call_params_stack_check == false, "unpaired begin/end function_call_stack in begin");
    generator->call_params_stack_check = true;
    generator->call_params_stack_head = 0;
}

internal void end_function_call_stack(Bytecode_generator *generator) {
    assert(generator->call_params_stack_check, "unpaired begin/end function_call_stack in end");
    generator->call_params_stack_check = false;
}

internal void push_stack_call(Bytecode_generator *generator) {
    Memory_stack *stack_link = generator->first_free;

    if (!stack_link) {
        stack_link = push_struct(generator->allocator, Memory_stack);
    }

    stack_link->offset = generator->stack_head;
    stack_link->next = 0;
    generator->stack = stack_link;

}

internal void pop_stack_call(Bytecode_generator *generator) {
    Memory_stack *stack_link = generator->stack;
    generator->stack = stack_link->next;

    stack_link->next = generator->first_free;
    generator->first_free = stack_link;

    generator->stack_head = stack_link->offset;
}

internal Bytecode_generator *new_bytecode_generator(Memory_pool *allocator, Type_spec_table *type_table, Operator_table *operator_table, Native_operations_table *native_operations_table) {
    Bytecode_generator *result = push_struct(allocator, Bytecode_generator);

    result->allocator = allocator;
    result->type_table = type_table;
    result->operator_table = operator_table;
    result->native_operations_table = native_operations_table;

    Memory_pool *offsets_allocator = push_struct(allocator, Memory_pool);
    result->tag_offset_table = new_tag_offset_hash_table(offsets_allocator);
    result->function_offset_table = new_function_offset_hash_table(offsets_allocator);

    result->current_register = R1;
    // result->bytecode_size = BYTECODE_FIRST_SIZE;
    // result->bytecode = push_array(result->allocator, Bytecode_instruction, result->bytecode_size);
    Memory_pool *string_pool_allocator = push_struct(allocator, Memory_pool);
    result->string_pool = new_regrowable_linear_buffer<u8>(string_pool_allocator);
    result->bytecode = new_regrowable_linear_buffer<Bytecode_instruction>(allocator);

    return result;
}

internal u32 new_register(Bytecode_generator *generator) {
    u32 result = generator->current_register++;
    return result;
}

// internal Bytecode_instruction *next(Bytecode_generator *generator) {
//     Bytecode_instruction *result = 0;

//     if (generator->bytecode_head >= generator->bytecode_size) {
//         u64 new_size = generator->bytecode_size * 2;
//         debug("resizing bytecode buffer")
//         debug(new_size)
//         Bytecode_instruction *new_bytecode = push_array_copy(generator->allocator, Bytecode_instruction, new_size, generator->bytecode);

//         generator->bytecode_size = new_size;
//         generator->bytecode = new_bytecode;
//     }

//     result = generator->bytecode + generator->bytecode_head++;

//     return result;
// }

enum BYTECODE_RESULT_TYPE {
    EB_NULL,

    E_LITERAL,
    E_REGISTER,
    E_MEMORY,

    EB_COUNT,
};

struct Bytecode_result {
    BYTECODE_RESULT_TYPE type;
    bool comparison_needed;
    u32 size;

    union {
        u64 _u64;
        u32 r;
        Address _address;
    };
};

internal Bytecode_result new_expression_bytecode_result(u64 _u64) {
    Bytecode_result result;
    result.type = E_LITERAL;
    result.comparison_needed = true;
    result._u64 = _u64;
    return result;
}

internal Bytecode_result new_expression_bytecode_literal(u64 _u64) {
    Bytecode_result result;
    result.type = E_LITERAL;
    result.comparison_needed = true;
    result._u64 = _u64;
    return result;
}

internal Bytecode_result new_expression_bytecode_result(u32 r) {
    Bytecode_result result;
    result.type = E_REGISTER;
    result.comparison_needed = true;
    result.r = r;
    return result;
}

internal Bytecode_result new_expression_bytecode_result_register(u32 r) {
    Bytecode_result result;
    result.type = E_REGISTER;
    result.comparison_needed = true;
    result.r = r;
    return result;
}

internal Bytecode_result new_expression_bytecode_result(Address _address) {
    Bytecode_result result;
    result.type = E_MEMORY;
    result.comparison_needed = true;
    result._address = _address;
    return result;
}

internal void print_string_pool(Bytecode_generator *generator) {
    printf(".STRING_POOL\n");

    Regrowable_linear_buffer_iterator<u8> it = iterate(generator->string_pool);

    while (valid(it)) {
        u32 *size = (u32 *)next(&it, 4);
        char *s = (char *)next(&it, *size);
        str _s = {*size, s};
        printf(_s, true);
        putchar('\n');
    }
}

internal void print_bytecode(Bytecode_generator *generator) {
    printf(".CODE\n");

    u32 max_length = 0;
    u32 head = generator->bytecode->head;

    while (head) {
        max_length++;
        head /= 10;
    }

    sfor_count(generator->bytecode->buffer, generator->bytecode->head) {
        u32 line_length = 0;
        u32 line = i;

        while (line) {
            line_length++;
            line /= 10;
        }

        line_length = al_max(line_length, 1);
        print_indent(max_length - line_length);

        if (it->instruction == NOP) {
            printf("%d:  NOP", i);
        } else if (it->instruction == RET) {
            printf("%d:  RET", i);
        } else if (it->instruction == LEAVE) {
            printf("%d:  LEAVE", i);
        } else if (it->instruction == FTAG) {
            printf("%d:", i);
            print(it->destination);
            putchar(':');
        } else if (it->instruction == TAG) {
            printf("%d:", i);
            print(it->destination);
            putchar(':');
        } else {
            printf("%d:  ", i);
            print(it->instruction);
            putchar(' ');
            print(it->destination);

            bool is_a_jump = (
                   it->instruction == CALL
                || it->instruction == JUMP
                || it->instruction == JEQ
                || it->instruction == JNEQ
            );
            if (!is_a_jump) {
                putchar(',');
                putchar(' ');
                print(it->source);
            }
        }


        putchar('\n');
    }
}


