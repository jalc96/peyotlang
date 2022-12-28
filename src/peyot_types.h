enum TYPE_SPEC_TYPE {
    TYPE_SPEC_NONE,

    TYPE_SPEC_NAME,
    TYPE_SPEC_FUNCTION,
    TYPE_SPEC_COMPOUND,

    TYPE_SPEC_COUNT,
};

internal char *to_string(TYPE_SPEC_TYPE type) {
    switch (type) {
        case TYPE_SPEC_NAME: {return "NAME";}
        case TYPE_SPEC_FUNCTION: {return "FUNCTION";}
        default: return "ERROR";
    }
}

struct Member;

#if DEVELOPMENT
#define MEMBER_TABLE_COUNT 2
#else
#define MEMBER_TABLE_COUNT 16
#endif

struct Member_info {
    str member_name;
    str type_name;
    Member_info *next;
};

internal u32 get_member_index(str member_name) {
    u32 result = hash(member_name);
    result = result & (MEMBER_TABLE_COUNT - 1);
    return result;
}

internal void put(Member_info **table, str member_name, str type_name, Memory_pool *allocator) {
    u32 index = get_member_index(member_name);

    Member_info *new_info = push_struct(allocator, Member_info);
    new_info->member_name = member_name;
    new_info->type_name = type_name;

    new_info->next = table[index];
    table[index] = new_info;
}

internal Member_info *get(Member_info **table, str member_name) {
    Member_info *result = 0;

    u32 index = get_member_index(member_name);

    lfor (table[index]) {
        if (equals(member_name, it->member_name)) {
            result = it;
            break;
        }
    }

    return result;
}

struct Type_spec {
    u32 id;
    TYPE_SPEC_TYPE type;
    Src_position src_p;
    str name;
    u32 size;
    Member_info *member_info_table[MEMBER_TABLE_COUNT];
    Type_spec *base;

    union {
        struct {
            u32 member_count;
            Member *members;
        } compound;
    };

    Type_spec *next;
};

internal Type_spec *new_type_spec(u32 id, TYPE_SPEC_TYPE type, str name, Src_position src_p, Memory_pool *allocator) {
    Type_spec *result = push_struct(allocator, Type_spec);

    result->id = id;
    result->type = type;
    result->name = name;
    result->src_p = src_p;
    result->next = 0;
    result->base = 0;

    return result;
}

internal bool equals(Type_spec *a, Type_spec *b) {
    return a->id == b->id;
}

internal bool any_equals(Type_spec *a, Type_spec *b) {
    Type_spec *ai = a;

    while (ai) {
        Type_spec *bi = b;

        while (bi) {
            if (ai->id == bi->id) {
                return true;
            }

            bi = bi->base;
        }

        ai = ai->base;
    }

    return false;
}

internal Type_spec *get_base(Type_spec *type) {
    Type_spec *result = type;

    while (type) {
        result = type;
        type = type->base;
    }

    return result;
}

internal void print(Type_spec *type) {
    printf("%.*s<id: %d>[size: %d]\n", STR_PRINT(type->name), type->id, type->size);
}

internal void print_entire_list(Type_spec *type, u32 indent=0) {
    lfor(type) {
        print_indent(indent);
        print(it);
        indent += 2;
    }
}

#if DEVELOPMENT
    // This is to have a smaller table to check
    #define TYPE_SPEC_HASH_TABLE_SIZE (1 << 4)
#else
    #define TYPE_SPEC_HASH_TABLE_SIZE (1 << 10)
#endif

struct Type_spec_table {
    u32 current_id;
    Type_spec *table[TYPE_SPEC_HASH_TABLE_SIZE];
    Memory_pool *allocator;
};

internal Type_spec_table *new_type_spec_table(Memory_pool *allocator) {
    Type_spec_table *result = push_struct(allocator, Type_spec_table);
    
    result->allocator = allocator;

    return result;
}

internal void print(Type_spec_table *table) {
    printf("---TYPE TABLE---\n");

    sfor(table->table) {
        print_entire_list(*it, 4);
    }
}

