enum PEYOT_TOKEN_TYPE {
    TOKEN_NULL,

    TOKEN_CHAR,
    TOKEN_LITERAL_CHAR,

    TOKEN_STR,
    TOKEN_LITERAL_STR,

    TOKEN_U8,
    TOKEN_U16,
    TOKEN_U32,
    TOKEN_U64,

    TOKEN_S8,
    TOKEN_S16,
    TOKEN_S32,
    TOKEN_S64,
    TOKEN_LITERAL_INTEGER,

    TOKEN_F32,
    TOKEN_F64,
    TOKEN_LITERAL_FLOAT,

    TOKEN_BOOL,
    TOKEN_LITERAL_BOOL_TRUE,
    TOKEN_LITERAL_BOOL_FALSE,

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
    TOKEN_DECLARATION,
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

    TOKEN_OPEN_BRACE,
    TOKEN_CLOSE_BRACE,
    TOKEN_OPEN_PARENTHESIS,
    TOKEN_CLOSE_PARENTHESIS,

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

internal str to_string(PEYOT_TOKEN_TYPE type) {
    str result = STATIC_STR("ERROR");

    switch (type){
        case TOKEN_NULL: { result = STATIC_STR("TOKEN_NULL");} break;

        case TOKEN_CHAR: { result = STATIC_STR("TOKEN_CHAR");} break;
        case TOKEN_LITERAL_CHAR: { result = STATIC_STR("TOKEN_LITERAL_CHAR");} break;

        case TOKEN_STR: { result = STATIC_STR("TOKEN_STR");} break;
        case TOKEN_LITERAL_STR: { result = STATIC_STR("TOKEN_LITERAL_STR");} break;

        case TOKEN_U8: { result = STATIC_STR("TOKEN_U8");} break;
        case TOKEN_U16: { result = STATIC_STR("TOKEN_U16");} break;
        case TOKEN_U32: { result = STATIC_STR("TOKEN_U32");} break;
        case TOKEN_U64: { result = STATIC_STR("TOKEN_U64");} break;

        case TOKEN_S8: { result = STATIC_STR("TOKEN_S8");} break;
        case TOKEN_S16: { result = STATIC_STR("TOKEN_S16");} break;
        case TOKEN_S32: { result = STATIC_STR("TOKEN_S32");} break;
        case TOKEN_S64: { result = STATIC_STR("TOKEN_S64");} break;
        case TOKEN_LITERAL_INTEGER: { result = STATIC_STR("TOKEN_LITERAL_INTEGER");} break;

        case TOKEN_F32: { result = STATIC_STR("TOKEN_F32");} break;
        case TOKEN_F64: { result = STATIC_STR("TOKEN_F64");} break;
        case TOKEN_LITERAL_FLOAT: { result = STATIC_STR("TOKEN_LITERAL_FLOAT");} break;

        case TOKEN_BOOL: { result = STATIC_STR("TOKEN_BOOL");} break;
        case TOKEN_LITERAL_BOOL_TRUE: { result = STATIC_STR("TOKEN_LITERAL_BOOL_TRUE");} break;
        case TOKEN_LITERAL_BOOL_FALSE: { result = STATIC_STR("TOKEN_LITERAL_BOOL_FALSE");} break;

        case TOKEN_NAME: { result = STATIC_STR("TOKEN_NAME");} break;
        case TOKEN_COLON: { result = STATIC_STR("TOKEN_COLON");} break;
        case TOKEN_DECLARATION: { result = STATIC_STR("TOKEN_DECLARATION");} break;
        case TOKEN_SEMICOLON: { result = STATIC_STR("TOKEN_SEMICOLON");} break;
        case TOKEN_COMMA: { result = STATIC_STR("TOKEN_COMMA");} break;
        case TOKEN_DOT: { result = STATIC_STR("TOKEN_DOT");} break;
        case TOKEN_EOL: { result = STATIC_STR("TOKEN_EOL");} break;
        case TOKEN_EOF: { result = STATIC_STR("TOKEN_EOF");} break;

        case TOKEN_ASSIGNMENT: { result = STATIC_STR("TOKEN_ASSIGNMENT");} break;
        case TOKEN_ADD: { result = STATIC_STR("TOKEN_ADD");} break;
        case TOKEN_SUB: { result = STATIC_STR("TOKEN_SUB");} break;
        case TOKEN_MUL: { result = STATIC_STR("TOKEN_MUL");} break;
        case TOKEN_DIV: { result = STATIC_STR("TOKEN_DIV");} break;
        case TOKEN_MOD: { result = STATIC_STR("TOKEN_MOD");} break;

        case TOKEN_BINARY_EQUALS: { result = STATIC_STR("TOKEN_BINARY_EQUALS");} break;
        case TOKEN_NOT_EQUALS: { result = STATIC_STR("TOKEN_NOT_EQUALS");} break;
        case TOKEN_GREATER_THAN: { result = STATIC_STR("TOKEN_GREATER_THAN");} break;
        case TOKEN_GREATER_THAN_OR_EQUALS: { result = STATIC_STR("TOKEN_GREATER_THAN_OR_EQUALS");} break;
        case TOKEN_LESS_THAN: { result = STATIC_STR("TOKEN_LESS_THAN");} break;
        case TOKEN_LESS_THAN_OR_EQUALS: { result = STATIC_STR("TOKEN_LESS_THAN_OR_EQUALS");} break;

        case TOKEN_UNARY_BITWISE_NOT: { result = STATIC_STR("TOKEN_UNARY_BITWISE_NOT");} break;
        case TOKEN_BITWISE_AND: { result = STATIC_STR("TOKEN_BITWISE_AND");} break;
        case TOKEN_BITWISE_OR: { result = STATIC_STR("TOKEN_BITWISE_OR");} break;

        case TOKEN_UNARY_LOGICAL_NOT: { result = STATIC_STR("TOKEN_UNARY_LOGICAL_NOT");} break;
        case TOKEN_LOGICAL_AND: { result = STATIC_STR("TOKEN_LOGICAL_AND");} break;
        case TOKEN_LOGICAL_OR: { result = STATIC_STR("TOKEN_LOGICAL_OR");} break;

        case TOKEN_RETURN_ARROW: { result = STATIC_STR("TOKEN_RETURN_ARROW");} break;
        case TOKEN_IF: { result = STATIC_STR("TOKEN_IF");} break;
        case TOKEN_ELSE: { result = STATIC_STR("TOKEN_ELSE");} break;
        case TOKEN_FOR: { result = STATIC_STR("TOKEN_FOR");} break;
        case TOKEN_WHILE: { result = STATIC_STR("TOKEN_WHILE");} break;
        case TOKEN_STRUCT: { result = STATIC_STR("TOKEN_STRUCT");} break;
        case TOKEN_UNION: { result = STATIC_STR("TOKEN_UNION");} break;
        case TOKEN_ENUM: { result = STATIC_STR("TOKEN_ENUM");} break;
        case TOKEN_OPEN_BRACE: { result = STATIC_STR("TOKEN_OPEN_BRACE");} break;
        case TOKEN_CLOSE_BRACE: { result = STATIC_STR("TOKEN_CLOSE_BRACE");} break;
        case TOKEN_OPEN_PARENTHESIS: { result = STATIC_STR("TOKEN_OPEN_PARENTHESIS");} break;
        case TOKEN_CLOSE_PARENTHESIS: { result = STATIC_STR("TOKEN_CLOSE_PARENTHESIS");} break;
        case TOKEN_BREAK: { result = STATIC_STR("TOKEN_BREAK");} break;
        case TOKEN_CONTINUE: { result = STATIC_STR("TOKEN_CONTINUE");} break;
        case TOKEN_RETURN: { result = STATIC_STR("TOKEN_RETURN");} break;
        case TOKEN_COUNT: { result = STATIC_STR("TOKEN_COUNT");} break;

        invalid_default_case_msg("missing PEYOT_TOKEN_TYPE in to_string");
    }

    return result;
}

struct Src_position {
    u32 line;
    // cf is not inclusive, is the index to the next token's first character, so you can get the length by cf - c0
    u32 c0, cf;
};

internal u32 length(Src_position src_p) {
    u32 result = src_p.cf - src_p.c0;
    return result;
}

internal bool equals(Src_position a, Src_position b) {
    bool result = (
           a.line == b.line
        && a.c0 == b.c0
        && a.cf == b.cf
    );
}

internal Src_position merge(Src_position a, Src_position b) {
    Src_position result;

    result.line = al_min(a.line, b.line);
    result.c0 = al_min(a.c0, b.c0);
    result.cf = al_max(a.cf, b.cf);

    return result;
}

struct Token {
    PEYOT_TOKEN_TYPE type;
    Src_position src_p;

