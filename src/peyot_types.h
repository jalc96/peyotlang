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

#define MEMBER_TABLE_COUNT 16

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

internal void print(Type_spec *type, u32 indent=0) {
    printf("%.*s<id: %d>\n", type->name.count, type->name.buffer, type->id);
}

internal void print_entire_list(Type_spec *type, u32 indent=0) {
    lfor(type) {
        print_indent(indent);
        printf("%.*s<%d>\n", it->name.count, it->name.buffer, it->id);
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

internal Type_spec *put(Type_spec_table *table, str name, TYPE_SPEC_TYPE type, Src_position src_p, Type_spec *base=0) {
    u32 index = get_type_spec_index(name);
    // When creating types in several threads maybe assign id ranges to each thread so they dont stall in the same lock all the time
    Type_spec *result = new_type_spec(table->current_id++, type, name, src_p, table->allocator);
    result->base = base;
    result->next = table->table[index];
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

internal void initialize_native_types(Type_spec_table *type_table, Memory_pool *allocator) {
    str native_types[] = {
        STATIC_STR("none"),

        STATIC_STR("char"),

        STATIC_STR("u8"),
        STATIC_STR("u16"),
        STATIC_STR("u32"),
        STATIC_STR("u64"),

        STATIC_STR("s8"),
        STATIC_STR("s16"),
        STATIC_STR("s32"),
        STATIC_STR("s64"),

        STATIC_STR("f32"),
        STATIC_STR("f64"),

        STATIC_STR("bool"),
    };

    sfor (native_types) {
        put(type_table, *it, TYPE_SPEC_NAME, {});
    }

    Type_spec *added = put(type_table, STATIC_STR("str"), TYPE_SPEC_NAME, {});
    put(added->member_info_table, STATIC_STR("count"), STATIC_STR("u32"), allocator);
    put(added->member_info_table, STATIC_STR("buffer"), STATIC_STR("char"), allocator);
}
