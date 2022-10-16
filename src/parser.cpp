//
// EXPRESSIONS
//
/*
expression ::= term ([+-] term)*
term ::= unary_expression ([/*%] unary_expression)
unary_expression ::= [+-]* factor
factor ::= (expression) | var_name | function_call | basic_type
*/

internal void leaf(Ast_expression *ast, AST_EXPRESSION_TYPE type) {
    ast->type = type;
    ast->left = 0;
    ast->right = 0;
}

internal Ast_expression *parse_factor(Lexer *lexer, Ast_expression *result) {
    if (!result) {
        result = push_struct(lexer->allocator, Ast_expression);
    }

    Token token = lexer->current_token;

    switch (token.type){
        case TOKEN_NAME: {
            leaf(result, AST_EXPRESSION_NAME);
            result->name = token.name;
        } break;

        case TOKEN_LITERAL_U32: {
            leaf(result, AST_EXPRESSION_LITERAL_U32);
            result->u64_value = token.u64_value;
        } break;

        invalid_default_case_msg("create a error in parse_factor()");
    }

    get_next_token(lexer);

    return result;
}

internal Ast_expression *parse_unary_expression(Lexer *lexer, Ast_expression *result) {
    if (!result) {
        result = push_struct(lexer->allocator, Ast_expression);
    }

    Token token = lexer->current_token;
    s32 sign = 1;

    while (is_add_operator(token.type)) {
        if (token.type == TOKEN_SUB) {
            sign *= -1;
        }

        token = get_next_token(lexer);
    }

    if (sign < 0) {
        result->type = AST_EXPRESSION_UNARY_SUB;
        result->right = parse_factor(lexer, 0);
    } else {
        result = parse_factor(lexer, result);
    }

    return result;
}

internal Ast_expression *parse_term(Lexer *lexer, Ast_expression *result) {
    result = parse_unary_expression(lexer, result);

    Token token = lexer->current_token;

    while (is_mul_operator(lexer->current_token.type)) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_unary_expression(lexer, 0);
        result = operator_tree;
        
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_expression(Lexer *lexer, Ast_expression *result) {
    result = parse_term(lexer, result);

    Token token = lexer->current_token;

    while (is_add_operator(token.type)) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_term(lexer, 0);
        result = operator_tree;
        token = lexer->current_token;
    }

    return result;
}

internal bool is_inequality_operator(PEYOT_TOKEN_TYPE type) {
    bool result = (
           type == TOKEN_GREATER_THAN
        || type == TOKEN_GREATER_THAN_OR_EQUALS
        || type == TOKEN_LESS_THAN
        || type == TOKEN_LESS_THAN_OR_EQUALS
    );
    return result;
}

internal Ast_expression *parse_relational_inequality_expression(Lexer *lexer, Ast_expression *result) {
    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_expression(lexer, result);
    Token token = lexer->current_token;

    while (is_inequality_operator(token.type)) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_expression(lexer, 0);
        result = operator_tree;
        token = lexer->current_token;
    }

    return result;
}

internal bool is_equality_operator(PEYOT_TOKEN_TYPE type) {
    bool result = (
           type == TOKEN_BINARY_EQUALS
        || type == TOKEN_NOT_EQUALS
    );
    return result;
}

