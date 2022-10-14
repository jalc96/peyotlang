struct Ast_block;
struct Ast_if;
struct Ast_expression;
struct Ast_statement;

internal void print(Ast_statement *ast, u32 indent=0);
internal void print(Ast_block *block, u32 indent=0);

//
// EXPRESSIONS
//

enum AST_EXPRESSION_TYPE {
    AST_NULL,

    AST_EXPRESSION_LITERAL_U32,
    AST_EXPRESSION_NAME,

    AST_EXPRESSION_UNARY_SUB,
    AST_EXPRESSION_BINARY_ADD,
    AST_EXPRESSION_BINARY_SUB,
    AST_EXPRESSION_BINARY_MUL,
    AST_EXPRESSION_BINARY_DIV,
    AST_EXPRESSION_BINARY_MOD,

    AST_EXPRESSION_BINARY_EQUALS,
    AST_EXPRESSION_BINARY_NOT_EQUALS,

    AST_EXPRESSION_BINARY_GREATER_THAN,
    AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS,
    AST_EXPRESSION_BINARY_LESS_THAN,
    AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS,

    AST_EXPRESSION_UNARY_LOGICAL_NOT,
    AST_EXPRESSION_BINARY_LOGICAL_OR,
    AST_EXPRESSION_BINARY_LOGICAL_AND,

    AST_EXPRESSION_UNARY_BITWISE_NOT,
    AST_EXPRESSION_BINARY_BITWISE_OR,
    AST_EXPRESSION_BINARY_BITWISE_AND,

    AST_EXPRESSION_BINARY_ASSIGNMENT,

    AST_COUNT,
};

internal AST_EXPRESSION_TYPE token_type_to_operation(PEYOT_TOKEN_TYPE token_type) {
    switch (token_type) {
        case TOKEN_BINARY_ADD: return AST_EXPRESSION_BINARY_ADD;
        case TOKEN_SUB: return AST_EXPRESSION_BINARY_SUB;
        case TOKEN_BINARY_MUL: return AST_EXPRESSION_BINARY_MUL;
        case TOKEN_BINARY_DIV: return AST_EXPRESSION_BINARY_DIV;
        case TOKEN_BINARY_MOD: return AST_EXPRESSION_BINARY_MOD;

        case TOKEN_BINARY_EQUALS: return AST_EXPRESSION_BINARY_EQUALS;
        case TOKEN_BINARY_NOT_EQUALS: return AST_EXPRESSION_BINARY_NOT_EQUALS;

        case TOKEN_BINARY_GREATER_THAN: return AST_EXPRESSION_BINARY_GREATER_THAN;
        case TOKEN_BINARY_GREATER_THAN_OR_EQUALS: return AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS;
        case TOKEN_BINARY_LESS_THAN: return AST_EXPRESSION_BINARY_LESS_THAN;
        case TOKEN_BINARY_LESS_THAN_OR_EQUALS: return AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS;

        case TOKEN_UNARY_LOGICAL_NOT: return AST_EXPRESSION_UNARY_LOGICAL_NOT;
        case TOKEN_BINARY_LOGICAL_AND: return AST_EXPRESSION_BINARY_LOGICAL_AND;
        case TOKEN_BINARY_LOGICAL_OR: return AST_EXPRESSION_BINARY_LOGICAL_OR;

        case TOKEN_UNARY_BITWISE_NOT: return AST_EXPRESSION_UNARY_BITWISE_NOT;
        case TOKEN_BINARY_BITWISE_AND: return AST_EXPRESSION_BINARY_BITWISE_AND;
        case TOKEN_BINARY_BITWISE_OR: return AST_EXPRESSION_BINARY_BITWISE_OR;

        case TOKEN_ASSIGNMENT: return AST_EXPRESSION_BINARY_ASSIGNMENT;
        invalid_default_case_msg("impossible to translate PEYOT_TOKEN_TYPE to PEYOT_TYPE");
    }

    return AST_NULL;
}

struct Ast_expression {
    AST_EXPRESSION_TYPE type;

    union {
        u64 u64_value;
        s64 s64_value;
        f64 f64_value;
        str str_value;
        str name;
    };

    Symbol *symbol;
    Ast_expression *left;
    Ast_expression *right;
};

internal Ast_expression *new_ast_expression(Memory_pool *allocator) {
    Ast_expression *result = push_struct(allocator, Ast_expression);
    return result;
}