internal u32 get_type_spec_index(str name) {
    u32 h = hash(name);
    u32 result = h & (TYPE_SPEC_HASH_TABLE_SIZE - 1);
    return result;
}

internal Type_spec *put(Type_spec_table *table, str name, TYPE_SPEC_TYPE type, Src_position src_p, Type_spec *base=0, u32 size=0) {
    u32 index = get_type_spec_index(name);
    // When creating types in several threads maybe assign id ranges to each thread so they dont stall in the same lock all the time
    Type_spec *result = new_type_spec(table->current_id++, type, name, src_p, table->allocator);
    result->base = base;
    result->next = table->table[index];
    result->size = size;
    table->table[index] = result;

    return result;
}

internal Type_spec *put(Type_spec_table *table, str name, TYPE_SPEC_TYPE type, Src_position src_p, u32 member_count, Member *members) {
    u32 index = get_type_spec_index(name);
    // When creating types in several threads maybe assign id ranges to each thread so they dont stall in the same lock all the time
    Type_spec *result = new_type_spec(table->current_id++, type, name, src_p, table->allocator);
    result->compound.member_count = member_count;
    result->compound.members = members;
    result->next = table->table[index];
    table->table[index] = result;

    return result;
}

internal Type_spec *get(Type_spec_table *table, str name) {
    // TODO: check everywhere this is called to add to the out of order list
    u32 index = get_type_spec_index(name);
    Type_spec *result = table->table[index];

    while (result) {
        if (equals(result->name, name)) {
            break;
        }

        result = result->next;
    }

    return result;
}

internal bool is_native_type(Token token) {
    switch (token.type) {
        case TOKEN_CHAR:
        case TOKEN_STR:
        case TOKEN_U8:
        case TOKEN_U16:
        case TOKEN_U32:
        case TOKEN_U64:
        case TOKEN_S8:
        case TOKEN_S16:
        case TOKEN_S32:
        case TOKEN_S64:
        case TOKEN_F32:
        case TOKEN_F64:
        case TOKEN_BOOL: {
            return true;
        }

        default: {
            return false;
        }
    }
}

internal bool is_type(Type_spec_table *table, Token token) {
    switch (token.type) {
        case TOKEN_CHAR:
        case TOKEN_STR:
        case TOKEN_U8:
        case TOKEN_U16:
        case TOKEN_U32:
        case TOKEN_U64:
        case TOKEN_S8:
        case TOKEN_S16:
        case TOKEN_S32:
        case TOKEN_S64:
        case TOKEN_F32:
        case TOKEN_F64:
        case TOKEN_BOOL: {
            return true;
        }

        case TOKEN_NAME: {
            // NOTE(Juan Antonio) 2022-11-05: i comment this because this check will be made in the type checker where the "undefined type" error will be shown 
            // Type_spec *type = get_type(table, token.name);
            // return (bool)type;
            return true;
        }

        default: {
            return false;
        }
    }
}

struct Native_types_init {
    str name;
    u32 size;
};

internal void initialize_native_types(Type_spec_table *type_table, Memory_pool *allocator) {
    Native_types_init native_types[] = {
        {STATIC_STR("none"), 0},

        {STATIC_STR("void"), 0},

        {STATIC_STR("char"), 1},

        {STATIC_STR("u8"), 1},
        {STATIC_STR("u16"), 2},
        {STATIC_STR("u32"), 4},
        {STATIC_STR("u64"), 8},

        {STATIC_STR("s8"), 1},
        {STATIC_STR("s16"), 2},
        {STATIC_STR("s32"), 4},
        {STATIC_STR("s64"), 8},

        {STATIC_STR("f32"), 4},
        {STATIC_STR("f64"), 8},

        {STATIC_STR("bool"), 1},
    };

    sfor (native_types) {
        put(type_table, it->name, TYPE_SPEC_NAME, {}, 0, it->size);
    }

    Type_spec *added = put(type_table, STATIC_STR("str"), TYPE_SPEC_NAME, {});
    added->size = 12;
    put(added->member_info_table, STATIC_STR("count"), STATIC_STR("u32"), allocator);
    put(added->member_info_table, STATIC_STR("buffer"), STATIC_STR("char"), allocator);
}

