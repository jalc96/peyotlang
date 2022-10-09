//
// EXPRESSIONS
//
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

internal Ast_expression *parse_binary_expression(Lexer *lexer, Ast_expression *result=0) {
    if (!result) {
        result = (Ast_expression *)malloc(sizeof(Ast_expression));
    }

    result = parse_expression(lexer, result);

    Token token = lexer->current_token;

    if (token.type == TOKEN_ASSIGNMENT) {
        Ast_expression *operator_tree = (Ast_expression *)malloc(sizeof(Ast_expression));
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_expression(lexer, 0);
        result = operator_tree;
    }

    return result;
}


internal Ast_expression *DEPRECATED_parse_basic_token(Token token, Ast_expression *result) {
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

//
// DECLARATIONS
//

internal AST_DECLARATION_TYPE get_declaration_type(Lexer *lexer) {
    AST_DECLARATION_TYPE result = AST_DECLARATION_NONE;
    Lexer_savepoint savepoint = create_savepoint(lexer);

    Token first = lexer->current_token;
    result = AST_DECLARATION_VARIABLE;

    rollback_lexer(savepoint);
    return result;
}

internal Ast_declaration *parse_declaration(Lexer *lexer, Ast_declaration *result=0) {
    if (!result) {
        result = (Ast_declaration *)malloc(sizeof(Ast_expression));
    }

    AST_DECLARATION_TYPE declaration_type = get_declaration_type(lexer);
    result->type = declaration_type;

    if (declaration_type == AST_DECLARATION_VARIABLE) {
        get_next_token(lexer);
        result->variable = parse_binary_expression(lexer, 0);
        Token semicolon = lexer->current_token;
        assert(semicolon.type == TOKEN_SEMICOLON, "now this is an assert next should be an error with check or something");
        // Consume the semicolon to start fresh
        get_next_token(lexer);
    } else {
        invalid_code_path;
    }

    return result;
}

//
// BLOCKS
//

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
    if (it->current->statement_count >= AST_STATEMENTS_PER_BLOCK_LINK) {
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

//
// IF
//

internal Ast_if *parse_if(Lexer *lexer, Ast_if *result=0) {
    if (!result) {
        result = new_ast_if(0);
    }

    require_token(lexer, TOKEN_IF, "parse_if");
    get_next_token(lexer);
    require_token(lexer, TOKEN_OPEN_PARENTHESIS, "parse_if");
    get_next_token(lexer);

    if (parsing_errors(lexer)) {
        // TODO: handle errors this way and have a buffer in the Lexer struct to store the error message
        return 0;
    }

    result->condition = *parse_binary_expression(lexer, &result->condition);
    require_token(lexer, TOKEN_CLOSE_PARENTHESIS, "parse_if");
    get_next_token(lexer);
    require_token(lexer, TOKEN_OPEN_BRACE, "parse_if");

    if (parsing_errors(lexer)) {
        return 0;
    }

    result->block = *parse_block(lexer, &result->block);

    return result;
}

//
// LOOPS
//

internal Ast_loop *parse_loop(Lexer *lexer, Ast_loop *result=0) {
    if (!result) {
        result = new_ast_loop(0);
    }

    Token loop = lexer->current_token;
    get_next_token(lexer);
    require_token(lexer, TOKEN_OPEN_PARENTHESIS, "parse_loop");

    if (loop.type == TOKEN_FOR) {
        result->pre = parse_declaration(lexer);
        result->condition = parse_binary_expression(lexer);
        require_token(lexer, TOKEN_SEMICOLON, "parse_loop");
        result->post = parse_binary_expression(lexer);

        require_token(lexer, TOKEN_CLOSE_PARENTHESIS, "parse_loop");
        result->block = parse_block(lexer);
    } else if (loop.type == TOKEN_WHILE) {
        result->pre = 0;
        result->condition = parse_binary_expression(lexer);
        result->post = 0;

        require_token(lexer, TOKEN_CLOSE_PARENTHESIS, "parse_loop");
        result->block = parse_block(lexer);
    }

    return result;
}

//
// STATEMENTS
//

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
        case TOKEN_LITERAL_U32: {
            result = AST_STATEMENT_EXPRESSION;
        } break;
        case TOKEN_VARIABLE: {
            result = AST_STATEMENT_EXPRESSION;
        } break;
        case TOKEN_WHILE:
        case TOKEN_FOR: {
            result = AST_STATEMENT_LOOP;
        } break;
        invalid_default_case_msg("get_statement_type unhandled type");
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
            result->expression_statement = parse_binary_expression(lexer);
            Token semicolon = lexer->current_token;
            assert(semicolon.type == TOKEN_SEMICOLON, "when parsing an expression statement, this must be ended with a semicolon ';'");
            get_next_token(lexer);
        } break;
        case AST_STATEMENT_DECLARATION: {
            result->declaration_statement = parse_declaration(lexer);
        } break;
        case AST_STATEMENT_LOOP: {
            result->loop_statement = parse_loop(lexer);
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
