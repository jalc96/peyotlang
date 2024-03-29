/*
One problem with this type of parsing expressions is that if you only want to parse 3, you have to recurs ALL the way down starting from parse_binary_expression, and that is dog crap slow, one way that came to mind to solve this is to do a pre pass on the expression and select the lowest precedence operator as starting point

have a function template and type

#define EXPRESSION_PARSING_FUNCTION(name) Ast *name(Parser *parser, Ast *parent)
typedef EXPRESSION_PARSING_FUNCTION(Expression_parsing_function)

struct Expression_parsing_shortcut {
    EXPRESSION_TYPE type;
    Expression_parsing_function *parser;
};

then having a table with the expression types and the functions

static Expression_parsing_shortcut expression_parsing_shortcut[] = {
    AST_EXPRESSION_BINARY_ADD, parse_expression,
    AST_EXPRESSION_BINARY_MUL, parse_term,
    ...
};

then choose the lowest precedence as the first function to tall instead of always calling the least precedence one

Lexer_savepoint savepoint = create_savepoint(lexer);

EXPRESSION_TYPE lowest_precedence = HERE THE HIGHEST PRECEDENCE;

while (!end_of_expression) {
    Token t = get_next_token(lexer);
    lowest_precedence = select_lowest_precedence(lowest_precedence, to_ast_type(t));
}

rollback_lexer(savepoint);



expression_parsing_shortcut[lowest_precedence].parser(parser, root);

*/
internal void print(Ast_statement *ast, u32 indent=0, bool print_leaf=false);
internal void print(Ast_block *block, u32 indent=0);

#define log_error(eb, format, ...) eb->head += stbsp_snprintf(get_buffer(eb), eb->size, format, __VA_ARGS__)
#define skip_new_line 1

struct Parser {
// TODO: maybe put the Lexer in here and pass this around instead??
    Type_spec_table *type_table;
    Symbol_table *current_scope;
    Symbol_table *first_free_table;
    Symbol *first_free_symbol;
    Operator_table *operator_table;
    Native_operations_table *native_operations_table;


    bool parsing_errors;
    bool type_errors;
    Str_buffer error_buffer;
    Memory_pool *allocator;

    Pending_type sentinel;
};

internal Parser *new_parser(Memory_pool *allocator, Type_spec_table *type_table, Symbol_table *symbol_table, Operator_table *operator_table, Native_operations_table *native_operations_table) {
    Parser *result = push_struct(allocator, Parser);

    result->type_table = type_table;
    result->current_scope = symbol_table;
    result->operator_table = operator_table;
    result->native_operations_table = native_operations_table;
    result->parsing_errors = false;
    result->error_buffer = new_str_buffer(allocator, 65536);
    result->allocator = allocator;
    result->sentinel.next = &result->sentinel;
    result->sentinel.previous = &result->sentinel;

    return result;
}

internal bool parsing_errors(Parser *parser) {
    return parser->parsing_errors;
}

internal bool out_of_order_declaration_errors(Parser *parser) {
    return parser->sentinel.previous != &parser->sentinel;
}

internal bool type_errors(Parser *parser) {
    return parser->type_errors;
}







//
// EXPRESSIONS
//

enum AST_EXPRESSION_TYPE {
    AST_NULL,


    AST_EXPRESSION_SIZEOF,
    AST_EXPRESSION_OFFSETOF,
    AST_EXPRESSION_TYPEOF,

    AST_EXPRESSION_LITERAL_TYPE,
    AST_EXPRESSION_LITERAL_CHAR,
    AST_EXPRESSION_LITERAL_STR,
    AST_EXPRESSION_LITERAL_INTEGER,
    AST_EXPRESSION_LITERAL_FLOAT,
    AST_EXPRESSION_LITERAL_BOOL,
    AST_EXPRESSION_NAME,
    AST_EXPRESSION_MEMBER,
    AST_EXPRESSION_FUNCTION_CALL,

