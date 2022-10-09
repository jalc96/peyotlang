struct Ast_block;
struct Ast_if;
struct Ast_expression;
struct Ast_statement;

internal void print(Ast_statement *ast, u32 indent=0);

// 
// EXPRESSIONS
// 

enum AST_TYPE {
    AST_NULL,

    AST_LITERAL_U32,
    AST_VARIABLE,
    AST_BLOCK,

    AST_BINARY_ADD,
    AST_BINARY_SUB,
    AST_BINARY_MUL,
    AST_BINARY_DIV,
    AST_BINARY_MOD,
    AST_BINARY_ASSIGNMENT,

    AST_COUNT,
};

internal AST_TYPE token_type_to_operation(TOKEN_TYPE token_type) {
    switch (token_type) {
        case TOKEN_BINARY_ADD: return AST_BINARY_ADD;
        case TOKEN_BINARY_SUB: return AST_BINARY_SUB;
        case TOKEN_BINARY_MUL: return AST_BINARY_MUL;
        case TOKEN_BINARY_DIV: return AST_BINARY_DIV;
        case TOKEN_BINARY_MOD: return AST_BINARY_MOD;
        case TOKEN_ASSIGNMENT: return AST_BINARY_ASSIGNMENT;
        invalid_default_case_msg("impossible to translate TOKEN_TYPE to PEYOT_TYPE");
    }

    return AST_NULL;
}

struct Ast_expression {
    AST_TYPE type;

    union {
        u64 u64_value;
        s64 s64_value;
        f64 f64_value;
        str str_value;
        str variable_name;
    };

    Symbol *symbol;
    Ast_expression *left;
    Ast_expression *right;
};

internal void print(Ast_expression *ast, u32 indent=0, bool is_declaration=false) {
    printf("%*s", indent, "");
    // printf("<%d>", indent);

    switch (ast->type) {
        case AST_LITERAL_U32: {printf("%lld\n", ast->u64_value);} break;
        case AST_BINARY_ADD:  {printf("+:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_BINARY_SUB:  {printf("-:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_BINARY_MUL:  {printf("*:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_BINARY_DIV:  {printf("/:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_BINARY_MOD:  {printf("%:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_VARIABLE:    {printf("%.*s\n", ast->variable_name.count, ast->variable_name.buffer);} break;
        case AST_BINARY_ASSIGNMENT:  {printf("=:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
    }
}

// 
// DECLARATIONS
// 

enum AST_DECLARATION_TYPE {
    AST_DECLARATION_NONE,

    AST_DECLARATION_VARIABLE,

    AST_DECLARATION_COUNT,
};

internal char *to_string(AST_DECLARATION_TYPE type) {
    switch (type) {
        case AST_DECLARATION_VARIABLE: {return "VARIABLE";}
        default: return "ERROR";
    }
}

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
    TYPE_SPEC_TYPE type;
    u32 id;
};

#define get_type_name(...) STATIC_STR("u32 prueba")

internal void print(Type_spec *type) {
    str type_name = get_type_name(type_name_table, type->id);
    printf("<%.*s>", type_name.count, type_name.buffer);
}

struct Ast_declaration {
    AST_DECLARATION_TYPE type;
    Type_spec *variable_type;
    Ast_expression *variable;
};

internal void print(Ast_declaration *ast, u32 indent=0) {
    print(ast->variable, indent, true);
}

// 
// STATEMENTS
// 

enum AST_STATEMENT_TYPE {
    AST_STATEMENT_NONE,

    AST_STATEMENT_BLOCK,
    AST_STATEMENT_IF,
    AST_STATEMENT_EXPRESSION,
    AST_STATEMENT_DECLARATION,

    AST_STATEMENT_COUNT,
};

struct Ast_statement {
    AST_STATEMENT_TYPE type;

    union {
        Ast_block *block_statement;
        Ast_if *if_statement;
        Ast_expression *expression_statement;
        Ast_declaration *declaration_statement;
    };
};

internal Ast_statement *new_ast_statement(void *allocator) {
    Ast_statement *result = (Ast_statement *)malloc(sizeof(Ast_statement));

    *result = {};
    result->type = AST_STATEMENT_NONE;

    return result;
}

// 
// BLOCKS
// 

#define AST_STATEMENTS_PER_BLOCK_LINK 32

struct Ast_block {
    u32 statement_count;
    Ast_statement statements[AST_STATEMENTS_PER_BLOCK_LINK];
    Ast_block *next;
};

internal Ast_block *new_ast_block(void *allocator) {
    Ast_block *result = (Ast_block *)malloc(sizeof(Ast_block));
    result->statement_count = 0;
    result->next = 0;
    return result;
}

struct Ast_block_iterator {
    Ast_block *current;
    u32 i;
};

internal Ast_block_iterator iterate(Ast_block *block) {
    Ast_block_iterator result;

    result.current = block;
    result.i = 0;

    return result;
}

internal bool valid(Ast_block_iterator it) {
    return (it.current && (it.i < it.current->statement_count));
}

internal Ast_statement *advance(Ast_block_iterator *it) {
    if (it->i >= AST_STATEMENTS_PER_BLOCK_LINK) {
        it->i = 0;
        it->current = it->current->next;
    }

    Ast_statement *result = 0;

    if (it->current) {
        result = it->current->statements + it->i++;
    }

    return result;
}

internal void print(Ast_block *block, u32 indent=0) {
    Ast_block_iterator it = iterate(block);

    while (valid(it)) {
        Ast_statement *e = advance(&it);
        print(e, indent + 4);
    }
}

// 
// IF
// 

struct Ast_if {
    Ast_expression condition;
    Ast_block block;
    // TODO: add else, else if
};

internal Ast_if *new_ast_if(void *allocator) {
    Ast_if *result = (Ast_if *)malloc(sizeof(Ast_if));

    *result = {};

    return result;
}

internal void print(Ast_if *ast, u32 indent=0) {
    printf("%*s", indent, "");
    // printf("<%d>", indent);
    printf("if ");
    print(&ast->condition);
    print(&ast->block, indent);
}

internal void print(Ast_statement *ast, u32 indent) {
    switch (ast->type) {
        case AST_STATEMENT_BLOCK: {
            print(ast->block_statement, indent);
        } break;
        case AST_STATEMENT_IF: {
            print(ast->if_statement, indent);
        } break;
        case AST_STATEMENT_EXPRESSION: {
            print(ast->expression_statement, indent);
        } break;
        case AST_STATEMENT_DECLARATION: {
            print(ast->declaration_statement, indent);
        } break;
    }
}

// UNUSED
// UNUSED
// UNUSED

struct Ast {
    AST_TYPE type;

    union {
        Ast_block block;
    };
};
