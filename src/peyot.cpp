/*
######## TODO ########
    -get a memory allocator in the platform layer
    -MODES:
        -compile: creates a binary into a .pvm file
        -run: runs a binary .pvm file with the virtual machine
    -dump the asm generated
    -take the ltl_interpreter path recomendator
    -windows layer, get rid of as much of std_lib/crt as possible
    -linux layer, get rid of as much of std_lib/crt as possible
    -asm segments: https://www.tutorialspoint.com/assembly_programming/assembly_quick_guide.htm
    -performance analysis probably multithread the asm creation just to test performance
    -you can set the stack size of the program and if overflowed an error is shown
    -debugger for the virtual machine, showing the content of the registers and the content of the stack, show a window of the code (like 11 lines of assembly, the current one, 5 above and 5 below)
    -function parsing is easier if a keyword is introduced for function declaration like "fn" or "func" "fn do_stuff(u32 a) -> u32 {return 2 * a;}"
    -error reporter that shows a sample of the code that created the error (a couple of lines above the current line should be enough)
    -get the stb_printf for compositing strings (for the ast debug print)
*/

#include"types.h"
#include"utils.h"

#include<stdio.h>
#include<stdlib.h>

#include"debug.h"
#include"string.h"


enum PEYOT_TYPE {
    TYPE_NULL,

    TYPE_U32,
    TYPE_CUSTOM,



    TYPE_COUNT,
};

#include"symbol_table.h"
#include"lexer.cpp"



enum AST_TYPE {
    AST_NULL,

    AST_LITERAL_U32,
    AST_VARIABLE,
    AST_ASSIGNMENT,
    AST_BLOCK,

    AST_BINARY_ADD,
    AST_BINARY_SUB,
    AST_BINARY_MUL,
    AST_BINARY_DIV,
    AST_BINARY_MOD,



    AST_COUNT,
};