    AST_EXPRESSION_UNARY_PRE_INCREMENT,
    AST_EXPRESSION_UNARY_POST_INCREMENT,
    AST_EXPRESSION_UNARY_PRE_DECREMENT,
    AST_EXPRESSION_UNARY_POST_DECREMENT,
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
        case TOKEN_PLUS: return AST_EXPRESSION_BINARY_ADD;
        case TOKEN_MINUS: return AST_EXPRESSION_BINARY_SUB;
        case TOKEN_STAR: return AST_EXPRESSION_BINARY_MUL;
        case TOKEN_SLASH: return AST_EXPRESSION_BINARY_DIV;
        case TOKEN_PERCENT: return AST_EXPRESSION_BINARY_MOD;

        case TOKEN_BINARY_EQUALS: return AST_EXPRESSION_BINARY_EQUALS;
        case TOKEN_NOT_EQUALS: return AST_EXPRESSION_BINARY_NOT_EQUALS;

        case TOKEN_GREATER_THAN: return AST_EXPRESSION_BINARY_GREATER_THAN;
        case TOKEN_GREATER_THAN_OR_EQUALS: return AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS;
        case TOKEN_LESS_THAN: return AST_EXPRESSION_BINARY_LESS_THAN;
        case TOKEN_LESS_THAN_OR_EQUALS: return AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS;

        case TOKEN_UNARY_LOGICAL_NOT: return AST_EXPRESSION_UNARY_LOGICAL_NOT;
        case TOKEN_LOGICAL_AND: return AST_EXPRESSION_BINARY_LOGICAL_AND;
        case TOKEN_LOGICAL_OR: return AST_EXPRESSION_BINARY_LOGICAL_OR;

        case TOKEN_UNARY_BITWISE_NOT: return AST_EXPRESSION_UNARY_BITWISE_NOT;
        case TOKEN_BITWISE_AND: return AST_EXPRESSION_BINARY_BITWISE_AND;
        case TOKEN_BITWISE_OR: return AST_EXPRESSION_BINARY_BITWISE_OR;

        case TOKEN_ASSIGNMENT: return AST_EXPRESSION_BINARY_ASSIGNMENT;
        invalid_default_case_msg("impossible to translate PEYOT_TOKEN_TYPE to PEYOT_TYPE");
    }

    return AST_NULL;
}

internal PEYOT_TOKEN_TYPE to_op_token_type(AST_EXPRESSION_TYPE type) {
    switch (type) {
        case AST_EXPRESSION_BINARY_ADD: {
            return TOKEN_PLUS;
        } break;
        case AST_EXPRESSION_BINARY_SUB: {
            return TOKEN_MINUS;
        } break;
        case AST_EXPRESSION_BINARY_MUL: {
            return TOKEN_STAR;
        } break;
        case AST_EXPRESSION_BINARY_DIV: {
            return TOKEN_SLASH;
        } break;
        case AST_EXPRESSION_BINARY_MOD: {
            return TOKEN_PERCENT;
        } break;
        case AST_EXPRESSION_BINARY_EQUALS: {
            return TOKEN_BINARY_EQUALS;
        } break;
        case AST_EXPRESSION_BINARY_NOT_EQUALS: {
            return TOKEN_NOT_EQUALS;
        } break;
        case AST_EXPRESSION_BINARY_GREATER_THAN: {
            return TOKEN_GREATER_THAN;
        } break;
        case AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS: {
            return TOKEN_GREATER_THAN_OR_EQUALS;
        } break;
        case AST_EXPRESSION_BINARY_LESS_THAN: {
            return TOKEN_LESS_THAN;
        } break;
        case AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS: {
            return TOKEN_LESS_THAN_OR_EQUALS;
        } break;
        case AST_EXPRESSION_BINARY_LOGICAL_OR: {
            return TOKEN_BITWISE_AND;
        } break;
        case AST_EXPRESSION_BINARY_LOGICAL_AND: {
            return TOKEN_BITWISE_OR;
        } break;
        case AST_EXPRESSION_BINARY_BITWISE_OR: {
            return TOKEN_LOGICAL_AND;
        } break;
        case AST_EXPRESSION_BINARY_BITWISE_AND: {
            return TOKEN_LOGICAL_OR;
        } break;

        default: {
            assert(false, "check for this in the calling site");
            return TOKEN_NULL;
        } break;
    }
}

struct Call_parameter {
    Ast_expression *parameter;
    Call_parameter *next;
};

struct Ast_statement;

struct Ast_expression {
    AST_EXPRESSION_TYPE type;
    Src_position src_p;
    Type_spec *op_type;

