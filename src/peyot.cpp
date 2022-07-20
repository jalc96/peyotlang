#include"types.h"

#include<stdio.h>
#include<stdlib.h>


#include"debug.h"

/*
string
*/

struct str {
    u32 count;
    char *buffer;
};

internal u32 length(str string) {
    return string.count;
}

internal u32 length(str *string) {
    return string->count;
}

internal u32 length(char *buffer) {
    u32 c0 = (u32)buffer;

    while (*buffer) {
        buffer++;
    }

    u32 result = (u32)buffer - c0;
    return result;
}

/*
string
*/

/*
######## TODO ########
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
-stream files by chunk
-create a macro for assert and default_case_error and replace DEFAULT_CASE_STUB with the correct one
*/

#define DEFAULT_CASE_STUB

enum AST_TYPE {
    AST_NULL,
    AST_VARIABLE,
    AST_ASSIGNMENT,
    AST_BLOCK,



    AST_COUNT,
};

struct Ast_expression {
    AST_TYPE type;
    Ast_expression *left;
    Ast_expression *right;
};

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

enum TOKEN_TYPE {
    TOKEN_NULL,

    TOKEN_TYPE_DECLARATION,
    TOKEN_VARIABLE,
    TOKEN_EQUALS,
    TOKEN_LITERAL_INTEGER,
    TOKEN_SEMICOLON,
    TOKEN_EOL,

    TOKEN_BINARY_PLUS,
    TOKEN_BINARY_MINUS,
    TOKEN_BINARY_MUL,
    TOKEN_BINARY_DIV,

    TOKEN_EOF,




    TOKEN_COUNT,
};

internal char *to_string(TOKEN_TYPE type) {
    switch (type){
        case TOKEN_NULL: {
            return "TOKEN_NULL";
        } break;

        case TOKEN_TYPE_DECLARATION: {
            return "TOKEN_TYPE_DECLARATION";
        } break;

        case TOKEN_VARIABLE: {
            return "TOKEN_VARIABLE";
        } break;

        case TOKEN_EQUALS: {
            return "TOKEN_EQUALS";
        } break;

        case TOKEN_LITERAL_INTEGER: {
            return "TOKEN_LITERAL_INTEGER";
        } break;

        case TOKEN_SEMICOLON: {
            return "TOKEN_SEMICOLON";
        } break;

        case TOKEN_EOL: {
            return "TOKEN_EOL";
        } break;

        case TOKEN_BINARY_PLUS: {
            return "TOKEN_BINARY_PLUS";
        } break;

        case TOKEN_BINARY_MINUS: {
            return "TOKEN_BINARY_MINUS";
        } break;

        case TOKEN_BINARY_MUL: {
            return "TOKEN_BINARY_MUL";
        } break;

        case TOKEN_BINARY_DIV: {
            return "TOKEN_BINARY_DIV";
        } break;

        case TOKEN_EOF: {
            return "TOKEN_EOF";
        } break;

        case TOKEN_COUNT: {
            return "TOKEN_COUNT";
        } break;
DEFAULT_CASE_STUB
    }
}

struct Token {
    TOKEN_TYPE type;
    u32 line;
    u32 c0, cf;
};

struct Lexer {
    char *buffer;
    u32 size;
    u32 index;
    u32 current_line;
};


internal Lexer create_lexer(char *program) {
    Lexer result;

    result.buffer = program;
    result.size = length(program);
    result.index = 0;
    result.current_line = 0;

    return result;
}

internal char get_char(Lexer *lexer) {
    char result = lexer->buffer[lexer->index];
    return result;
}

internal void advance(Lexer *lexer) {
    lexer->index++;
}

internal Token get_next_token(Lexer *lexer) {
    Token result;
    result.c0 = lexer->index;
    result.line = lexer->current_line;

    char c = get_char(lexer);

    if (lexer->index >= lexer->size) {
        result.type = TOKEN_EOF;
    } else if (c == '\n') {
        result.type = TOKEN_EOL;
        lexer->current_line++;
        advance(lexer);
    } else {
        result.type = TOKEN_EQUALS;
        advance(lexer);
    }

    result.cf = lexer->index;
    return result;
}

internal Ast *parse_program(Lexer *lexer) {
    Ast *result = {};

    Token token = get_next_token(lexer);

    while (token.type != TOKEN_EOF) {
        if (token.type == TOKEN_EOL) {
            printf("%d[%d:%d]: %s\n", token.line, token.c0, token.cf, to_string(token.type));
        }

        token = get_next_token(lexer);
    }

    return result;
}


s16 main(s16 arg_count, char **args) {

    char *program = R"PROGRAM(
u32 a = 1;
u32 b = 2;
u32 c = a + b;
)PROGRAM";
    
    Lexer lexer = create_lexer(program);
    debug(lexer.buffer);
    debug(lexer.size);
    debug(lexer.index);
    debug(lexer.current_line);

    parse_program(&lexer);

    return 0;
}
