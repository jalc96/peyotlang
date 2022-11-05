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

struct Type_spec {
    u32 id;
    TYPE_SPEC_TYPE type;
    Src_position src_p;
    str name;

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
    return result;
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

internal Type_spec *put(Type_spec_table *table, str name, TYPE_SPEC_TYPE type, Src_position src_p) {
    u32 index = get_type_spec_index(name);
    // When creating types in several threads maybe assign id ranges to each thread so they dont stall in the same lock all the time
    Type_spec *result = new_type_spec(table->current_id++, type, name, src_p, table->allocator);
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
        case TOKEN_U32: {
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

internal void initialize_native_types(Type_spec_table *type_table) {
    str native_types[] = {
        STATIC_STR("none"),
        STATIC_STR("u32"),
    };

    sfor (native_types) {
        put(type_table, *it, TYPE_SPEC_NAME, {});
    }
}