    union {
        char char_value;
        bool bool_value;
        u64 u64_value;
        s64 s64_value;
        f64 f64_value;
        str str_value;
        str name;
    };

    union {
        struct {
            Ast_expression *left;
            Ast_expression *right;
        } binary;
        struct {
            Ast_expression *member_name;
        } member;
        struct {
            u32 parameter_count;
            Call_parameter *parameter;
        } function_call;
        Ast_statement *statement;
    };
};

internal Ast_expression *new_ast_expression(Memory_pool *allocator) {
    Ast_expression *result = push_struct(allocator, Ast_expression);
    return result;
}

internal void print(Ast_expression *ast, u32 indent=0, bool is_declaration=false, bool print_new_line=true) {
    print_indent(indent);
    // printf("<%d>", indent);

    switch (ast->type) {
        case AST_EXPRESSION_NAME:    {printf("%.*s", ast->name.count, ast->name.buffer);} break;
        case AST_EXPRESSION_LITERAL_TYPE: {
            Type_spec *type = get(global_type_table, ast->name);
            printf("%.*s", STR_PRINT(type->name));
        } break;
        case AST_EXPRESSION_SIZEOF: {print(ast->statement, indent, true);} break;
        case AST_EXPRESSION_OFFSETOF: {print(ast->statement, indent, true);} break;
        case AST_EXPRESSION_TYPEOF: {print(ast->statement, indent, true);} break;
        case AST_EXPRESSION_LITERAL_CHAR: {printf("'%c'", ast->char_value);} break;
        case AST_EXPRESSION_LITERAL_STR: {printf("\"%.*s\"", STR_PRINT(ast->str_value));} break;
        case AST_EXPRESSION_LITERAL_INTEGER: {printf("%lld", ast->u64_value);} break;
        case AST_EXPRESSION_LITERAL_FLOAT: {printf("%f", ast->f64_value);} break;
        case AST_EXPRESSION_LITERAL_BOOL: {printf("%s", ast->bool_value ? "true" : "false");} break;
        case AST_EXPRESSION_MEMBER: {
            printf("%.*s", STR_PRINT(ast->name));
            putchar('.');
            printf("%.*s", STR_PRINT(ast->member.member_name->name));
        } break;
        case AST_EXPRESSION_FUNCTION_CALL: {
            printf("%.*s(", STR_PRINT(ast->name));

            lfor (ast->function_call.parameter) {
                print(it->parameter, 0, false, false);
            }

            putchar(')');
        } break;


        case AST_EXPRESSION_UNARY_PRE_INCREMENT: {printf("pre++"); print(ast->binary.right, indent+4);} break;
        case AST_EXPRESSION_UNARY_POST_INCREMENT: {printf("post++"); print(ast->binary.right, indent+4);} break;
        case AST_EXPRESSION_UNARY_PRE_DECREMENT: {printf("pre--"); print(ast->binary.right, indent+4);} break;
        case AST_EXPRESSION_UNARY_POST_DECREMENT: {printf("post--"); print(ast->binary.right, indent+4);} break;
        case AST_EXPRESSION_UNARY_SUB:  {printf("-:"); print(ast->binary.right, indent+4); } break;
        case AST_EXPRESSION_BINARY_ADD:  {printf("+:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent+4); } break;
        case AST_EXPRESSION_BINARY_SUB:  {printf("-:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent+4); } break;
        case AST_EXPRESSION_BINARY_MUL:  {printf("*:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent+4); } break;
        case AST_EXPRESSION_BINARY_DIV:  {printf("/:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent+4); } break;
        case AST_EXPRESSION_BINARY_MOD:  {printf("%:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent+4); } break;

        case AST_EXPRESSION_BINARY_EQUALS: {printf("==:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;
        case AST_EXPRESSION_BINARY_NOT_EQUALS: {printf("!=:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;

        case AST_EXPRESSION_BINARY_GREATER_THAN: {printf(">:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;
        case AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS: {printf(">=:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;
        case AST_EXPRESSION_BINARY_LESS_THAN: {printf("<:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;
        case AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS: {printf("<=:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;

        case AST_EXPRESSION_UNARY_LOGICAL_NOT: {printf("!:"); print(ast->binary.right, indent +4);} break;
        case AST_EXPRESSION_BINARY_LOGICAL_OR: {printf("||:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;
        case AST_EXPRESSION_BINARY_LOGICAL_AND: {printf("&&:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;

        case AST_EXPRESSION_UNARY_BITWISE_NOT: {printf("~:"); print(ast->binary.right, indent +4);} break;
        case AST_EXPRESSION_BINARY_BITWISE_OR: {printf("|:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;
        case AST_EXPRESSION_BINARY_BITWISE_AND: {printf("&:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent +4);} break;

        case AST_EXPRESSION_BINARY_ASSIGNMENT:  {printf("=:"); print(ast->binary.left, indent+4); print(ast->binary.right, indent+4); } break;
    }

    if (print_new_line) {
        putchar('\n');
    }
}

internal bool is_binary(AST_EXPRESSION_TYPE type) {
    switch (type) {
        case AST_EXPRESSION_BINARY_ADD:
        case AST_EXPRESSION_BINARY_SUB:
        case AST_EXPRESSION_BINARY_MUL:
        case AST_EXPRESSION_BINARY_DIV:
        case AST_EXPRESSION_BINARY_MOD:
        case AST_EXPRESSION_BINARY_EQUALS:
        case AST_EXPRESSION_BINARY_NOT_EQUALS:
        case AST_EXPRESSION_BINARY_GREATER_THAN:
        case AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS:
        case AST_EXPRESSION_BINARY_LESS_THAN:
        case AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS:
        case AST_EXPRESSION_BINARY_LOGICAL_OR:
        case AST_EXPRESSION_BINARY_LOGICAL_AND:
        case AST_EXPRESSION_BINARY_BITWISE_OR:
        case AST_EXPRESSION_BINARY_BITWISE_AND:
        case AST_EXPRESSION_BINARY_ASSIGNMENT:  {
            return true;
        }

        default: {return false;}
    }
}

internal bool is_unary(AST_EXPRESSION_TYPE type) {
    switch (type) {
        case AST_EXPRESSION_MEMBER:
        case AST_EXPRESSION_UNARY_PRE_INCREMENT:
        case AST_EXPRESSION_UNARY_POST_INCREMENT:
        case AST_EXPRESSION_UNARY_PRE_DECREMENT:
        case AST_EXPRESSION_UNARY_POST_DECREMENT:
        case AST_EXPRESSION_UNARY_SUB:
        case AST_EXPRESSION_UNARY_LOGICAL_NOT:
        case AST_EXPRESSION_UNARY_BITWISE_NOT: {
            return true;
        }

        default: {return false;}
    }
}

internal bool is_leaf(AST_EXPRESSION_TYPE type) {
    switch (type) {
        case AST_EXPRESSION_LITERAL_TYPE:
        case AST_EXPRESSION_LITERAL_CHAR:
        case AST_EXPRESSION_LITERAL_STR:
        case AST_EXPRESSION_LITERAL_INTEGER:
        case AST_EXPRESSION_LITERAL_FLOAT:
        case AST_EXPRESSION_NAME:
        case AST_EXPRESSION_MEMBER:
        case AST_EXPRESSION_FUNCTION_CALL: {
            return true;
        }

        default: {return false;}
    }
}

internal bool is_relational(AST_EXPRESSION_TYPE type) {
    switch(type) {
        case AST_EXPRESSION_BINARY_EQUALS:
        case AST_EXPRESSION_BINARY_NOT_EQUALS:
        case AST_EXPRESSION_BINARY_GREATER_THAN:
        case AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS:
        case AST_EXPRESSION_BINARY_LESS_THAN:
        case AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS: {
            return true;
        }
        default: {return false;}
    }
}

internal bool is_arithmetic(AST_EXPRESSION_TYPE type) {
    switch(type) {
        case AST_EXPRESSION_UNARY_PRE_INCREMENT:
        case AST_EXPRESSION_UNARY_POST_INCREMENT:
        case AST_EXPRESSION_UNARY_PRE_DECREMENT:
        case AST_EXPRESSION_UNARY_POST_DECREMENT:
        case AST_EXPRESSION_UNARY_SUB:
        case AST_EXPRESSION_BINARY_ADD:
        case AST_EXPRESSION_BINARY_SUB:
        case AST_EXPRESSION_BINARY_MUL:
        case AST_EXPRESSION_BINARY_DIV:
        case AST_EXPRESSION_BINARY_MOD: {
            return true;
        }
        default: {return false;}
    }
}

internal bool is_boolean(AST_EXPRESSION_TYPE type) {
    switch(type) {
        case AST_EXPRESSION_UNARY_LOGICAL_NOT:
        case AST_EXPRESSION_BINARY_LOGICAL_OR:
        case AST_EXPRESSION_BINARY_LOGICAL_AND: {
            return true;
        }
        default: {return false;}
    }
}

internal bool is_assignment(AST_EXPRESSION_TYPE type) {
    if (type == AST_EXPRESSION_BINARY_ASSIGNMENT) {
        return true;
    }

    return false;
}

internal bool is_bit_operator(AST_EXPRESSION_TYPE type) {
    switch(type) {
        case AST_EXPRESSION_UNARY_BITWISE_NOT:
        case AST_EXPRESSION_BINARY_BITWISE_OR:
        case AST_EXPRESSION_BINARY_BITWISE_AND: {
            return true;
        }
        default: {return false;}
    }
}


internal Ast_expression *get_left_leaf(Ast_expression *ast) {
    if (is_binary(ast->type)) {
        return get_left_leaf(ast->binary.left);
    }

    if (is_unary(ast->type)) {
        return ast;
    }

    assert(is_leaf(ast->type), "the last option in get_right_leaf is to be a leaf node");

    return ast;
}

internal Ast_expression *get_right_leaf(Ast_expression *ast) {
    if (is_binary(ast->type)) {
        return get_right_leaf(ast->binary.right);
    }

    if (is_unary(ast->type)) {
        return get_right_leaf(ast->binary.right);
    }

    assert(is_leaf(ast->type), "the last option in get_right_leaf is to be a leaf node");

    return ast;
}

//
// DECLARATIONS
//

enum AST_DECLARATION_TYPE {
    AST_DECLARATION_NONE,

    AST_DECLARATION_VARIABLE,
    AST_DECLARATION_FUNCTION,
    AST_DECLARATION_COMPOUND,
    AST_DECLARATION_ENUM,
    AST_DECLARATION_TYPEDEF,
    AST_DECLARATION_CONSTANT,
    AST_DECLARATION_OPERATOR,
/*
    import
*/
    AST_DECLARATION_COUNT,
};

internal char *to_string(AST_DECLARATION_TYPE type) {
    switch (type) {
        case AST_DECLARATION_VARIABLE: {return "VARIABLE";}
        default: return "ERROR";
    }
}

struct Parameter {
    Type_spec *type;
    Src_position src_p;
    str name;
    Parameter *next;
};

enum MEMBER_TYPE {
    MEMBER_NONE,

    MEMBER_SIMPLE,
    MEMBER_COMPOUND,

    MEMBER_COUNT,
};

struct Compound;
internal void print(Compound *compound, u32 indent=0);

struct Member {
    MEMBER_TYPE member_type;
    Src_position src_p;

    union {
        struct {
            str type_name;
            str name;
        };
        Compound *sub_compound;
    };

    Member *next;
};

internal void print(Member *member, u32 indent=0) {
    if (member->member_type == MEMBER_SIMPLE) {
        print_indent(indent);
        printf("%.*s ", STR_PRINT(member->name));
        printf("%.*s ", STR_PRINT(member->type_name));
    } else {
        print(member->sub_compound, indent+4);
    }
}

enum COMPOUND_TYPE {
    COMPOUND_NONE,

    COMPOUND_STRUCT,
    COMPOUND_UNION,

    COMPOUND_COUNT,
};

struct Compound {
    COMPOUND_TYPE compound_type;
    Src_position src_p;
    u32 member_count;
    Member *members;
};

internal Compound *new_compound(Memory_pool *allocator) {
    Compound *result = push_struct(allocator, Compound);
    return result;
}

internal char *to_string(COMPOUND_TYPE type) {
    switch (type) {
        case COMPOUND_NONE:  {return "none";} break;
        case COMPOUND_STRUCT: {return "struct";} break;
        case COMPOUND_UNION: {return "union";} break;
        invalid_default_case_msg("COMPOUND_TYPE unrecognized");
    }

    return "ERROR";
}

internal void print(Compound *compound, u32 indent) {
    print_indent(indent);
    printf("<members: %d> {\n", compound->member_count);

    lfor(compound->members) {
        print(it, indent+4);
    }

    print_indent(indent);
    printf("}\n");
}

internal COMPOUND_TYPE token_type_to_compound_type(PEYOT_TOKEN_TYPE token_type) {
    if (token_type == TOKEN_STRUCT) {
        return COMPOUND_STRUCT;
    }

    if (token_type == TOKEN_UNION) {
        return COMPOUND_UNION;
    }

    return COMPOUND_NONE;
}

struct Enum_item {
    str name;
    Src_position src_p;
    Ast_expression *value;
};

struct Function {
    u32 param_count;
    Parameter *params;
    Type_spec *return_type;
    str return_type_name;
    Src_position return_src_p;
    Ast_block *block;
    bool needs_explicit_return;
    bool has_explicit_return;
    Symbol_table *current_scope;
};

struct Ast_declaration {
    AST_DECLARATION_TYPE type;
    Src_position src_p;
    str name;

    union {
        struct {
            Type_spec *variable_type;
            Ast_expression *expression;
            bool do_inference;
        } variable;
        Function *function;
        struct {
            PEYOT_TOKEN_TYPE operator_token;
            Function *declaration;
        } _operator;
        Compound *compound; // STRUCT/UNION
        struct {
            u32 item_count;
            Enum_item *items;
            Type_spec *enum_type;
        } _enum;
        struct {
            str new_type_name;
            Src_position new_type_name_src_p;
            str base_type_name;
            Src_position base_type_name_src_p;
        } _typedef;
    };
};

internal Ast_declaration *new_ast_declaration(Memory_pool *allocator) {
    Ast_declaration *result = push_struct(allocator, Ast_declaration);

    *result = {};

    return result;
}

internal void print(Ast_declaration *ast, u32 indent=0) {
    switch (ast->type) {
        case AST_DECLARATION_VARIABLE: {
            print_indent(indent);
            printf(ast->name);

            if (ast->variable.expression) {
                print(ast->variable.expression, indent, true);
            }

            putchar('\n');
        } break;
        case AST_DECLARATION_FUNCTION: {
            printf(ast->name);
            printf(" -> ");
            printf(ast->function->return_type->name);
            putchar('\n');
            print_indent(indent);

            sfor_count(ast->function->params, ast->function->param_count) {
                printf("%.*s :%.*s, ", STR_PRINT(it->name), STR_PRINT(it->type->name));
            }

            putchar('\n');
            print(ast->function->block);
        } break;
        case AST_DECLARATION_OPERATOR: {
            Function *function = ast->_operator.declaration;
            printf(to_symbol(ast->_operator.operator_token));
            printf(" -> ");
            printf(function->return_type->name);
            putchar('\n');
            print_indent(indent);

            sfor_count(function->params, function->param_count) {
                printf("%.*s :%.*s, ", STR_PRINT(it->name), STR_PRINT(it->type->name));
            }

            putchar('\n');
            print(function->block);
        } break;
        case AST_DECLARATION_COMPOUND: {
            Compound *c = ast->compound;
            print_indent(indent);
            printf("%s :: ", to_string(c->compound_type));
            printf(ast->name, false);
            putchar('\n');

            print(c);

            print_indent(indent);
            putchar('\n');
        } break;
        case AST_DECLARATION_ENUM: {
            print_indent(indent);
            printf("enum :: ");
            printf(ast->name, false);
            putchar('{');

            sfor_count(ast->_enum.items, ast->_enum.item_count) {
                if (it->value) {
                    printf("%.*s=", it->name.count, it->name.buffer);
                    print(it->value, 0, false, false);
                } else {
                    printf("%.*s, ", it->name.count, it->name.buffer);
                }
            }

            putchar('}');
            putchar('\n');
        } break;
        invalid_default_case_msg("missing print ast_declaration type");
    }
}

//
// LOOPS
//

struct Ast_loop {
    Src_position src_p;
    Ast_declaration *pre;
    Ast_expression *condition;
    Ast_expression *post;
    Ast_block *block;
    Symbol_table *current_scope;
    struct Tag *loop_tag;
    struct Tag *end_tag;
    struct Tag *post_tag;
};

internal Ast_loop *new_ast_loop(Memory_pool *allocator) {
    Ast_loop *result = push_struct(allocator, Ast_loop);

    *result = {};

    return result;
}

internal void print(Ast_loop *ast, u32 indent=0) {
    printf("%*sloop:\n", indent, "");
    print_indent(indent);
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
    AST_STATEMENT_BREAK,
    AST_STATEMENT_CONTINUE,
    AST_STATEMENT_RETURN,
    AST_STATEMENT_SIZEOF,
    AST_STATEMENT_OFFSETOF,
    AST_STATEMENT_TYPEOF,

    AST_STATEMENT_COUNT,
};

struct Ast_statement {
    AST_STATEMENT_TYPE type;
    Src_position src_p;

    union {
        struct {
            Ast_block *block;
            Symbol_table *current_scope;
        } block_statement;
        Ast_if *if_statement;
        Ast_expression *expression_statement;
        Ast_declaration *declaration_statement;
        Ast_statement *break_continue_loop;
        struct {
            Ast_declaration *function;
            Ast_expression *return_expression;
        } return_statement;
        Ast_loop *loop_statement;
        struct {
            str name;
            Src_position name_src_p;
            Ast_expression *expression;
        } sizeof_statement;
        struct {
            str type_name;
            Src_position type_src_p;
            str member_name;
            Src_position member_src_p;
            Ast_expression *expression;
        } offsetof_statement;
        struct {
            str name;
            Src_position name_src_p;
            Ast_expression *expression;
        } type_statement;
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
    Src_position src_p;
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
struct If {
    Src_position src_p;
    Ast_expression *condition;
    Ast_block block;
    Symbol_table *current_scope;
    If *next;
};

struct Ast_if {
    Src_position src_p;
    u32 if_count;
    If *ifs;

    Ast_block *else_block;
    Symbol_table *else_scope;
};

internal void print(Ast_if *ast, u32 indent=0) {
    print_indent(indent);
    bool first = true;

    lfor (ast->ifs) {
        print_indent(indent);

        if (first) {
            printf("if ");
        } else {
            printf("if else");
        }

        print(it->condition);
        print(&it->block, indent);
    }

    if (ast->else_block) {
        print_indent(indent);
        printf("else\n");
        print(ast->else_block, indent);
    }
}

internal void print(Ast_statement *ast, u32 indent, bool print_leaf) {
    switch (ast->type) {
        case AST_STATEMENT_BLOCK: {
            print(ast->block_statement.block, indent);
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
        case AST_STATEMENT_BREAK: {
            print_indent(indent);
            printf("break\n");
        } break;
        case AST_STATEMENT_CONTINUE: {
            print_indent(indent);
            printf("continue\n");
        } break;
        case AST_STATEMENT_RETURN: {
            print_indent(indent);
            printf("return\n");
        } break;
        case AST_STATEMENT_SIZEOF: {
            if (print_leaf) {
                print_indent(indent);
                printf("sizeof(");
                print(ast->sizeof_statement.expression);
                printf(")\n");
            } else {
                print(ast->sizeof_statement.expression);
            }
        } break;
        case AST_STATEMENT_OFFSETOF: {
            if (print_leaf) {
                print_indent(indent);
                printf("offsetof(%.*s, %.*s)\n", STR_PRINT(ast->offsetof_statement.type_name), STR_PRINT(ast->offsetof_statement.member_name));
            } else {
                print(ast->offsetof_statement.expression);
            }
        } break;
        case AST_STATEMENT_TYPEOF: {
            if (print_leaf) {
                print_indent(indent);
                printf("type(");
                print(ast->type_statement.expression);
                printf(")\n");
            } else {
                print(ast->type_statement.expression);
            }
        } break;
    }
}

#define AST_DECLARATIONS_PER_NODE 32

struct Ast_program {
    u32 declaration_count;
    Ast_declaration declarations[AST_DECLARATIONS_PER_NODE];

    Ast_program *next;
};

internal Ast_program *new_ast_program(Memory_pool *allocator) {
    Ast_program *result = push_struct(allocator, Ast_program);

    result->declaration_count = 0;
    result->next = 0;

    return result;
}

internal void print(Ast_program *ast) {
    while (ast) {
        sfor_count (ast->declarations, ast->declaration_count) {
            print(it);
        }

        ast = ast->next;
    }
}
