enum PEYOT_TOKEN_TYPE {
    TOKEN_NULL,

    // TODO: maybe delete TOKEN_U32 and make all the types behave the same way??
    TOKEN_U32,
    TOKEN_LITERAL_U32,
    TOKEN_NAME,

    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_STRUCT,
    TOKEN_UNION,
    TOKEN_ENUM,

    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_RETURN,

    TOKEN_ASSIGNMENT,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,

    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_MOD,

    TOKEN_BINARY_EQUALS,
    TOKEN_NOT_EQUALS,
    TOKEN_GREATER_THAN,
    TOKEN_GREATER_THAN_OR_EQUALS,
    TOKEN_LESS_THAN,
    TOKEN_LESS_THAN_OR_EQUALS,

    TOKEN_UNARY_BITWISE_NOT,
    TOKEN_BITWISE_AND,
    TOKEN_BITWISE_OR,

    TOKEN_UNARY_LOGICAL_NOT,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,

    TOKEN_OPEN_BRACE = '{',
    TOKEN_CLOSE_BRACE = '}',
    TOKEN_OPEN_PARENTHESIS = '(',
    TOKEN_CLOSE_PARENTHESIS = ')',

    TOKEN_RETURN_ARROW,



    TOKEN_EOL,
    TOKEN_EOF,




    TOKEN_COUNT,
};

internal bool is_add_operator(PEYOT_TOKEN_TYPE type) {
    return ((type == TOKEN_ADD) || (type == TOKEN_SUB));
}

internal bool is_mul_operator(PEYOT_TOKEN_TYPE type) {
    return ((type == TOKEN_MUL) || (type == TOKEN_DIV) || (type == TOKEN_MOD));
}

internal char *to_string(PEYOT_TOKEN_TYPE type) {
    switch (type){
        case TOKEN_NULL: {return "TOKEN_NULL";} break;

        case TOKEN_U32: {return "TOKEN_U32";} break;

        case TOKEN_NAME: {return "TOKEN_NAME";} break;

        case TOKEN_LITERAL_U32: {return "TOKEN_LITERAL_U32";} break;

        case TOKEN_COLON: {return "TOKEN_COLON";} break;
        case TOKEN_SEMICOLON: {return "TOKEN_SEMICOLON";} break;
        case TOKEN_COMMA: {return "TOKEN_COMMA";} break;
        case TOKEN_DOT: {return "TOKEN_DOT";} break;

        case TOKEN_EOL: {return "TOKEN_EOL";} break;
        case TOKEN_EOF: {return "TOKEN_EOF";} break;

        case TOKEN_ASSIGNMENT: {return "TOKEN_ASSIGNMENT";} break;

        case TOKEN_ADD: {return "TOKEN_ADD";} break;
        case TOKEN_SUB: {return "TOKEN_SUB";} break;
        case TOKEN_MUL: {return "TOKEN_MUL";} break;
        case TOKEN_DIV: {return "TOKEN_DIV";} break;
        case TOKEN_MOD: {return "TOKEN_MOD";} break;

        case TOKEN_BINARY_EQUALS: {return "TOKEN_BINARY_EQUALS";} break;
        case TOKEN_NOT_EQUALS: {return "TOKEN_NOT_EQUALS";} break;
        case TOKEN_GREATER_THAN: {return "TOKEN_GREATER_THAN";} break;
        case TOKEN_GREATER_THAN_OR_EQUALS: {return "TOKEN_GREATER_THAN_OR_EQUALS";} break;
        case TOKEN_LESS_THAN: {return "TOKEN_LESS_THAN";} break;
        case TOKEN_LESS_THAN_OR_EQUALS: {return "TOKEN_LESS_THAN_OR_EQUALS";} break;

        case TOKEN_UNARY_BITWISE_NOT: {return "TOKEN_UNARY_BITWISE_NOT";} break;
        case TOKEN_BITWISE_AND: {return "TOKEN_BITWISE_AND";} break;
        case TOKEN_BITWISE_OR: {return "TOKEN_BITWISE_OR";} break;

        case TOKEN_UNARY_LOGICAL_NOT: {return "TOKEN_UNARY_LOGICAL_NOT";} break;
        case TOKEN_LOGICAL_AND: {return "TOKEN_LOGICAL_AND";} break;
        case TOKEN_LOGICAL_OR: {return "TOKEN_LOGICAL_OR";} break;

        case TOKEN_RETURN_ARROW: {return "TOKEN_RETURN_ARROW";} break;

        case TOKEN_IF: {return "TOKEN_IF";} break;
        case TOKEN_ELSE: {return "TOKEN_ELSE";} break;
        case TOKEN_FOR: {return "TOKEN_FOR";} break;
        case TOKEN_WHILE: {return "TOKEN_WHILE";} break;
        case TOKEN_STRUCT: {return "TOKEN_STRUCT";} break;
        case TOKEN_UNION: {return "TOKEN_UNION";} break;
        case TOKEN_ENUM: {return "TOKEN_ENUM";} break;

        case TOKEN_OPEN_BRACE: {return "TOKEN_OPEN_BRACE";} break;
        case TOKEN_CLOSE_BRACE: {return "TOKEN_CLOSE_BRACE";} break;
        case TOKEN_OPEN_PARENTHESIS: {return "TOKEN_OPEN_PARENTHESIS";} break;
        case TOKEN_CLOSE_PARENTHESIS: {return "TOKEN_CLOSE_PARENTHESIS";} break;

        case TOKEN_BREAK: {return "TOKEN_BREAK";} break;
        case TOKEN_CONTINUE: {return "TOKEN_CONTINUE";} break;
        case TOKEN_RETURN: {return "TOKEN_RETURN";} break;




        case TOKEN_COUNT: {return "TOKEN_COUNT";} break;

        invalid_default_case_msg("missing PEYOT_TOKEN_TYPE in to_string");
    }
}

