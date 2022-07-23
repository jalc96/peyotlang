enum TOKEN_TYPE {
    TOKEN_NULL,

    TOKEN_U32,
    TOKEN_LITERAL_U32,
    TOKEN_VARIABLE,

    TOKEN_IF,
    TOKEN_EQUALS,
    TOKEN_ASSIGNMENT,
    TOKEN_SEMICOLON,

    TOKEN_BINARY_ADD,
    TOKEN_BINARY_SUB,
    TOKEN_BINARY_MUL,
    TOKEN_BINARY_DIV,
    TOKEN_BINARY_MOD,

    TOKEN_EOL,
    TOKEN_EOF,




    TOKEN_COUNT,
};

internal bool is_add_operator(TOKEN_TYPE type) {
    return ((type == TOKEN_BINARY_ADD) || (type == TOKEN_BINARY_SUB));
}

internal bool is_mul_operator(TOKEN_TYPE type) {
    return ((type == TOKEN_BINARY_MUL) || (type == TOKEN_BINARY_DIV) || (type == TOKEN_BINARY_MOD));
}

internal PEYOT_TYPE token_type_to_peyot_type(TOKEN_TYPE token_type) {
    switch (token_type) {
        case TOKEN_U32: return TYPE_U32;
        // handle the struct declaration
        invalid_default_case_msg("impossible to translate TOKEN_TYPE to PEYOT_TYPE");
    }
}

internal char *to_string(TOKEN_TYPE type) {
    switch (type){
        case TOKEN_NULL: {
            return "TOKEN_NULL";
        } break;

        case TOKEN_U32: {
            return "TOKEN_U32";
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

        case TOKEN_LITERAL_U32: {
            return "TOKEN_LITERAL_U32";
        } break;

        case TOKEN_SEMICOLON: {
            return "TOKEN_SEMICOLON";
        } break;

        case TOKEN_EOL: {
            return "TOKEN_EOL";
        } break;

        case TOKEN_BINARY_ADD: {
            return "TOKEN_BINARY_ADD";
        } break;

        case TOKEN_BINARY_SUB: {
            return "TOKEN_BINARY_SUB";
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
        str variable_name;
    };
};

struct Lexer {
    str source;
    u32 index;
    u32 current_line;
    Token current_token;
    void *allocator;
    Symbol_table *symbol_table;
};


internal Lexer create_lexer(char *program) {
    Lexer result;

    result.source = create_str(program);
    result.index = 0;
    result.current_line = 1;

    return result;
}

internal char get_char(Lexer *lexer) {
    char result = lexer->source.buffer[lexer->index];
    return result;
}

internal void advance(Lexer *lexer, u32 c=1) {
    lexer->index += c;
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

    result->type = TOKEN_LITERAL_U32;
    result->u64_value = number;
}

struct Keyword_match {
    str pattern;
    TOKEN_TYPE type;
};

#define str_(static_string) {sizeof(static_string), static_string}

internal Token get_next_token(Lexer *lexer) {
    char c = get_char(lexer);

    while (is_whitespace(c) || is_eol(c)) {
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
        advance(lexer);
    } else if (c == '+') {
        result.type = TOKEN_BINARY_ADD;
        advance(lexer);
    } else if (c == '-') {
        result.type = TOKEN_BINARY_SUB;
        advance(lexer);
    } else if (c == '*') {
        result.type = TOKEN_BINARY_MUL;
        advance(lexer);
    } else if (c == '/') {
        result.type = TOKEN_BINARY_DIV;
        advance(lexer);
    } else if (c == '=') {
        result.type = TOKEN_ASSIGNMENT;

        advance(lexer);
        c = get_char(lexer);

        if (c == '=') {
            result.type = TOKEN_EQUALS;
            advance(lexer);
        }
    } else if (is_alpha(c)) {
        result.type = TOKEN_VARIABLE;
        result.variable_name = get_variable_name(lexer);

        Keyword_match keywords[] = {
            {str_("u32"), TOKEN_U32},
            {str_("if"), TOKEN_IF}
        };

        sfor (keywords) {
            if (match(result.variable_name, it->pattern)) {
                result.type = it->type;
            }
        }
    } else if (is_numeric(c)) {
        get_numeric_token(lexer, &result);
    } else {
        result.type = TOKEN_NULL;
        advance(lexer);
    }

    result.cf = lexer->index;
    lexer->current_token = result;
    return result;
}
