internal str get_name(Lexer *lexer) {
    str result;
    result.buffer = lexer->source.buffer + lexer->index;
    result.count = 0;
    char c = get_char(lexer);

    while (is_alpha_numeric(c) || c == '_') {
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
    PEYOT_TOKEN_TYPE type;
};

internal Token get_next_token(Lexer *lexer) {
    char c = get_char(lexer);

    // TODO: thest what happend in this loop with a ill-defined program, maybe it also needs the lexer_finished(Lexer *lexer) function, maybe not...
    while (is_whitespace(c) || is_eol(c)) {
        if (is_eol(c)) {
            lexer->current_line++;
        }

        advance(lexer);
        c = get_char(lexer);
    }

    Token result = {};
    result.src_p.c0 = lexer->index;
    result.src_p.line = lexer->current_line;

    if (lexer->index >= length(lexer->source)) {
        result.type = TOKEN_EOF;
    } else if (c == '\n') {
        result.type = TOKEN_EOL;
        lexer->current_line++;
        advance(lexer);
    } else if (c == ':') {
        result.type = TOKEN_COLON;
        advance(lexer);
        c = get_char(lexer);

        if (c == ':') {
            result.type = TOKEN_DECLARATION;
            advance(lexer);
        }
    } else if (c == ';') {
        result.type = TOKEN_SEMICOLON;
        advance(lexer);
    } else if (c == ',') {
        result.type = TOKEN_COMMA;
        advance(lexer);
    } else if (c == '.') {
        result.type = TOKEN_DOT;
        advance(lexer);
    } else if (c == '+') {
        result.type = TOKEN_ADD;
        advance(lexer);
    } else if (c == '-') {
        result.type = TOKEN_SUB;
        advance(lexer);
        c = get_char(lexer);

        if (c == '>') {
            result.type = TOKEN_RETURN_ARROW;
            advance(lexer);
        }
    } else if (c == '~') {
        result.type = TOKEN_UNARY_BITWISE_NOT;
        advance(lexer);
    } else if (c == '!') {
        result.type = TOKEN_UNARY_LOGICAL_NOT;

        advance(lexer);
        c = get_char(lexer);

        if (c == '=') {
            result.type = TOKEN_NOT_EQUALS;
            advance(lexer);
        }
    } else if (c == '&') {
        result.type = TOKEN_BITWISE_AND;

        advance(lexer);
        c = get_char(lexer);

        if (c == '&') {
            result.type = TOKEN_LOGICAL_AND;
            advance(lexer);
        }
    } else if (c == '|') {
        result.type = TOKEN_BITWISE_OR;

        advance(lexer);
        c = get_char(lexer);

        if (c == '|') {
            result.type = TOKEN_LOGICAL_OR;
            advance(lexer);
        }
    } else if (c == '*') {
        result.type = TOKEN_MUL;
        advance(lexer);
    } else if (c == '/') {
        result.type = TOKEN_DIV;
        advance(lexer);
    } else if (c == '%') {
        result.type = TOKEN_MOD;
        advance(lexer);
    } else if (c == '<') {
        result.type = TOKEN_LESS_THAN;

        advance(lexer);
        c = get_char(lexer);

        if (c == '=') {
            result.type = TOKEN_LESS_THAN_OR_EQUALS;
            advance(lexer);
        }
    } else if (c == '>') {
        result.type = TOKEN_GREATER_THAN;

        advance(lexer);
        c = get_char(lexer);

        if (c == '=') {
            result.type = TOKEN_GREATER_THAN_OR_EQUALS;
            advance(lexer);
        }
    } else if (c == '(') {
        result.type = TOKEN_OPEN_PARENTHESIS;
        advance(lexer);
    } else if (c == ')') {
        result.type = TOKEN_CLOSE_PARENTHESIS;
        advance(lexer);
    } else if (c == '{') {
        result.type = TOKEN_OPEN_BRACE;
        advance(lexer);
    } else if (c == '}') {
        result.type = TOKEN_CLOSE_BRACE;
        advance(lexer);
    } else if (c == '=') {
        result.type = TOKEN_ASSIGNMENT;

        advance(lexer);
        c = get_char(lexer);

        if (c == '=') {
            result.type = TOKEN_BINARY_EQUALS;
            advance(lexer);
        }
    } else if (is_alpha(c)) {
        result.type = TOKEN_NAME;
        result.name = get_name(lexer);

        /* C keywords
            [_] double
            [_] int
            [_] long
            [_] char
            [_] short
            [_] float
            [_] unsigned
            [_] signed
            [_] void

            [_] auto
            [_] const
            [_] volatile
            [_] static
            [_] extern

            [_] register

            [X] struct
            [X] union
            [X] enum
            [_] typedef

            [X] if
            [X] else
            [_] switch
            [_] case
            [_] default

            [X] for
            [X] while
            [_] do
            [_] goto
            [X] break
            [X] continue
            [X] return

            [_] sizeof
        */
        Keyword_match keywords[] = {
            {STATIC_STR("u32"), TOKEN_U32},
            {STATIC_STR("if"), TOKEN_IF},
            {STATIC_STR("else"), TOKEN_ELSE},
            {STATIC_STR("for"), TOKEN_FOR},
            {STATIC_STR("while"), TOKEN_WHILE},
            {STATIC_STR("struct"), TOKEN_STRUCT},
            {STATIC_STR("union"), TOKEN_UNION},
            {STATIC_STR("enum"), TOKEN_ENUM},
            {STATIC_STR("break"), TOKEN_BREAK},
            {STATIC_STR("continue"), TOKEN_CONTINUE},
            {STATIC_STR("return"), TOKEN_RETURN},
        };

        sfor (keywords) {
            if (equals(result.name, it->pattern)) {
                result.type = it->type;
            }
        }
    } else if (is_numeric(c)) {
        get_numeric_token(lexer, &result);
    } else {
        result.type = TOKEN_NULL;
        advance(lexer);
    }

    result.src_p.cf = lexer->index;
    lexer->previous_token = lexer->current_token;
    lexer->current_token = result;


    Token_stack *debug = push_struct(&lexer->debug_allocator, Token_stack);
    debug->token = result;
    debug->next = lexer->debug_token_stack;
    lexer->debug_token_stack = debug;


    return result;
}

internal void require_token(Lexer *lexer, PEYOT_TOKEN_TYPE type, const char message[64]) {
    // TODO: handle here some parsing errors
    Token t = lexer->current_token;
    char m[128];
    sprintf(m, "error in %s, <%s> expected, <%s> found", message, to_string(type), to_string(t.type));
    assert(t.type == type, m);
    get_next_token(lexer);
}

internal bool parsing_errors(Lexer *lexer) {
    return false;
}