internal void print(Ast_expression *ast, u32 indent=0, bool is_declaration=false) {
    printf("%*s", indent, "");
    // printf("<%d>", indent);

    switch (ast->type) {
        case AST_EXPRESSION_NAME:    {printf("%.*s\n", ast->name.count, ast->name.buffer);} break;
        case AST_EXPRESSION_LITERAL_U32: {printf("%lld\n", ast->u64_value);} break;

        case AST_EXPRESSION_UNARY_SUB:  {printf("-:\n"); print(ast->right, indent+4); } break;
        case AST_EXPRESSION_BINARY_ADD:  {printf("+:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_EXPRESSION_BINARY_SUB:  {printf("-:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_EXPRESSION_BINARY_MUL:  {printf("*:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_EXPRESSION_BINARY_DIV:  {printf("/:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_EXPRESSION_BINARY_MOD:  {printf("%:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;

        case AST_EXPRESSION_BINARY_EQUALS: {printf("==:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;
        case AST_EXPRESSION_BINARY_NOT_EQUALS: {printf("!=:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;

        case AST_EXPRESSION_BINARY_GREATER_THAN: {printf(">:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;
        case AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS: {printf(">=:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;
        case AST_EXPRESSION_BINARY_LESS_THAN: {printf("<:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;
        case AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS: {printf("<=:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;

        case AST_EXPRESSION_UNARY_LOGICAL_NOT: {printf("!:\n"); print(ast->right, indent +4);} break;
        case AST_EXPRESSION_BINARY_LOGICAL_OR: {printf("||:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;
        case AST_EXPRESSION_BINARY_LOGICAL_AND: {printf("&&:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;

        case AST_EXPRESSION_UNARY_BITWISE_NOT: {printf("~:\n"); print(ast->right, indent +4);} break;
        case AST_EXPRESSION_BINARY_BITWISE_OR: {printf("|:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;
        case AST_EXPRESSION_BINARY_BITWISE_AND: {printf("&:\n"); print(ast->left, indent+4); print(ast->right, indent +4);} break;

        case AST_EXPRESSION_BINARY_ASSIGNMENT:  {printf("=:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
    }
}

//
// DECLARATIONS
//

enum AST_DECLARATION_TYPE {
    AST_DECLARATION_NONE,

    AST_DECLARATION_VARIABLE,
    AST_DECLARATION_FUNCTION,

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

struct Parameter {
    Type_spec type;
    str name;
};

struct Ast_declaration {
    AST_DECLARATION_TYPE type;
    union {
        struct {
            Type_spec *variable_type;
            Ast_expression *variable;
        }; // VARIABLE
        struct {
            Ast_expression function_name;
            u32 param_count;
            Parameter *params;
            Type_spec return_type;
            Ast_block *block;
        }; // FUNCTION
    };
};

internal void print(Ast_declaration *ast, u32 indent=0) {
    switch (ast->type) {
        case AST_DECLARATION_VARIABLE: {print(ast->variable, indent, true);} break;
        case AST_DECLARATION_FUNCTION: {
            print(&ast->function_name);
            printf("%*s", indent, "");

            sfor_count(ast->params, ast->param_count) {
                // weird bug with the param names, the pointer to the params is changed
                printf("%.*s, ", it->name.count, it->name.buffer);
            }

            print(ast->block);
        } break;
    }
}

//
// LOOPS
//

struct Ast_loop {
    Ast_declaration *pre;
    Ast_expression *condition;
    Ast_expression *post;
    Ast_block *block;
};

internal Ast_loop *new_ast_loop(Memory_pool *allocator) {
    Ast_loop *result = push_struct(allocator, Ast_loop);

    *result = {};

    return result;
}

internal void print(Ast_loop *ast, u32 indent=0) {
    printf("%*sloop:\n", indent, "");
    printf("%*s", indent, "");
    if (ast->pre) print(ast->pre, indent);
    print(ast->condition, indent+4);
    if (ast->post) print(ast->post, indent+4);
    print(ast->block, indent+4);
}

//
// STATEMENTS
//

enum AST_STATEMENT_TYPE {
    AST_STATEMENT_NONE,

    AST_STATEMENT_BLOCK,
    AST_STATEMENT_IF,
    AST_STATEMENT_LOOP,
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
        Ast_loop *loop_statement;
    };
};

internal Ast_statement *new_ast_statement(Memory_pool *allocator) {
    Ast_statement *result = push_struct(allocator, Ast_statement);

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

internal Ast_block *new_ast_block(Memory_pool *allocator) {
    Ast_block *result = push_struct(allocator, Ast_block);
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

internal void print(Ast_block *block, u32 indent) {
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

internal Ast_if *new_ast_if(Memory_pool *allocator) {
    Ast_if *result = push_struct(allocator, Ast_if);

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
        case AST_STATEMENT_LOOP: {
            print(ast->loop_statement, indent);
        } break;
    }
}

// UNUSED
// UNUSED
// UNUSED

struct Ast {
    AST_EXPRESSION_TYPE type;

    union {
        Ast_block block;
    };
};