    union {
        u64 u64_value;
        s64 s64_value;
        f64 f64_value;
        char char_value;
        str str_value;
        str name;
    };
};

internal str to_symbol(PEYOT_TOKEN_TYPE type, Token *token = 0) {
    str result = STATIC_STR("ERROR");

    switch (type){
        case TOKEN_NULL: { result = STATIC_STR("null");} break;

        case TOKEN_CHAR: { result = STATIC_STR("char");} break;
        case TOKEN_STR: { result = STATIC_STR("str");} break;

        case TOKEN_U8: { result = STATIC_STR("u8");} break;
        case TOKEN_U16: { result = STATIC_STR("u16");} break;
        case TOKEN_U32: { result = STATIC_STR("u32");} break;
        case TOKEN_U64: { result = STATIC_STR("u64");} break;

        case TOKEN_S8: { result = STATIC_STR("s8");} break;
        case TOKEN_S16: { result = STATIC_STR("s16");} break;
        case TOKEN_S32: { result = STATIC_STR("s32");} break;
        case TOKEN_S64: { result = STATIC_STR("s64");} break;

        case TOKEN_F32: { result = STATIC_STR("f32");} break;
        case TOKEN_F64: { result = STATIC_STR("f64");} break;

        case TOKEN_BOOL: { result = STATIC_STR("bool");} break;

        case TOKEN_COLON: { result = STATIC_STR( ":");} break;
        case TOKEN_DECLARATION: { result = STATIC_STR( "::");} break;
        case TOKEN_SEMICOLON: { result = STATIC_STR( ";");} break;
        case TOKEN_COMMA: {result = STATIC_STR(",");} break;
        case TOKEN_DOT: { result = STATIC_STR( ".");} break;

        case TOKEN_EOL: { result = STATIC_STR("EOL");} break;
        case TOKEN_EOF: { result = STATIC_STR("EOF");} break;

        case TOKEN_ASSIGNMENT: { result = STATIC_STR( "=");} break;

        case TOKEN_ADD: { result = STATIC_STR( "+");} break;
        case TOKEN_SUB: { result = STATIC_STR( "-");} break;
        case TOKEN_MUL: { result = STATIC_STR( "*");} break;
        case TOKEN_DIV: { result = STATIC_STR( "/");} break;
        case TOKEN_MOD: { result = STATIC_STR( "%");} break;

        case TOKEN_BINARY_EQUALS: { result = STATIC_STR( "==");} break;
        case TOKEN_NOT_EQUALS: { result = STATIC_STR( "!=");} break;

        case TOKEN_GREATER_THAN: { result = STATIC_STR( ">");} break;
        case TOKEN_GREATER_THAN_OR_EQUALS: { result = STATIC_STR( ">=");} break;
        case TOKEN_LESS_THAN: { result = STATIC_STR( "<");} break;
        case TOKEN_LESS_THAN_OR_EQUALS: { result = STATIC_STR( "<=");} break;

        case TOKEN_UNARY_BITWISE_NOT: { result = STATIC_STR( "~");} break;
        case TOKEN_BITWISE_AND: { result = STATIC_STR( "&");} break;
        case TOKEN_BITWISE_OR: { result = STATIC_STR( "|");} break;

        case TOKEN_UNARY_LOGICAL_NOT: { result = STATIC_STR( "!");} break;
        case TOKEN_LOGICAL_AND: { result = STATIC_STR( "&&");} break;
        case TOKEN_LOGICAL_OR: { result = STATIC_STR( "||");} break;

        case TOKEN_RETURN_ARROW: { result = STATIC_STR( "->");} break;
        case TOKEN_IF: { result = STATIC_STR("if");} break;
        case TOKEN_ELSE: { result = STATIC_STR("else");} break;
        case TOKEN_FOR: { result = STATIC_STR("for");} break;
        case TOKEN_WHILE: { result = STATIC_STR("while");} break;
        case TOKEN_STRUCT: { result = STATIC_STR("struct");} break;
        case TOKEN_UNION: { result = STATIC_STR("union");} break;
        case TOKEN_ENUM: { result = STATIC_STR("enum");} break;
        case TOKEN_OPEN_BRACE: { result = STATIC_STR("{");} break;
        case TOKEN_CLOSE_BRACE: { result = STATIC_STR( "}");} break;
        case TOKEN_OPEN_PARENTHESIS: { result = STATIC_STR("(");} break;
        case TOKEN_CLOSE_PARENTHESIS: { result = STATIC_STR(")");} break;
        case TOKEN_BREAK: { result = STATIC_STR("break");} break;
        case TOKEN_CONTINUE: { result = STATIC_STR("continue");} break;
        case TOKEN_RETURN: { result = STATIC_STR("return");} break;

        case TOKEN_NAME: {
            char scratch[512];
            scratch[511] = 0;
            stbsp_snprintf(scratch, 512, "%.*s", token->name.count, token->name.buffer);
            result = STR(scratch);
        } break;

        case TOKEN_LITERAL_INTEGER: {
            char scratch[512];
            scratch[511] = 0;
            stbsp_snprintf(scratch, 512, "%llu", token->u64_value);
            result = STR(scratch);
        } break;

        case TOKEN_LITERAL_FLOAT: {
            char scratch[512];
            scratch[511] = 0;
            stbsp_snprintf(scratch, 512, "%f", token->f64_value);
            result = STR(scratch);
        } break;

        case TOKEN_LITERAL_CHAR: {
            char scratch[512];
            scratch[511] = 0;
            stbsp_snprintf(scratch, 512, "%c", token->char_value);
            result = STR(scratch);
        } break;

        case TOKEN_LITERAL_STR: {
            result = token->str_value;
        } break;


        invalid_default_case_msg("missing PEYOT_TOKEN_TYPE in to_symbol");
    }

    return result;
}

struct Parser;

struct Token_stack {
    Token token;
    Token_stack *next;
};

struct Lexer {
    str source;
    u32 index;
    u32 current_line;
    Token previous_token;
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
