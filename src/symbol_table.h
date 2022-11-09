struct Symbol {
    str name;
    // NOTE(Juan Antonio) 2022-11-05: maybe use a pointer to the type_spec?? that can make the out of order a bit more annoying to deal with though, storing the name allow to wait until the out of order is finished to check whether the type is declared or not and the type check is already made, so this seems more convenient.
    str type_name;
    Src_position src_p;
    // Scope
    Symbol *next;
};

internal Symbol *new_symbol(Memory_pool *allocator, str name, str type_name, Src_position src_p) {
    Symbol *result = push_struct(allocator, Symbol);

    result->name = name;
    result->type_name = type_name;
    result->src_p = src_p;

    return result;
}

internal void print_entire_list(Symbol *symbol, u32 indent=0) {
    lfor(symbol) {
        print_indent(indent);
        printf("%.*s<%.*s>\n", STR_PRINT(it->name), STR_PRINT(it->type_name));
        indent += 2;
    }
}

#if DEVELOPMENT
    // This is to have a smaller table to check
    #define SYMBOL_TABLE_SIZE (1 << 4)
#else
    #define SYMBOL_TABLE_SIZE (1 << 10)
#endif

struct Symbol_table {
    Symbol *symbols[SYMBOL_TABLE_SIZE];
    Memory_pool *allocator;
    Symbol_table *next;
};

internal Symbol_table *new_symbol_table(Memory_pool *allocator) {
    Symbol_table *result = push_struct(allocator, Symbol_table);

    result->allocator = allocator;

    return result;
}

internal void print(Symbol_table *table) {
    printf("---SYMBOL TABLE---\n");

    sfor(table->symbols) {
        print_entire_list(*it, 4);
    }
}

internal u32 get_symbol_index(str name) {
    u32 h = hash(name);
    u32 result = h & (SYMBOL_TABLE_SIZE - 1);
    return result;
}

internal void put(Symbol_table *table, str name, str type_name, Src_position src_p) {
    Symbol *symbol = new_symbol(table->allocator, name, type_name, src_p);

    u32 i = get_symbol_index(symbol->name);

    symbol->next = table->symbols[i];
    table->symbols[i] = symbol;
}

internal Symbol *get(Symbol_table *table, str name) {
    Symbol *result = 0;
    u32 i = get_symbol_index(name);

    lfor (table) {
        result = it->symbols[i];

        while (result && !equals(result->name, name)) {
            result = result->next;
        }

        if (result) {
            break;
        }
    }

    return result;
}
