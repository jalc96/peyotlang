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
    -function parsing is easier if a keyword is introduced for function declaration like "fn" or "func" "fn do_stuff(u32 a) -> u32 {return 2 * a;}" maybe "function" since a lot of languages use that
    -error reporter that shows a sample of the code that created the error (a couple of lines above the current line should be enough)
    -add introspection for example have .type in structs to check for the type of structs and also be able to iterate over the members of structs, implement this with an integer, each time a struct is declare the type integer is incremented and that is asign to the struct, the basic types of the language are the first numbers. Add also something like typedef??
    -add the hability to undefine variables with the keyword undef
    -get the stb_printf for compositing strings (for the ast debug print)
    -structs
    -arrays
    -variable names with _ and the other special characters allowed in variable names in c-like languages
    -add introspection, be able to check a struct type and iterate over struct members
    -type checking in the ast
    -generics struct v2 <T> {
        T x, y;
    }
    maybe expand the struct later to v2_u32 internally in the ast if v2<u32> is in the code and another like v2_f32 if v2<f32> is found, etc. These will just invoke to the create_type() thing or whatever i make for the structs, allow to check the type that was used in the T.
    -operator overload
    -before allocating the memory for the bytecode, calculate all of the constants sizes and take that into account
    -do string pooling in the .data segment for constant strings
    -interface with the OS to get memory/open_files/etc
    -grammar check with a function require_token(lexer, TOKEN_OPEN_BRACE); and maybe have an error bool in the lexer or something to check stuff??
    -try Casey's idea for integers: dont have signed/unsigned types, have only integers and when type is important in an operation (multiply/divide/shift/etc) show an error and ask the user to specify someway (figure out this) which type is going to be used
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

internal char *to_string(PEYOT_TYPE type) {
    switch(type) {
        case TYPE_NULL: {
            return "NULL";
        } break;
        case TYPE_U32: {
            return "U32";
        } break;
        case TYPE_CUSTOM: {
            return "CUSTOM";
        } break;
        case TYPE_COUNT: {
            return "COUNT";
        } break;
    }
}

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

    Symbol *symbol;
    Ast_expression *left;
    Ast_expression *right;
};

internal void print(Ast_expression *ast, u32 indent=0) {
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
        case AST_ASSIGNMENT:  {printf("=:\n"); print(ast->left, indent+4); print(ast->right, indent+4); } break;
    }
}

#define AST_EXPRESSIONS_PER_BLOCK_LINK 32

struct Ast_block;
struct Ast_if;
struct Ast_expression;

enum AST_STATEMENT_TYPE {
    AST_STATEMENT_NONE,

    AST_STATEMENT_BLOCK,
    AST_STATEMENT_IF,
    AST_STATEMENT_DECLARATION,
    AST_STATEMENT_EXPRESSION,

    AST_STATEMENT_COUNT,
};

struct Ast_statement {
    AST_STATEMENT_TYPE type;

    union {
        Ast_block *block_statement;
        Ast_if *if_statement;
        Ast_expression *expression_statement;
    };
};

internal Ast_statement *new_ast_statement(void *allocator) {
    Ast_statement *result = (Ast_statement *)malloc(sizeof(Ast_statement));

    *result = {};
    result->type = AST_STATEMENT_NONE;

    return result;
}

struct Ast_block {
    u32 statement_count;
    Ast_statement statements[AST_EXPRESSIONS_PER_BLOCK_LINK];
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
    if (it->i >= AST_EXPRESSIONS_PER_BLOCK_LINK) {
        it->i = 0;
        it->current = it->current->next;
    }

    Ast_statement *result = 0;

    if (it->current) {
        result = it->current->statements + it->i++;
    }

    return result;
}

internal void print(Ast_statement *ast, u32 indent=0);

internal void print(Ast_block *block, u32 indent=0) {
    Ast_block_iterator it = iterate(block);

    while (valid(it)) {
        Ast_statement *e = advance(&it);
        print(e, indent + 4);
    }
}

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
            print(ast->expression_statement, indent);
        } break;
    }
}


struct Ast {
    AST_TYPE type;