internal AST_TYPE token_type_to_operation(TOKEN_TYPE token_type) {
    switch (token_type) {
        case TOKEN_BINARY_ADD: return AST_BINARY_ADD;
        case TOKEN_BINARY_SUB: return AST_BINARY_SUB;
        case TOKEN_BINARY_MUL: return AST_BINARY_MUL;
        case TOKEN_BINARY_DIV: return AST_BINARY_DIV;
        case TOKEN_BINARY_MOD: return AST_BINARY_MOD;
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

    Ast_expression *left;
    Ast_expression *right;
};

internal void print(Ast_expression *ast, u32 indent=0) {
    printf("%*s", indent, "");

    switch (ast->type) {
        case AST_LITERAL_U32: {printf("%lld\n", ast->u64_value);} break;
        case AST_BINARY_ADD: {printf("+:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_BINARY_SUB: {printf("-:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_BINARY_MUL: {printf("*:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_BINARY_DIV: {printf("/:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
        case AST_BINARY_MOD: {printf("%:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
    }
}

#define AST_EXPRESSIONS_PER_BLOCK_LINK 32

struct Ast_block {
    u32 expression_count;
    Ast_expression expressions[AST_EXPRESSIONS_PER_BLOCK_LINK];
    Ast_block *next;
};

struct Ast {
    AST_TYPE type;

    union {
        Ast_block block;
    };
};



internal Ast_expression *parse_basic_token(Token token) {
    Ast_expression *result = (Ast_expression *)malloc(sizeof(Ast_expression));

    switch (token.type) {
        case TOKEN_VARIABLE: {
            result->type = AST_VARIABLE;
            result->variable_name = token.variable_name;
        } break;

        case TOKEN_LITERAL_U32: {
            result->type = AST_LITERAL_U32;
            result->u64_value = token.u64_value;
        } break;

        invalid_default_case_msg("unable to parse basic token");
    }

    return result;
}

internal Ast_expression *parse_declaration(Lexer *lexer) {
    Ast_expression *result = 0;

    Token type = get_next_token(lexer);
    Token variable = get_next_token(lexer);
    Token next = get_next_token(lexer);

    // TODO: check if symbol already declared (track scope also) and log error
    Symbol *symbol = create_symbol(
        lexer->symbol_table,
        variable.variable_name,
        token_type_to_peyot_type(type.type)
    );
    put(lexer->symbol_table, symbol);

    if (next.type == TOKEN_ASSIGNMENT) {
        result = (Ast_expression *)malloc(sizeof(Ast_expression));
        result->type = AST_ASSIGNMENT;
        result->left = parse_basic_token(variable);
        Token right_side = get_next_token(lexer);
        result->right = parse_basic_token(right_side);
    } else {
        // TODO: do this with the error reporter
        assert(next.type == TOKEN_SEMICOLON, "now this is an assert next should be an error with check or something");
        result = parse_basic_token(variable);
    }


    return result;
}

/*
expression ::= term ([+-] term)*
term ::= unary_expression ([/*%] unary_expression)
unary_expression ::= [+-]* factor
factor ::= (expression) | var_name | function_call | basic_type
*/

internal void leaf(Ast_expression *ast, AST_TYPE type) {
    ast->type = type;
    ast->left = 0;
    ast->right = 0;
}

internal Ast_expression *parse_factor(Lexer *lexer) {
    Token token = get_next_token(lexer);
    Ast_expression *result = (Ast_expression *)malloc(sizeof(Ast_expression));

    switch (token.type){
        case TOKEN_VARIABLE: {
            leaf(result, AST_VARIABLE);
            result->variable_name = token.variable_name;
        } break;

        case TOKEN_LITERAL_U32: {
            leaf(result, AST_LITERAL_U32);
            result->u64_value = token.u64_value;
        } break;

        invalid_default_case_msg("create a error in parse_factor()");
    }

    get_next_token(lexer);

    return result;
}

internal Ast_expression *parse_term(Lexer *lexer) {
    Ast_expression *result = parse_factor(lexer);

    Token token = lexer->current_token;

    while (is_mul_operator(lexer->current_token.type)) {
        Ast_expression *operator_tree = (Ast_expression *)malloc(sizeof(Ast_expression));
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        operator_tree->right = parse_factor(lexer);
        result = operator_tree;
        
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_expression(Lexer *lexer) {
    Ast_expression *result = parse_term(lexer);

    Token token = lexer->current_token;

    while (is_add_operator(token.type)) {
        Ast_expression *operator_tree = (Ast_expression *)malloc(sizeof(Ast_expression));
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        operator_tree->right = parse_term(lexer);
        result = operator_tree;
        
        token = lexer->current_token;
    }

    return result;
}

internal Ast *parse_block(Lexer *lexer) {
    // Ast *result = parse_expression(lexer);
    // return result;
}

internal Ast *parse_program(Lexer *lexer) {
    // Ast *result = parse_block(lexer);
    // return result;
}

internal Ast *test_parser(Lexer *lexer) {
    Ast *result = {};

    Token token = get_next_token(lexer);

    while (token.type != TOKEN_EOF) {
        printf("%d[%d:%d]: %s", token.line, token.c0, token.cf, to_string(token.type));

        switch (token.type) {
            case TOKEN_VARIABLE: {
                printf("<%.*s>", token.variable_name.count, token.variable_name.buffer);
                assert(token.variable_name.count == (token.cf - token.c0), "the char offset selection in get_next_token is wrong");
            } break;

            case TOKEN_LITERAL_U32: {
                printf("<%lld>", token.u64_value);
            } break;
        }

        putchar('\n');

        token = get_next_token(lexer);
    }

    return result;
}


s16 main(s16 arg_count, char **args) {
    char *program1 = R"PROGRAM(
        u32 a = 1;
        u32 b = 2;
        u32 c = a + b;
    )PROGRAM";

    char *program = R"PROGRAM(
        1+2+3*2+4*4;
    )PROGRAM";
    
    Lexer lexer = create_lexer(program);
    lexer.symbol_table = create_symbol_table(0);
    debug(lexer.source.buffer);
    debug(lexer.source.count);
    debug(lexer.index);
    debug(lexer.current_line);

    Ast_expression *ast = parse_expression(&lexer);
    // test_parser(&lexer);
    print(ast);

    printf("finished correctly\n");

    return 0;
}
