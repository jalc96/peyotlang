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

struct Native_types_op {
    PEYOT_TOKEN_TYPE op_type;
    str op1;
    str op2;
    str return_type;
};

internal void initialize_operators(Type_spec_table *type_table, Operator_table *operator_table, Memory_pool *allocator) {
    Native_types_op native_types_op[] = {
        {TOKEN_PLUS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_MINUS, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_STAR, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_SLASH, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},
        {TOKEN_PERCENT, STATIC_STR("u8"), STATIC_STR("u8"), STATIC_STR("u8")},

        {TOKEN_PLUS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_MINUS, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_STAR, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_SLASH, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},
        {TOKEN_PERCENT, STATIC_STR("u16"), STATIC_STR("u16"), STATIC_STR("u16")},

        {TOKEN_PLUS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_MINUS, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_STAR, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_SLASH, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},
        {TOKEN_PERCENT, STATIC_STR("u32"), STATIC_STR("u32"), STATIC_STR("u32")},

        {TOKEN_PLUS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_MINUS, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_STAR, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_SLASH, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},
        {TOKEN_PERCENT, STATIC_STR("u64"), STATIC_STR("u64"), STATIC_STR("u64")},


        {TOKEN_PLUS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_MINUS, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_STAR, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_SLASH, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},
        {TOKEN_PERCENT, STATIC_STR("s8"), STATIC_STR("s8"), STATIC_STR("s8")},

        {TOKEN_PLUS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_MINUS, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_STAR, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_SLASH, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},
        {TOKEN_PERCENT, STATIC_STR("s16"), STATIC_STR("s16"), STATIC_STR("s16")},

        {TOKEN_PLUS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_MINUS, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_STAR, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_SLASH, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},
        {TOKEN_PERCENT, STATIC_STR("s32"), STATIC_STR("s32"), STATIC_STR("s32")},

        {TOKEN_PLUS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_MINUS, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_STAR, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_SLASH, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},
        {TOKEN_PERCENT, STATIC_STR("s64"), STATIC_STR("s64"), STATIC_STR("s64")},


        {TOKEN_PLUS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_MINUS, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_STAR, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_SLASH, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},
        {TOKEN_PERCENT, STATIC_STR("f32"), STATIC_STR("f32"), STATIC_STR("f32")},

        {TOKEN_PLUS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_MINUS, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_STAR, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_SLASH, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},
        {TOKEN_PERCENT, STATIC_STR("f64"), STATIC_STR("f64"), STATIC_STR("f64")},

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
