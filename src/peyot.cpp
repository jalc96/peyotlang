#include"types.h"

#include<stdio.h>
#include<stdlib.h>


#include"debug.h"

/*
string
*/

internal u32 to_int(char c) {
    u32 result = c - '0';
    return result;
}

internal bool is_whitespace(char c) {
    if (c == ' ') return true;
    if (c == '\t') return true;
    if (c == '\r') return true;
    return false;
}

internal bool is_alpha(char c) {
    if ((c >= 'A') && (c <= 'Z')) return true;
    if ((c >= 'a') && (c <= 'z')) return true;
    return false;
}

internal bool is_numeric(char c) {
    if ((c >= '0') && (c <= '9')) return true;
    return false;
}

internal bool is_alpha_numeric(char c) {
    return (is_alpha(c) || is_numeric(c));
}

internal u32 length(char *buffer) {
    u64 c0 = (u64)buffer;

    while (*buffer) {
        buffer++;
    }

    u32 result = (u32)((u64)buffer - c0);
    return result;
}

struct str {
    u32 count;
    char *buffer;
};

internal str create_str(char *buffer) {
    str result;
    result.buffer = buffer;
    result.count = length(buffer);
    return result;
};

internal u32 length(str string) {
    return string.count;
}

internal u32 length(str *string) {
    return string->count;
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
-debugger for the virtual machine, showing the content of the registers and the content of the stack, show a window of the code (like 11 lines of assembly, the current one, 5 above and 5 below)
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
    TOKEN_ASSIGNMENT,
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

        case TOKEN_ASSIGNMENT: {
            return "TOKEN_ASSIGNMENT";
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

        invalid_default_case_msg("missing TOKEN_TYPE");
    }
}

struct Token {
    TOKEN_TYPE type;
    u32 line;
    u32 c0, cf;

    union {
        u64 u64_value;
        s64 s64_value;
        f64 f64_value;
        str str_value;
    };
};

struct Lexer {
    str source;
    u32 index;
    u32 current_line;
};


internal Lexer create_lexer(char *program) {
    Lexer result;

    result.source = create_str(program);
    result.index = 0;
    result.current_line = 0;

    return result;
}

internal char get_char(Lexer *lexer) {
    char result = lexer->source.buffer[lexer->index];
    return result;
}

internal void advance(Lexer *lexer) {
    lexer->index++;
}

internal str get_variable_name(Lexer *lexer) {
    str result;
    result.buffer = lexer->source.buffer + lexer->index;
    result.count = 0;
    char c = get_char(lexer);

    while (is_alpha_numeric(c)) {
        result.count++;
        advance(lexer);
        c = get_char(lexer);
    }

    return result;
}

internal void get_numeric_token(Lexer *lexer, Token *result) {
    u64 number = 0;
    char c = get_char(lexer);

    while (is_numeric(c)) {
        number *= 10;
        number += to_int(c);
        advance(lexer);
        c = get_char(lexer);
    }

    result->type = TOKEN_LITERAL_INTEGER;
    result->u64_value = number;
}

internal Token get_next_token(Lexer *lexer) {
    char c = get_char(lexer);

    while (is_whitespace(c)) {
        advance(lexer);
        c = get_char(lexer);
    }

    Token result;
    result.c0 = lexer->index;
    result.line = lexer->current_line;

    if (lexer->index >= length(lexer->source)) {
        result.type = TOKEN_EOF;
    } else if (c == '\n') {
        result.type = TOKEN_EOL;
        lexer->current_line++;
        advance(lexer);
    } else if (c == ';') {
        result.type = TOKEN_SEMICOLON;
        lexer->current_line++;
        advance(lexer);
    } else if (c == '+') {
        result.type = TOKEN_BINARY_PLUS;
        lexer->current_line++;
        advance(lexer);
    } else if (c == '-') {
        result.type = TOKEN_BINARY_MINUS;
        lexer->current_line++;
        advance(lexer);
    } else if (c == '*') {
        result.type = TOKEN_BINARY_MUL;
        lexer->current_line++;
        advance(lexer);
    } else if (c == '/') {
        result.type = TOKEN_BINARY_DIV;
        lexer->current_line++;
        advance(lexer);
    } else if (c == '=') {
        result.type = TOKEN_ASSIGNMENT;

        advance(lexer);
        c = get_char(lexer);

        if (c == '=') {
            result.type = TOKEN_EQUALS;
            advance(lexer);
        }
    // } else if (c == 'u') {
    //     // unsigned types
    //     advance(lexer);
    // } else if (c == 's') {
    //     // signed types
    //     advance(lexer);
    // } else if (c == 'f') {
    //     // float types
    //     advance(lexer);
    } else if (false) {
        // do something for the keywords like next_is_keyword(lexer) {get_keyword(lexer, &result);} that stores the state of the lexer and rewinds if next isnt a keyword
    } else if (is_alpha(c)) {
        result.type = TOKEN_VARIABLE;
        result.str_value = get_variable_name(lexer);
    } else if (is_numeric(c)) {
        get_numeric_token(lexer, &result);
    } else {
        result.type = TOKEN_NULL;
        advance(lexer);
    }

    result.cf = lexer->index;
    return result;
}

internal Ast *parse_program(Lexer *lexer) {
    Ast *result = {};

    Token token = get_next_token(lexer);

    while (token.type != TOKEN_EOF) {
        printf("%d[%d:%d]: %s", token.line, token.c0, token.cf, to_string(token.type));

        switch (token.type) {
            // case TOKEN_EOL: {
            // } break;
            case TOKEN_VARIABLE: {
                printf("<%.*s>", token.str_value.count, token.str_value.buffer);
                assert(token.str_value.count == (token.cf - token.c0), "the char offset selection in get_next_token is wrong");
            } break;

            case TOKEN_LITERAL_INTEGER: {
                printf("<%lld>", token.u64_value);
            } break;
        }

        putchar('\n');

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
    debug(lexer.source.buffer);
    debug(lexer.source.count);
    debug(lexer.index);
    debug(lexer.current_line);

    parse_program(&lexer);

    return 0;
}
