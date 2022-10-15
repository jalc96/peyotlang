enum TYPE_SPEC_TYPE {
    TYPE_SPEC_NONE,

    TYPE_SPEC_NAME,
    TYPE_SPEC_FUNCTION,

    TYPE_SPEC_COUNT,
};

internal char *to_string(TYPE_SPEC_TYPE type) {
    switch (type) {
        case TYPE_SPEC_NAME: {return "NAME";}
        case TYPE_SPEC_FUNCTION: {return "FUNCTION";}
        default: return "ERROR";
    }
}

struct Type_spec {
    u32 id;
    TYPE_SPEC_TYPE type;
    str name;
    Type_spec *next;
};

internal Type_spec *new_type_spec(u32 id, TYPE_SPEC_TYPE type, str name, Memory_pool *allocator) {
    Type_spec *result = push_struct(allocator, Type_spec);
    result->id = id;
    result->type = type;
    result->name = name;
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

#define TYPE_SPEC_HASH_TABLE_SIZE 1024

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

internal Type_spec *push_type(Type_spec_table *table, str name, TYPE_SPEC_TYPE type) {
    u32 index = get_type_spec_index(name);
    Type_spec *result = new_type_spec(table->current_id++, type, name, table->allocator);
    result->next = table->table[index];
    table->table[index] = result;

    return result;
}

internal Type_spec *get_type(Type_spec_table *table, str name) {
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
        // TODO: how to handle custom types here??, maybe do a query to the hash table of types??
        case TOKEN_U32: {
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
        push_type(type_table, *it, TYPE_SPEC_NAME);
    }
}
