#if DEVELOPMENT
#define OPERATOR_TABLE_SIZE 8
#else
#define OPERATOR_TABLE_SIZE 512
#endif

struct Ast_declaration;

struct Operator {
    PEYOT_TOKEN_TYPE type;
    // TODO: probably this should be type_specs??
    str operand_1;
    str operand_2;
    str _return_type;
    Ast_declaration *declaration;
    Operator *next;
};

internal Operator *new_operator(Memory_pool *allocator, PEYOT_TOKEN_TYPE type, str op1, str op2, str return_type, Ast_declaration *declaration) {
    Operator *result = push_struct(allocator, Operator);

    result->type = type;
    result->operand_1 = op1;
    result->operand_2 = op2;
    // result->return_type = return_type;
    result->declaration = declaration;
    result->next = 0;

    return result;
}

internal Operator new_stack_operator(PEYOT_TOKEN_TYPE type, str op1, str op2/*, str return_type*/) {
    Operator result = {};

    result.type = type;
    result.operand_1 = op1;
    result.operand_2 = op2;
    // result.return_type = return_type;

    return result;
}

internal bool equals(Operator *a, Operator *b) {
    // TODO: revise the equality between operators
    bool result = (
           a->type == b->type
        && equals(a->operand_1, b->operand_1)
        && equals(a->operand_2, b->operand_2)
        // && equals(a->return_type, b->return_type)
    );
    return result;
}

internal u32 hash(Operator *op) {
    // TODO: this hash is trash
    u32 result = ((u32)op->type) * hash(op->operand_1) + ((u32)op->type) * hash(op->operand_2);
    return result;
}

internal str *to_call_string(Operator *op) {
    u32 size = (
          length(to_symbol(op->type))
        + length(op->operand_1)
        + length(op->operand_2)
    );
    str *result = new_string(size);

    u32 si = 0;

    str s = to_symbol(op->type);

    for (u32 i = 0; i < length(s); i++) {
        result->buffer[si++] = s.buffer[i];
    }

    s = op->operand_1;

    for (u32 i = 0; i < length(s); i++) {
        result->buffer[si++] = s.buffer[i];
    }

    s = op->operand_2;

    for (u32 i = 0; i < length(s); i++) {
        result->buffer[si++] = s.buffer[i];
    }

    return result;
}

internal void print(Operator *op) {
    str ops = to_symbol(op->type);
    str op1 = op->operand_1;
    str op2 = op->operand_2;
    str return_type = {};//op->return_type;
    printf("%.*s: %.*s, %.*s -> %.*s\n", STR_PRINT(ops), STR_PRINT(op1), STR_PRINT(op2), STR_PRINT(return_type));
}

internal void print_entire_list(Operator *op, u32 indent=0) {
    lfor(op) {
        print_indent(indent);
        print(it);
        indent += 2;
    }
}

struct Operator_table {
    Operator *operators[OPERATOR_TABLE_SIZE];
    Memory_pool *allocator;
};

internal Operator_table *new_operator_table(Memory_pool *allocator) {
    Operator_table *result = push_struct(allocator, Operator_table);

    result->allocator = allocator;

    return result;
}

internal void print(Operator_table *table) {
    printf("---OPERATOR TABLE---\n");

    sfor(table->operators) {
        print_entire_list(*it, 4);
    }
}

internal u32 get_operator_index(Operator *op, u32 size) {
    u32 h = hash(op);
    u32 result = h & (size);
    return result;
}

internal void put(Operator_table *table, Operator *op) {
    u32 index = get_operator_index(op, OPERATOR_TABLE_SIZE - 1);

    op->next = table->operators[index];
    table->operators[index] = op;
}

internal Operator *get(Operator_table *table, PEYOT_TOKEN_TYPE op, str op1, str op2/*, str return_type*/) {
    Operator t = new_stack_operator(op, op1, op2/*, return_type*/);
    Operator *result = 0;
    u32 index = get_operator_index(&t, OPERATOR_TABLE_SIZE - 1);
    lfor (table->operators[index]) {
        if (equals(it, &t)) {
            // TODO: revise the equality between operators
            result = it;
            break;
        }
    }

    return result;
}