    union {
        Ast_block block;
    };
};



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

internal Ast_expression *parse_factor(Lexer *lexer, Ast_expression *result) {
    if (!result) {
        result = (Ast_expression *)malloc(sizeof(Ast_expression));
    }

    Token token = lexer->current_token;

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

internal Ast_expression *parse_term(Lexer *lexer, Ast_expression *result) {
    result = parse_factor(lexer, result);

    Token token = lexer->current_token;

    while (is_mul_operator(lexer->current_token.type)) {
        Ast_expression *operator_tree = (Ast_expression *)malloc(sizeof(Ast_expression));
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_factor(lexer, 0);
        result = operator_tree;
        
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_expression(Lexer *lexer, Ast_expression *result=0) {
    result = parse_term(lexer, result);

    Token token = lexer->current_token;

    while (is_add_operator(token.type)) {
        Ast_expression *operator_tree = (Ast_expression *)malloc(sizeof(Ast_expression));
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_term(lexer, 0);
        result = operator_tree;
        token = lexer->current_token;
    }

    return result;
}


internal Ast_expression *parse_basic_token(Token token, Ast_expression *result) {
    if (!result) {
        result = (Ast_expression *)malloc(sizeof(Ast_expression));
    }

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

internal Ast_expression *parse_declaration(Lexer *lexer, Ast_expression *result=0) {
    if (!result) {
        result = (Ast_expression *)malloc(sizeof(Ast_expression));
    }

    Token type = lexer->current_token;
    Token name = get_next_token(lexer);
    Token next = get_next_token(lexer);

    // TODO: check if symbol already declared (track scope also) and log error
    Symbol *symbol = create_symbol(
        lexer->symbol_table,
        name.variable_name,
        token_type_to_peyot_type(type.type)
    );
    put(lexer->symbol_table, symbol);

    if (next.type == TOKEN_ASSIGNMENT) {
        result->type = AST_ASSIGNMENT;
        result->left = parse_basic_token(name, 0);
        get_next_token(lexer);
        result->right = parse_expression(lexer, 0);
        Token semicolon = lexer->current_token;
        assert(semicolon.type == TOKEN_SEMICOLON, "now this is an assert next should be an error with check or something");
        // Consume the semicolon to start fresh
        get_next_token(lexer);
    } else {
        // TODO: do this with the error reporter
        assert(next.type == TOKEN_SEMICOLON, "now this is an assert next should be an error with check or something");
        result = parse_basic_token(name, result);
    }

    return result;
}

struct Block_parser {
    Lexer *lexer;
    bool finished;
};

internal bool parsing_block(Block_parser *parser) {
    return !parser->finished;
}

internal void update_block_parser(Block_parser *parser) {
    Token t = parser->lexer->current_token;
    parser->finished = (
           t.type == TOKEN_CLOSE_BRACE
        || lexer_finished(parser->lexer)
    );
}

struct Ast_block_creation_iterator {
    void *allocator;
    Ast_block *current;
};

internal Ast_block_creation_iterator iterate(Ast_block *block, void *allocator) {
    Ast_block_creation_iterator result;

    result.current = block;
    result.allocator = allocator;

    return result;
}

internal Ast_statement *advance(Ast_block_creation_iterator *it) {
    if (it->current->statement_count >= AST_EXPRESSIONS_PER_BLOCK_LINK) {
        Ast_block *t = it->current;
        t->next = new_ast_block(it->allocator);
        it->current = t->next;
    }

    Ast_statement *result = it->current->statements + it->current->statement_count++;

    return result;
}

internal Ast_statement *parse_statement(Lexer *lexer, Ast_statement *result);

internal Ast_block *parse_block(Lexer *lexer, Ast_block *result=0) {
    if (!result) {
        result = new_ast_block(0);
    }

    assert(lexer->current_token.type == TOKEN_OPEN_BRACE, "when parsing a block, the current token must be an open brace '{'");
    get_next_token(lexer);

    Block_parser parser;
    parser.lexer = lexer;
    parser.finished = false;
    Ast_block_creation_iterator it = iterate(result, 0);

    while (parsing_block(&parser)) {
        Ast_statement *e = advance(&it);
        parse_statement(lexer, e);
        update_block_parser(&parser);
    }

    assert(lexer->current_token.type == TOKEN_CLOSE_BRACE, "when parsing a block, the last current token must be a close brace '}'");
    get_next_token(lexer);

    return result;
}

internal Ast_if *parse_if(Lexer *lexer, Ast_if *result=0) {
    if (!result) {
        result = new_ast_if(0);
    }

    require_token(lexer, TOKEN_IF);
    get_next_token(lexer);
    require_token(lexer, TOKEN_OPEN_PARENTHESIS);
    get_next_token(lexer);

    if (parsing_errors(lexer)) {
        // TODO: handle errors this way and have a buffer in the Lexer struct to store the error message
        return 0;
    }

    result->condition = *parse_expression(lexer, &result->condition);
    require_token(lexer, TOKEN_CLOSE_PARENTHESIS);
    get_next_token(lexer);
    require_token(lexer, TOKEN_OPEN_BRACE);

    if (parsing_errors(lexer)) {
        return 0;
    }

    result->block = *parse_block(lexer, &result->block);

    return result;
}

internal AST_STATEMENT_TYPE get_statement_type(Lexer *lexer) {
    AST_STATEMENT_TYPE result = AST_STATEMENT_NONE;

    Lexer_savepoint savepoint = create_savepoint(lexer);
    Token t = lexer->current_token;

    switch (t.type) {
        case TOKEN_OPEN_BRACE: {
            result = AST_STATEMENT_BLOCK;
        } break;
        case TOKEN_IF: {
            result = AST_STATEMENT_IF;
        } break;
        case TOKEN_U32: {
            result = AST_STATEMENT_DECLARATION;
        } break;
        case TOKEN_VARIABLE: {
            result = AST_STATEMENT_EXPRESSION;
        } break;
    }

    rollback_lexer(savepoint);

    return result;
}

internal Ast_statement *parse_statement(Lexer *lexer, Ast_statement *result) {
    if (!result) {
        result = new_ast_statement(0);
    }

    Token t = lexer->current_token;
    AST_STATEMENT_TYPE type = get_statement_type(lexer);
    result->type = type;

    switch (result->type) {
        case AST_STATEMENT_BLOCK: {
            result->block_statement = parse_block(lexer);
        } break;
        case AST_STATEMENT_IF: {
            result->if_statement = parse_if(lexer);
        } break;
        case AST_STATEMENT_EXPRESSION: {
            result->expression_statement = parse_expression(lexer);
        } break;
        case AST_STATEMENT_DECLARATION: {
            result->expression_statement = parse_declaration(lexer);
        } break;
    }

    return result;
}

internal Ast *parse_program(Lexer *lexer) {
    // Ast *result = parse_block(lexer);
    // return result;
}

internal Ast *test_parser(Lexer *lexer) {
    Ast *result = {};

    Token token = lexer->current_token;

    while (!lexer_finished(lexer)) {
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
    char *program_block = R"PROGRAM(
    {
        u32 a = 1;
        u32 b = 2;
        {
            u32 c = a + b;
        }
    }
    )PROGRAM";

    char *program_1 = R"PROGRAM(
        u32 a = 1;
    )PROGRAM";

    char *program_0 = R"PROGRAM(
        1+2+3*2+4*4;
    )PROGRAM";

    char *program_if = R"PROGRAM(
        if (1) {
            u32 a = 0;
        }
    )PROGRAM";

    
    Lexer lexer = create_lexer(program_block);
    get_next_token(&lexer);
    Lexer_savepoint lexer_savepoint = create_savepoint(&lexer);

    debug(lexer.source);
    debug(lexer.index);
    debug(lexer.current_line);

    // Ast_block *ast = parse_block(&lexer, 0);
    Ast_statement *ast = parse_statement(&lexer, 0);
    print(ast);
    rollback_lexer(lexer_savepoint);
    test_parser(&lexer);

    BOLD(ITALIC(UNDERLINE(GREEN("\n\n\nfinished correctly\n"))));

    return 0;
}