struct Token {
    PEYOT_TOKEN_TYPE type;
    u32 line;
    u32 c0, cf;

    union {
        u64 u64_value;
        s64 s64_value;
        f64 f64_value;
        str str_value;
        str name;
    };
};

struct Parser;

struct Token_stack {
    Token token;
    Token_stack *next;
};

struct Lexer {
    str source;
    u32 index;
    u32 current_line;
    Token current_token;
    Parser *parser;
    Memory_pool *allocator;



    Token_stack *debug_token_stack;
    Memory_pool debug_allocator;
};

internal Lexer create_lexer(char *program, Parser *parser, Memory_pool *allocator) {
    Lexer result;

    result.source = create_str(program);
    result.index = 0;
    result.current_line = 1;
    result.parser = parser;
    result.allocator = allocator;


    result.debug_token_stack = 0;
    result.debug_allocator = {};

    return result;
}

struct Lexer_savepoint {
    Lexer *lexer;
    Lexer savepoint;
    Temporary_memory temp_memory_stuff_for_the_memory_arena;
};

internal Lexer_savepoint create_savepoint(Lexer *lexer) {
    Lexer_savepoint result;

    result.lexer = lexer;
    result.savepoint = *lexer;
    result.temp_memory_stuff_for_the_memory_arena = begin_temporary_memory(lexer->allocator);

    return result;
}

internal void rollback_lexer(Lexer_savepoint lexer_savepoint) {
    end_temporary_memory(lexer_savepoint.temp_memory_stuff_for_the_memory_arena);
    *lexer_savepoint.lexer = lexer_savepoint.savepoint;
}

internal bool lexer_finished(Lexer *lexer) {
    return lexer->current_token.type == TOKEN_EOF;
}

internal char get_char(Lexer *lexer) {
    char result = lexer->source.buffer[lexer->index];
    return result;
}

internal void advance(Lexer *lexer, u32 c=1) {
    lexer->index += c;
}
