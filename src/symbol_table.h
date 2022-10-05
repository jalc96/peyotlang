struct Symbol {
    str name;
    PEYOT_TYPE type;
    str custom_type;
    // str type; when structs appear this will be needed?? 2022-10-05: NO, just use an incremental number to determine the type, and have a parallel array to store the string name of the type for debug and error reporting
    Symbol *next;
};

#define SYMBOL_TABLE_SIZE (1 << 10)

struct Symbol_table {
    Symbol *symbols[SYMBOL_TABLE_SIZE];
    void *allocator;
    Symbol *first_free;
};

internal Symbol_table *create_symbol_table(void *allocator) {
    Symbol_table *result = (Symbol_table *)malloc(sizeof(Symbol_table));
    result->allocator = allocator;
    result->first_free = 0;

    sfor (result->symbols) {
        *it = 0;
    }

    return result;
}

internal Symbol *create_symbol(Symbol_table *table, str name, PEYOT_TYPE type, str custom_type) {
    // TODO: do the check first with the free list and create an allocator and use it
    Symbol *result = (Symbol *)malloc(sizeof(Symbol));
    result->name = name;
    result->type = type;
    result->custom_type = custom_type;
    return result;
}

internal Symbol *create_symbol(Symbol_table *table, str name, PEYOT_TYPE type) {
    str empty = {};
    return create_symbol(table, name, type, empty);
}

internal void put(Symbol_table *table, Symbol *symbol) {
    u32 h = hash(symbol->name);
    u32 i = h & (SYMBOL_TABLE_SIZE - 1);

    symbol->next = table->symbols[i];
    table->symbols[i] = symbol;
}