internal Ast_expression *parse_relational_equality_expression(Lexer *lexer, Ast_expression *result) {
    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_relational_inequality_expression(lexer, result);
    Token token = lexer->current_token;

    while (is_equality_operator(token.type)) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_relational_inequality_expression(lexer, 0);
        result = operator_tree;
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_and_expression(Lexer *lexer, Ast_expression *result) {
    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_relational_equality_expression(lexer, result);
    Token token = lexer->current_token;

    while (token.type == TOKEN_LOGICAL_AND) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_relational_equality_expression(lexer, 0);
        result = operator_tree;
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_or_expression(Lexer *lexer, Ast_expression *result) {
    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_and_expression(lexer, result);
    Token token = lexer->current_token;

    while (token.type == TOKEN_LOGICAL_OR) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_and_expression(lexer, 0);
        result = operator_tree;
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_binary_expression(Lexer *lexer, Ast_expression *result) {
    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_or_expression(lexer, result);

    Token token = lexer->current_token;

    if (token.type == TOKEN_ASSIGNMENT) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->left = result;
        get_next_token(lexer);
        operator_tree->right = parse_or_expression(lexer, 0);
        result = operator_tree;
    }

    return result;
}


internal Ast_expression *DEPRECATED_parse_basic_token(Token token, Ast_expression *result) {
    if (!result) {
        // result = new_ast_expression(lexer->allocator);
    }

    switch (token.type) {
        case TOKEN_NAME: {
            result->type = AST_EXPRESSION_NAME;
            result->name = token.name;
        } break;

        case TOKEN_LITERAL_U32: {
            result->type = AST_EXPRESSION_LITERAL_U32;
            result->u64_value = token.u64_value;
        } break;

        invalid_default_case_msg("unable to parse basic token");
    }

    return result;
}

//
// DECLARATIONS
//

internal Type_spec_table *type_table(Lexer *lexer) {
    return lexer->parser->type_table;
}

internal bool is_compound(PEYOT_TOKEN_TYPE type) {
    bool result = (
           type == TOKEN_STRUCT
        || type == TOKEN_UNION
    );
    return result;
}

internal AST_DECLARATION_TYPE get_declaration_type(Lexer *lexer) {
    AST_DECLARATION_TYPE result = AST_DECLARATION_NONE;
    Lexer_savepoint savepoint = create_savepoint(lexer);

    Token t1 = lexer->current_token;
    Token t2 = get_next_token(lexer);
    Token t3 = get_next_token(lexer);
    Token t4 = get_next_token(lexer);

    bool is_function = (
           t1.type == TOKEN_NAME
        && t2.type == TOKEN_COLON
        && t3.type == TOKEN_COLON
        && t4.type == TOKEN_OPEN_PARENTHESIS
    );

    if (is_type(type_table(lexer), t1)) {
        result = AST_DECLARATION_VARIABLE;
    } else if (is_compound(t1.type)) {
        result = AST_DECLARATION_COMPOUND;
    } else if (t1.type == TOKEN_ENUM) {
        result = AST_DECLARATION_ENUM;
    } else if (is_function) {
        result = AST_DECLARATION_FUNCTION;
    }

    rollback_lexer(savepoint);
    return result;
}

internal u32 get_param_count(Lexer *lexer) {
    u32 result = 0;

    Lexer_savepoint savepoint = create_savepoint(lexer);

    Token t = lexer->current_token;

    while ((t.type != TOKEN_CLOSE_PARENTHESIS) && (t.type != TOKEN_EOF)) {
        if (is_type(type_table(lexer), t)) {
            result++;
        }

        t = get_next_token(lexer);
    }

    rollback_lexer(savepoint);

    return result;
}

struct Compound_parsing_result {
    str name;
    Compound *compound;
};

internal Compound_parsing_result parse_compound(Lexer *lexer, bool annonymous=false) {
    Compound_parsing_result result;
    result.compound = new_compound(lexer->allocator);
    result.compound->member_count = 0;

    Token t = lexer->current_token;
    assert(is_compound(t.type), "missing keyword for compound definition");
    result.compound->compound_type = token_type_to_compound_type(lexer->current_token.type);
    get_next_token(lexer);

    if (!annonymous) {
        result.name = lexer->current_token.name;
        get_next_token(lexer);
    }

    require_token(lexer, TOKEN_COLON, "in parse_compound");
    require_token(lexer, TOKEN_COLON, "in parse_compound");
    require_token(lexer, TOKEN_OPEN_BRACE, "in parse_compound");
        t = lexer->current_token;
        Member **last = &result.compound->members;

        while ((t.type != TOKEN_CLOSE_BRACE) && (t.type != TOKEN_EOF)) {
            Member *new_member = push_struct(lexer->allocator, Member);
            result.compound->member_count++;

            if (is_compound(t.type)) {
                new_member->member_type = MEMBER_COMPOUND;
                Compound_parsing_result sub = parse_compound(lexer, true);
                new_member->sub_compound = sub.compound;
            } else {
                new_member->member_type = MEMBER_SIMPLE;
                new_member->type = get_type(type_table(lexer), t.name);
                t = get_next_token(lexer);
                new_member->name = t.name;
                get_next_token(lexer);
                require_token(lexer, TOKEN_SEMICOLON, "in parse_declaration");
            }

            new_member->next = 0;

            *last = new_member;
            last = &new_member->next;

            t = lexer->current_token;
        }
    require_token(lexer, TOKEN_CLOSE_BRACE, "in parse_compound");
    require_token(lexer, TOKEN_SEMICOLON, "in parse_compound");

    return result;
}

internal u32 get_enum_items_count(Lexer *lexer) {
    u32 result = 0;

    Lexer_savepoint savepoint = create_savepoint(lexer);

    Token t = lexer->current_token;

    while ((t.type != TOKEN_CLOSE_BRACE) && (t.type != TOKEN_EOF)) {
        if (t.type == TOKEN_COMMA) {
            result++;
        }

        t = get_next_token(lexer);
    }

    rollback_lexer(savepoint);

    return result;
}

internal Ast_block *parse_block(Lexer *lexer, Ast_block *result);

internal Ast_declaration *parse_declaration(Lexer *lexer, Ast_declaration *result) {
    if (!result) {
        result = push_struct(lexer->allocator, Ast_declaration);
    }

    AST_DECLARATION_TYPE declaration_type = get_declaration_type(lexer);
    result->type = declaration_type;

    if (declaration_type == AST_DECLARATION_VARIABLE) {
        result->variable_type = get_type(lexer->parser->type_table, lexer->current_token.name);

        if (!result->variable_type) {
            // TODO: handle out of order declaration
        }

        get_next_token(lexer);
        result->expression = parse_binary_expression(lexer, 0);
        Token semicolon = lexer->current_token;
        assert(semicolon.type == TOKEN_SEMICOLON, "MISSING A SEMICOLON IN EXPRESSION DECLARATION now this is an assert next should be an error with check or something");
        // Consume the semicolon to start fresh
        get_next_token(lexer);
    } else if (declaration_type == AST_DECLARATION_FUNCTION) {
        result->name = lexer->current_token.name;
        get_next_token(lexer);

        require_token(lexer, TOKEN_COLON, "in parse_declaration");
        require_token(lexer, TOKEN_COLON, "in parse_declaration");

        require_token(lexer, TOKEN_OPEN_PARENTHESIS, "in parse_declaration");
            result->param_count = get_param_count(lexer);
            result->params = push_array(lexer->allocator, Parameter, result->param_count);

            sfor_count(result->params, result->param_count) {
                Token t = lexer->current_token;
                it->type = get_type(type_table(lexer), t.name);
                t = get_next_token(lexer);
                it->name = t.name;
                get_next_token(lexer);

                // +1 because arrays start at 0 and there are (param_count - 1) number of commas so we have to check (param_count - 1) times
                if ((i + 1) < result->param_count) {
                    require_token(lexer, TOKEN_COMMA, "in parse_declaration");
                }
            }
        require_token(lexer, TOKEN_CLOSE_PARENTHESIS, "in parse_declaration");

        require_token(lexer, TOKEN_RETURN_ARROW, "in parse_declaration");

        result->return_type = get_type(lexer->parser->type_table, lexer->current_token.name);

        if (!result->variable_type) {
            // TODO: handle out of order declaration
        }

        get_next_token(lexer);

        result->block = parse_block(lexer, 0);
    } else if (declaration_type == AST_DECLARATION_COMPOUND) {
        assert(is_compound(lexer->current_token.type), "in declaration parsing the first keyword must be struct or union");
        Compound_parsing_result cpr = parse_compound(lexer);
        result->compound = cpr.compound;
        result->name = cpr.name;
        push_type(type_table(lexer), result->name, TYPE_SPEC_NAME);
    } else if (declaration_type == AST_DECLARATION_ENUM) {
        require_token(lexer, TOKEN_ENUM, "in parse_declaration");
        result->name = lexer->current_token.name;
        result->enum_type = push_type(type_table(lexer), result->name, TYPE_SPEC_NAME);
        get_next_token(lexer);

        require_token(lexer, TOKEN_COLON, "in parse_declaration");
        require_token(lexer, TOKEN_COLON, "in parse_declaration");

        require_token(lexer, TOKEN_OPEN_BRACE, "in parse_declaration");
            result->item_count = get_enum_items_count(lexer);
            result->items = push_array(lexer->allocator, Enum_item, result->item_count);
            Token t;

            sfor_count(result->items, result->item_count) {
                t = lexer->current_token;
                it->name = t.name;
                t = get_next_token(lexer);

                if (t.type == TOKEN_ASSIGNMENT) {
                    get_next_token(lexer);
                    it->value = parse_expression(lexer, 0);
                }

                require_token(lexer, TOKEN_COMMA, "in parse_declaration");
            }

        require_token(lexer, TOKEN_CLOSE_BRACE, "in parse_declaration");
        require_token(lexer, TOKEN_SEMICOLON, "in parse_declaration");
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
    Memory_pool *allocator;
    Ast_block *current;
};

internal Ast_block_creation_iterator iterate(Ast_block *block, Memory_pool *allocator) {
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

internal Ast_block *parse_block(Lexer *lexer, Ast_block *result) {
    if (!result) {
        result = new_ast_block(lexer->allocator);
    }

    assert(lexer->current_token.type == TOKEN_OPEN_BRACE, "when parsing a block, the current token must be an open brace '{'");
    get_next_token(lexer);

    Block_parser parser;
    parser.lexer = lexer;
    parser.finished = false;
    Ast_block_creation_iterator it = iterate(result, lexer->allocator);

    while (parsing_block(&parser)) {
        Ast_statement *e = advance(&it);
        // the weird bug is in parse statement
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

struct If_creation_iterator {
    Lexer *lexer;
    Ast_if *result;
    If **last;
    bool last_block;
    bool first_if;
};

internal If_creation_iterator iterate(Lexer *lexer, Ast_if *_result) {
    If_creation_iterator result;

    result.lexer = lexer;
    result.result = _result;
    result.last = &_result->ifs;
    result.last_block = false;
    result.first_if = true;

    return result;
}

internal bool parsing_if(If_creation_iterator it) {
    return !it.last_block;
}

internal void parse_if_else(If_creation_iterator *it) {
    Lexer *lexer = it->lexer;

    if (it->first_if) {
        If *new_if = push_struct(lexer->allocator, If);

        require_token(lexer, TOKEN_IF, "parse_if");
        it->first_if = false;

        require_token(lexer, TOKEN_OPEN_PARENTHESIS, "parse_if");
            parse_binary_expression(lexer, &new_if->condition);
        require_token(lexer, TOKEN_CLOSE_PARENTHESIS, "parse_if");

        parse_block(lexer, &new_if->block);

        *it->last = new_if;
        it->last = &new_if->next;
    } else {
        Token t = lexer->current_token;

        if (t.type == TOKEN_ELSE) {

            t = get_next_token(lexer);

            if (t.type == TOKEN_IF) {
                If *new_if = push_struct(lexer->allocator, If);

                get_next_token(lexer);

                require_token(lexer, TOKEN_OPEN_PARENTHESIS, "parse_if");
                    parse_binary_expression(lexer, &new_if->condition);
                require_token(lexer, TOKEN_CLOSE_PARENTHESIS, "parse_if");

                parse_block(lexer, &new_if->block);

                *it->last = new_if;
                it->last = &new_if->next;
            } else {
                it->last_block = true;
                it->result->else_block = parse_block(lexer, 0);
            }
        } else {
            it->last_block = true;
        }
    }
}

internal Ast_if *parse_if(Lexer *lexer, Ast_if *result) {
    if (!result) {
        result = push_struct(lexer->allocator, Ast_if);
    }

    If_creation_iterator it = iterate(lexer, result);

    while (parsing_if(it)) {
        parse_if_else(&it);
    }

    return result;
}

//
// LOOPS
//

internal Ast_loop *parse_loop(Lexer *lexer, Ast_loop *result=0) {
    if (!result) {
        result = new_ast_loop(lexer->allocator);
    }

    Token loop = lexer->current_token;
    get_next_token(lexer);
    require_token(lexer, TOKEN_OPEN_PARENTHESIS, "parse_loop");

    if (loop.type == TOKEN_FOR) {
        // the weird bug seems to be here
        result->pre = parse_declaration(lexer, 0);
        result->condition = parse_binary_expression(lexer, 0);
        require_token(lexer, TOKEN_SEMICOLON, "parse_loop");
        result->post = parse_binary_expression(lexer, 0);

        require_token(lexer, TOKEN_CLOSE_PARENTHESIS, "parse_loop");
        result->block = parse_block(lexer, 0);
    } else if (loop.type == TOKEN_WHILE) {
        result->pre = 0;
        result->condition = parse_binary_expression(lexer, 0);
        result->post = 0;

        require_token(lexer, TOKEN_CLOSE_PARENTHESIS, "parse_loop");
        result->block = parse_block(lexer, 0);
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
        case TOKEN_NAME: {
            // TODO: do something with the custom types here, the token will be TOKEN_NAME have an if (is_type(token)) that checks in the types hash table if that type exists
            result = AST_STATEMENT_EXPRESSION;
        } break;
        case TOKEN_WHILE:
        case TOKEN_FOR: {
            result = AST_STATEMENT_LOOP;
        } break;
        case TOKEN_BREAK: {
            result = AST_STATEMENT_BREAK;
        } break;
        case TOKEN_CONTINUE: {
            result = AST_STATEMENT_CONTINUE;
        } break;
        case TOKEN_RETURN: {
            result = AST_STATEMENT_RETURN;
        } break;
        invalid_default_case_msg("get_statement_type unhandled type");
    }

    rollback_lexer(savepoint);

    return result;
}

internal Ast_statement *parse_statement(Lexer *lexer, Ast_statement *result) {
    if (!result) {
        result = new_ast_statement(lexer->allocator);
    }

    Token t = lexer->current_token;
    AST_STATEMENT_TYPE type = get_statement_type(lexer);
    result->type = type;

    switch (result->type) {
        case AST_STATEMENT_BLOCK: {
            result->block_statement = parse_block(lexer, 0);
        } break;
        case AST_STATEMENT_IF: {
            result->if_statement = parse_if(lexer, 0);
        } break;
        case AST_STATEMENT_EXPRESSION: {
            result->expression_statement = parse_binary_expression(lexer, 0);
            Token semicolon = lexer->current_token;
            assert(semicolon.type == TOKEN_SEMICOLON, "when parsing an expression statement, this must be ended with a semicolon ';'");
            get_next_token(lexer);
        } break;
        case AST_STATEMENT_DECLARATION: {
            result->declaration_statement = parse_declaration(lexer, 0);
        } break;
        case AST_STATEMENT_LOOP: {
            result->loop_statement = parse_loop(lexer);
        } break;
        case AST_STATEMENT_BREAK:{
            require_token(lexer, TOKEN_BREAK, "parsing break statement");
            require_token(lexer, TOKEN_SEMICOLON, "parsing break statement");
        } break;
        case AST_STATEMENT_CONTINUE: {
            require_token(lexer, TOKEN_CONTINUE, "parsing continue statement");
            require_token(lexer, TOKEN_SEMICOLON, "parsing continue statement");
        } break;
        case AST_STATEMENT_RETURN: {
            require_token(lexer, TOKEN_RETURN, "parsing return statement");
            require_token(lexer, TOKEN_SEMICOLON, "parsing return statement");
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
            case TOKEN_NAME: {
                printf("<%.*s>", token.name.count, token.name.buffer);
                assert(token.name.count == (token.cf - token.c0), "the char offset selection in get_next_token is wrong");
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