struct Native_types_op {
    PEYOT_TOKEN_TYPE op_type;
    str op1;
    str op2;
    str return_type;
    Native_types_op *next;
};

internal void print(Native_types_op *op) {
    printf("%.*s: %.*s, %.*s\n", STR_PRINT(to_symbol(op->op_type)), STR_PRINT(op->op1), STR_PRINT(op->op2));
}

internal void print_entire_list(Native_types_op *op, u32 indent=0) {
    lfor(op) {
        print_indent(indent);
        print(it);
        indent += 2;
    }
}

#if DEVELOPMENT
#define NATIVE_TYPE_HASH_TABLE_SIZE 32
#else
#define NATIVE_TYPE_HASH_TABLE_SIZE 1024
#endif

struct Native_operations_table {
    Native_types_op *table[NATIVE_TYPE_HASH_TABLE_SIZE];
    Memory_pool *allocator;
};

internal Native_operations_table *new_native_types_table(Memory_pool *allocator) {
    Native_operations_table *result = push_struct(allocator, Native_operations_table);

    result->allocator = allocator;

    return result;
}

internal void print(Native_operations_table *table) {
    printf("---TYPE TABLE---\n");

    sfor(table->table) {
        print_entire_list(*it, 4);
    }
}

internal u32 get_native_type_index(PEYOT_TOKEN_TYPE op_type, str op1, str op2) {
    u32 h = ((u32)op_type) *(hash(op1)>>2 + hash(op2)<<5);
    u32 result = h & (NATIVE_TYPE_HASH_TABLE_SIZE - 1);
    return result;
}

internal Native_types_op *put(Native_operations_table *table, PEYOT_TOKEN_TYPE op_type, str op1, str op2, str return_type) {
    u32 index = get_native_type_index(op_type, op1, op2);
    Native_types_op *result = push_struct(table->allocator, Native_types_op);

    result->op_type = op_type;
    result->op1 = op1;
    result->op2 = op2;
    result->return_type = return_type;
    result->next = table->table[index];
    table->table[index] = result;

    return result;
}

internal Native_types_op *get(Native_operations_table *table, PEYOT_TOKEN_TYPE op_type, str op1, str op2) {
    u32 index = get_native_type_index(op_type, op1, op2);
    Native_types_op *result = table->table[index];

    while (result) {
        bool are_equals = (
               result->op_type == op_type
            && equals(result->op1, op1)
            && equals(result->op2, op2)
        );

        if (are_equals) {
            break;
        }

        result = result->next;
    }

    return result;
}

internal bool is_native_expression(Native_operations_table *native_operations_table, PEYOT_TOKEN_TYPE op_type, str op1, str op2) {
    bool result = (bool)get(native_operations_table, op_type, op1, op2);
    return result;
}

struct Native_op_init {
    PEYOT_TOKEN_TYPE op_type;
    str op1;
    str op2;
    str return_type;
};

internal void initialize_native_operators(Native_operations_table *table) {
    Native_op_init native_types_op[] = {
        {TOKEN_PLUS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_MINUS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_STAR, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_SLASH, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_PERCENT, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_NOT_EQUALS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_GREATER_THAN, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_LESS_THAN, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_BITWISE_AND, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_BITWISE_OR, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_LOGICAL_AND, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_LOGICAL_OR, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},

        {TOKEN_PLUS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_MINUS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_STAR, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_SLASH, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_PERCENT, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_NOT_EQUALS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_GREATER_THAN, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_LESS_THAN, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_BITWISE_AND, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_BITWISE_OR, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_LOGICAL_AND, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_LOGICAL_OR, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},

        {TOKEN_PLUS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_MINUS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_STAR, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_SLASH, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_PERCENT, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_NOT_EQUALS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_GREATER_THAN, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_LESS_THAN, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_BITWISE_AND, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_BITWISE_OR, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_LOGICAL_AND, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_LOGICAL_OR, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},

        {TOKEN_PLUS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_MINUS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_STAR, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_SLASH, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_PERCENT, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_NOT_EQUALS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_GREATER_THAN, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_LESS_THAN, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_BITWISE_AND, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_BITWISE_OR, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_LOGICAL_AND, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_LOGICAL_OR, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},




        {TOKEN_PLUS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_MINUS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_STAR, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_SLASH, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_PERCENT, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_NOT_EQUALS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_GREATER_THAN, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_LESS_THAN, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_BITWISE_AND, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_BITWISE_OR, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_LOGICAL_AND, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_LOGICAL_OR, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},

        {TOKEN_PLUS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_MINUS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_STAR, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_SLASH, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_PERCENT, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_NOT_EQUALS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_GREATER_THAN, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_LESS_THAN, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_BITWISE_AND, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_BITWISE_OR, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_LOGICAL_AND, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_LOGICAL_OR, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},

        {TOKEN_PLUS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_MINUS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_STAR, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_SLASH, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_PERCENT, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_NOT_EQUALS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_GREATER_THAN, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_LESS_THAN, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_BITWISE_AND, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_BITWISE_OR, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_LOGICAL_AND, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_LOGICAL_OR, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},

        {TOKEN_PLUS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_MINUS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_STAR, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_SLASH, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_PERCENT, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_NOT_EQUALS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_GREATER_THAN, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_LESS_THAN, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_BITWISE_AND, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_BITWISE_OR, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_LOGICAL_AND, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_LOGICAL_OR, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},




        {TOKEN_PLUS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_MINUS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_STAR, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_SLASH, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_PERCENT, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_NOT_EQUALS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_GREATER_THAN, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_LESS_THAN, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_BITWISE_AND, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_BITWISE_OR, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_LOGICAL_AND, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_LOGICAL_OR, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},

        {TOKEN_PLUS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_MINUS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_STAR, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_SLASH, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_PERCENT, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_NOT_EQUALS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_GREATER_THAN, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_LESS_THAN, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_BITWISE_AND, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_BITWISE_OR, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_LOGICAL_AND, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_LOGICAL_OR, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
    };

    sfor (native_types_op) {
        put(table, it->op_type, it->op1, it->op2, it->return_type);
    }
}

#if 0
internal void initialize_operators(Type_spec_table *type_table, Operator_table *operator_table, Memory_pool *allocator) {
    Native_types_op native_types_op[] = {
        {TOKEN_PLUS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_MINUS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_STAR, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_SLASH, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_PERCENT, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_NOT_EQUALS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_GREATER_THAN, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_LESS_THAN, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_BITWISE_AND, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_BITWISE_OR, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_LOGICAL_AND, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_LOGICAL_OR, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},

        {TOKEN_PLUS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_MINUS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_STAR, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_SLASH, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_PERCENT, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_NOT_EQUALS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_GREATER_THAN, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_LESS_THAN, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_BITWISE_AND, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_BITWISE_OR, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_LOGICAL_AND, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_LOGICAL_OR, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},

        {TOKEN_PLUS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_MINUS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_STAR, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_SLASH, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_PERCENT, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_NOT_EQUALS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_GREATER_THAN, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_LESS_THAN, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_BITWISE_AND, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_BITWISE_OR, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_LOGICAL_AND, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_LOGICAL_OR, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},

        {TOKEN_PLUS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_MINUS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_STAR, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_SLASH, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_PERCENT, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_NOT_EQUALS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_GREATER_THAN, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_LESS_THAN, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_BITWISE_AND, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_BITWISE_OR, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_LOGICAL_AND, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_LOGICAL_OR, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},




        {TOKEN_PLUS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_MINUS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_STAR, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_SLASH, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_PERCENT, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_NOT_EQUALS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_GREATER_THAN, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_LESS_THAN, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_BITWISE_AND, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_BITWISE_OR, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_LOGICAL_AND, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_LOGICAL_OR, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},

        {TOKEN_PLUS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_MINUS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_STAR, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_SLASH, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_PERCENT, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_NOT_EQUALS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_GREATER_THAN, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_LESS_THAN, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_BITWISE_AND, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_BITWISE_OR, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_LOGICAL_AND, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_LOGICAL_OR, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},

        {TOKEN_PLUS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_MINUS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_STAR, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_SLASH, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_PERCENT, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_NOT_EQUALS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_GREATER_THAN, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_LESS_THAN, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_BITWISE_AND, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_BITWISE_OR, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_LOGICAL_AND, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_LOGICAL_OR, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},

        {TOKEN_PLUS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_MINUS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_STAR, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_SLASH, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_PERCENT, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_NOT_EQUALS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_GREATER_THAN, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_LESS_THAN, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_BITWISE_AND, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_BITWISE_OR, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_LOGICAL_AND, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_LOGICAL_OR, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},




        {TOKEN_PLUS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_MINUS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_STAR, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_SLASH, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_PERCENT, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_NOT_EQUALS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_GREATER_THAN, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_LESS_THAN, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_BITWISE_AND, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_BITWISE_OR, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_LOGICAL_AND, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_LOGICAL_OR, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},

        {TOKEN_PLUS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_MINUS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_STAR, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_SLASH, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_PERCENT, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_BINARY_EQUALS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_NOT_EQUALS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_GREATER_THAN, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_GREATER_THAN_OR_EQUALS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_LESS_THAN, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_LESS_THAN_OR_EQUALS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_BITWISE_AND, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_BITWISE_OR, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_LOGICAL_AND, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_LOGICAL_OR, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},

    };


    sfor (native_types_op) {
        Ast_expression *a = new_ast_expression(allocator);
        a->type = AST_EXPRESSION_NAME;
        a->name = STATIC_STR("a");

        Ast_expression *b = new_ast_expression(allocator);
        b->type = AST_EXPRESSION_NAME;
        b->name = STATIC_STR("b");

        Ast_expression *operation = new_ast_expression(allocator);
        operation->type = token_type_to_operation(it->op_type);
        operation->binary.left = a;
        operation->binary.right = b;

        Ast_declaration *c = new_ast_declaration(allocator);
        c->type = AST_DECLARATION_VARIABLE;
        c->name = STATIC_STR("c");
        c->variable.variable_type = get(type_table, it->return_type);
        c->variable.expression = operation;
        c->variable.do_inference = false;


        Ast_block *block = new_ast_block(allocator);
        block->statement_count = 2;

        Ast_statement *sa = block->statements;
        sa->type = AST_STATEMENT_DECLARATION;
        sa->declaration_statement = c;


        Ast_expression *sbc = new_ast_expression(allocator);
        sbc->type = AST_EXPRESSION_NAME;
        sbc->name = STATIC_STR("c");

        Ast_statement *sb = block->statements + 1;
        sb->type = AST_STATEMENT_RETURN;
        sb->return_statement.function;
        sb->return_statement.return_expression = sbc;


        Function *f = push_struct(allocator, Function);
        f->param_count = 2;
        f->params = push_array(allocator, Parameter, 2);
        f->params[0].type = get(type_table, it->op1);
        f->params[0].name = STATIC_STR("a");
        f->params[1].type = get(type_table, it->op2);
        f->params[1].name = STATIC_STR("b");
        f->return_type = get(type_table, it->return_type);
        f->return_type_name = it->return_type;
        f->block = block;

        Ast_declaration *ast = new_ast_declaration(allocator);
        ast->type = AST_DECLARATION_OPERATOR;
        ast->name = to_symbol(it->op_type);
        ast->_operator.operator_token = it->op_type;
        ast->_operator.declaration = f;


        Operator *op = new_operator(allocator, it->op_type, it->op1, it->op2, it->return_type, 0);
        put(operator_table, op);
    }
}
#endif
