//
// ERROR REPORTING
//

#define log_error(eb, format, ...) eb->head += stbsp_snprintf(get_buffer(eb), eb->size, format, __VA_ARGS__)

#define FIRST_LINE_IN_CODE_EDITORS_STARTS_AT_1 1

struct Syntax_error_positions {
    Src_position start;
    Src_position last_correct;
};

#define skip_new_line 1

#define SYNTAX_CHECK_FUNCTION(name) bool name(PEYOT_TOKEN_TYPE desired_token_type, Token token, Type_spec_table *type_table)
typedef SYNTAX_CHECK_FUNCTION(Check_function);

internal SYNTAX_CHECK_FUNCTION(token_check) {
    bool result = token.type == desired_token_type;
    return result;
}

internal SYNTAX_CHECK_FUNCTION(type_check) {
    bool result = is_type(type_table, token);
    return result;
}

internal SYNTAX_CHECK_FUNCTION(always_false) {
    bool result = false;
    return result;
}

internal void require_token_and_report_syntax_error(Lexer *lexer, Check_function check_function, PEYOT_TOKEN_TYPE desired_token_type, Syntax_error_positions positions, char *msg, bool print_wrong_token_in_red) {
    if (!check_function(desired_token_type, lexer->current_token, lexer->parser->type_table)) {
        lexer->parser->parsing_errors = true;
        u32 l0 = positions.start.c0;
        u32 lf = positions.last_correct.cf;
        Src_position error_at_p = positions.last_correct;
        Src_position last_valid_p = positions.last_correct;

        l0 = find_first_from_position(lexer->source, l0, '\n', true);
        str block = slice(lexer->source, l0 + skip_new_line, lf);
        Str_buffer *eb = &lexer->parser->error_buffer;
        log_error(eb, STATIC_RED("SYNTAX ERROR"), 0);
        log_error(eb, ": %s\n\n", msg);

        u32 line_count = positions.start.line;
        u32 non_used_chars = l0;

        for (Split_iterator it = split(block, STATIC_STR("\n")); valid(&it); it = next(it)) {
            char scratch[256];
            u32 line_show_count = stbsp_snprintf(scratch, 256, "    %d:", line_count);
            str line = it.sub_str;
            log_error(eb, "%s%.*s", scratch, line.count, line.buffer);

            if (!print_wrong_token_in_red) {
                log_error(eb, "\n");
            }

            if (line_count == error_at_p.line) {
                // this has to be in line relative space, not global source space as it is now
                if (print_wrong_token_in_red) {
                    log_error(eb, STATIC_RED("%s\n"), to_symbol(lexer->current_token.type, &lexer->current_token));
                }
                u32 offset_to_the_last_valid_char = last_valid_p.cf - non_used_chars + line_show_count - 1;

                count_for(offset_to_the_last_valid_char) {
                    log_error(eb, " ");
                }

                log_error(eb, "^");
            }

            non_used_chars += line.count;
            #define LINE_COUNT_ABOVE_DOESNT_COUNT_THE_NEW_LINE_CHAR 1
            non_used_chars += LINE_COUNT_ABOVE_DOESNT_COUNT_THE_NEW_LINE_CHAR;
            line_count++;
        }
    }

    get_next_token(lexer);
}

internal void report_parsing_errors(Lexer *lexer) {
    Str_buffer *eb = &lexer->parser->error_buffer;

    printf("%.*s\n", eb->head, eb->buffer);
}


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
    ast->binary.left = 0;
    ast->binary.right = 0;
}

struct Call_parameter_list_creator {
    Call_parameter *parameter;
    Call_parameter **last;
    u32 parameter_count;
    Lexer *lexer;
    bool finished;
    // TODO: check this positions
    Syntax_error_positions *positions;
};

internal Call_parameter_list_creator iterate_parameter_list(Lexer *lexer, Syntax_error_positions *positions) {
    Call_parameter_list_creator result;

    result.parameter = 0;
    result.last = 0;
    result.parameter_count = 0;
    result.lexer = lexer;
    result.positions = positions;
    PEYOT_TOKEN_TYPE t = lexer->current_token.type;
    // TODO: have a better check for this
    result.finished = (
           (t == TOKEN_CLOSE_PARENTHESIS) 
        || (
               (t != TOKEN_LITERAL_U32)
            && (t != TOKEN_NAME)
           )
    );

    return result;
}

internal bool parsing(Call_parameter_list_creator it) {
    return !it.finished;
}

internal Ast_expression *parse_or_expression(Lexer *lexer, Ast_expression *result);
internal Ast_expression *parse_binary_expression(Lexer *lexer, Ast_expression *result);

internal void parse_parameter(Call_parameter_list_creator *it) {
    assert(it->last, "last pointer never can be zero, it must be initialized outside the iteration function");

    Token token = it->lexer->current_token;
    it->parameter_count++;
    Call_parameter *new_parameter = push_struct(it->lexer->allocator, Call_parameter);
    new_parameter->parameter = parse_or_expression(it->lexer, 0);
    *it->last = new_parameter;
    it->last = &new_parameter->next;

    token = it->lexer->current_token;
    it->finished = it->lexer->parser->parsing_errors;
    if (it->finished) return;

    if (token.type == TOKEN_CLOSE_PARENTHESIS) {
        it->finished = true;
    } else if (token.type == TOKEN_COMMA) {
        // TODO: where do we want to check for this error, here or in the parse_factor function??
        require_token_and_report_syntax_error(it->lexer, token_check, TOKEN_COMMA, *it->positions, "comma expected while parsing parameter list in function call", false);
    } else {
        it->finished = true;
    }

    it->positions->last_correct = it->lexer->current_token.src_p;
}

internal Ast_expression *parse_factor(Lexer *lexer, Ast_expression *result) {
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = push_struct(lexer->allocator, Ast_expression);
    }

    Token token = lexer->current_token;
    result->src_p = token.src_p;
    Syntax_error_positions positions;
    positions.start = token.src_p;

    switch (token.type){
        case TOKEN_NAME: {
            leaf(result, AST_EXPRESSION_NAME);
            result->name = token.name;

            Lexer_savepoint sp = create_savepoint(lexer);
            token = get_next_token(lexer);

            if (token.type == TOKEN_DOT) {
                result->type = AST_EXPRESSION_MEMBER;

                positions.last_correct = token.src_p;
                token = get_next_token(lexer);
                require_token_and_report_syntax_error(lexer, token_check, TOKEN_NAME, positions, "struct members must be accessed by a name identifier", true);
                if (lexer->parser->parsing_errors) return 0;

                result->binary.right = push_struct(lexer->allocator, Ast_expression);
                leaf(result->binary.right, AST_EXPRESSION_NAME);
                result->binary.right->name = token.name;
            } else if (token.type == TOKEN_OPEN_PARENTHESIS) {
                result->type = AST_EXPRESSION_FUNCTION_CALL;

                get_next_token(lexer);
                Call_parameter_list_creator it = iterate_parameter_list(lexer, &positions);
                it.last = &it.parameter;

                while (parsing(it)) {
                    parse_parameter(&it);
                }

                if (lexer->parser->parsing_errors) return 0;

                Token pt = lexer->previous_token;
                positions.last_correct = pt.src_p;
                require_token_and_report_syntax_error(lexer, token_check, TOKEN_CLOSE_PARENTHESIS, positions, "missing close parenthesis ')' in a function call", false);
                if (lexer->parser->parsing_errors) return 0;

                result->function_call.parameter_count = it.parameter_count;
                result->function_call.parameter = it.parameter;
            } else {
                // JA(2022-10-25): i think this rollback is not necesarry anymore because we have to get the next token
                // rollback_lexer(sp);
            }
        } break;

        case TOKEN_LITERAL_U32: {
            leaf(result, AST_EXPRESSION_LITERAL_U32);
            result->u64_value = token.u64_value;
            get_next_token(lexer);
        } break;

        case TOKEN_OPEN_PARENTHESIS: {
            get_next_token(lexer);
            result = parse_binary_expression(lexer, result);

            Token pt = lexer->previous_token;
            positions.last_correct = pt.src_p;
            require_token_and_report_syntax_error(lexer, token_check, TOKEN_CLOSE_PARENTHESIS, positions, "missing close parenthesis ')' for expression", false);
            if (lexer->parser->parsing_errors) return 0;

            result->u64_value = token.u64_value;
        } break;

        default: {
            Syntax_error_positions error_p;
            error_p.start = lexer->previous_token.src_p;
            error_p.last_correct = lexer->previous_token.src_p;

            char msg[256];
            stbsp_snprintf(msg, 256, "unexpected token found '%s'", to_symbol(token.type));
            require_token_and_report_syntax_error(lexer, always_false, TOKEN_NULL, error_p, msg, true);
        }

        // invalid_default_case_msg("create a error in parse_factor()");
    }

    return result;
}

internal Ast_expression *parse_unary_expression(Lexer *lexer, Ast_expression *result) {
    if (lexer->parser->parsing_errors) return 0;

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
        result->src_p = token.src_p;
        result->binary.right = parse_factor(lexer, 0);
    } else {
        result = parse_factor(lexer, result);
    }

    return result;
}

internal Ast_expression *parse_term(Lexer *lexer, Ast_expression *result) {
    if (lexer->parser->parsing_errors) return 0;

    result = parse_unary_expression(lexer, result);

    Token token = lexer->current_token;

    while (is_mul_operator(lexer->current_token.type)) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->src_p = token.src_p;
        operator_tree->binary.left = result;
        get_next_token(lexer);
        operator_tree->binary.right = parse_unary_expression(lexer, 0);
        result = operator_tree;
        
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_expression(Lexer *lexer, Ast_expression *result) {
    if (lexer->parser->parsing_errors) return 0;

    result = parse_term(lexer, result);

    Token token = lexer->current_token;

    while (is_add_operator(token.type)) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->src_p = token.src_p;
        operator_tree->binary.left = result;
        get_next_token(lexer);
        operator_tree->binary.right = parse_term(lexer, 0);
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
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_expression(lexer, result);
    Token token = lexer->current_token;

    while (is_inequality_operator(token.type)) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->src_p = token.src_p;
        operator_tree->binary.left = result;
        get_next_token(lexer);
        operator_tree->binary.right = parse_expression(lexer, 0);
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
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_relational_inequality_expression(lexer, result);
    Token token = lexer->current_token;

    while (is_equality_operator(token.type)) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->src_p = token.src_p;
        operator_tree->binary.left = result;
        get_next_token(lexer);
        operator_tree->binary.right = parse_relational_inequality_expression(lexer, 0);
        result = operator_tree;
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_and_expression(Lexer *lexer, Ast_expression *result) {
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_relational_equality_expression(lexer, result);
    Token token = lexer->current_token;

    while (token.type == TOKEN_LOGICAL_AND) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->src_p = token.src_p;
        operator_tree->binary.left = result;
        get_next_token(lexer);
        operator_tree->binary.right = parse_relational_equality_expression(lexer, 0);
        result = operator_tree;
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_or_expression(Lexer *lexer, Ast_expression *result) {
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_and_expression(lexer, result);
    Token token = lexer->current_token;

    while (token.type == TOKEN_LOGICAL_OR) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->src_p = token.src_p;
        operator_tree->binary.left = result;
        get_next_token(lexer);
        operator_tree->binary.right = parse_and_expression(lexer, 0);
        result = operator_tree;
        token = lexer->current_token;
    }

    return result;
}

internal Ast_expression *parse_binary_expression(Lexer *lexer, Ast_expression *result) {
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = new_ast_expression(lexer->allocator);
    }

    result = parse_or_expression(lexer, result);

    Token token = lexer->current_token;

    if (token.type == TOKEN_ASSIGNMENT) {
        Ast_expression *operator_tree = new_ast_expression(lexer->allocator);
        operator_tree->type = token_type_to_operation(token.type);
        operator_tree->src_p = token.src_p;
        operator_tree->binary.left = result;
        get_next_token(lexer);
        operator_tree->binary.right = parse_or_expression(lexer, 0);
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
    /*
          t1   t2  t3
        <name> :: struct
        <name> :: union
        <name> :: enum
        <name> :: (...)
        <name> : <type>
    */
    AST_DECLARATION_TYPE result = AST_DECLARATION_NONE;
    Lexer_savepoint savepoint = create_savepoint(lexer);

    Token t1 = lexer->current_token;
    Token t2 = get_next_token(lexer);
    Token t3 = get_next_token(lexer);

    if (is_type(type_table(lexer), t3)) {
        result = AST_DECLARATION_VARIABLE;
    } else if (is_compound(t3.type)) {
        result = AST_DECLARATION_COMPOUND;
    } else if (t3.type == TOKEN_ENUM) {
        result = AST_DECLARATION_ENUM;
    } else if (t3.type == TOKEN_OPEN_PARENTHESIS) {
        result = AST_DECLARATION_FUNCTION;
    }

    rollback_lexer(savepoint);
    return result;
}

internal u32 get_param_count(Lexer *lexer, Syntax_error_positions positions) {
    u32 result = 0;

    Lexer_savepoint savepoint = create_savepoint(lexer);


    // while ((t.type != TOKEN_CLOSE_PARENTHESIS) && (t.type != TOKEN_EOF)) {
    while (1) {
        Src_position last_correct = lexer->current_token.src_p;
        require_token_and_report_syntax_error(lexer, token_check, TOKEN_NAME, positions, "name expected in a function parameter declaration", false);
        if (lexer->parser->parsing_errors) break;
        positions.last_correct = last_correct;

        last_correct = lexer->current_token.src_p;
        require_token_and_report_syntax_error(lexer, token_check, TOKEN_COLON, positions, "colon ':' expected in a function parameter declaration", false);
        if (lexer->parser->parsing_errors) break;
        positions.last_correct = last_correct;

        last_correct = lexer->current_token.src_p;
        require_token_and_report_syntax_error(lexer, type_check, TOKEN_NAME, positions, "type expected in a function parameter declaration", false);
        if (lexer->parser->parsing_errors) break;
        positions.last_correct = last_correct;

        bool finished = (
               (lexer->current_token.type == TOKEN_CLOSE_PARENTHESIS)
            || (lexer->current_token.type == TOKEN_EOF)
        );

        if (finished) {
            result++;
            break;
        }

        last_correct = lexer->current_token.src_p;
        require_token_and_report_syntax_error(lexer, token_check, TOKEN_COMMA, positions, "comma ',' expected in a function parameter declaration", false);
        if (lexer->parser->parsing_errors) break;
        positions.last_correct = last_correct;

        result++;
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

    require_token(lexer, TOKEN_OPEN_BRACE, "in parse_compound");
        Token t = lexer->current_token;
        Member **last = &result.compound->members;

        while ((t.type != TOKEN_CLOSE_BRACE) && (t.type != TOKEN_EOF)) {
            Member *new_member = push_struct(lexer->allocator, Member);
            result.compound->member_count++;

            if (is_compound(t.type)) {
                // TODO: now the annonymous and the names have to be handled here
                new_member->member_type = MEMBER_COMPOUND;
                new_member->src_p = t.src_p;
                Compound_parsing_result sub = parse_compound(lexer, true);
                new_member->sub_compound = sub.compound;
            } else {
                new_member->member_type = MEMBER_SIMPLE;
                new_member->name = t.name;
                new_member->src_p = t.src_p;

                t = get_next_token(lexer);
                assert(t.type == TOKEN_COLON, "token after a member name in a struct declaration must be a colon");

                t = get_next_token(lexer);
                new_member->type = get_type(type_table(lexer), t.name);

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
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = push_struct(lexer->allocator, Ast_declaration);
    }

    Syntax_error_positions positions;

    Token name = lexer->current_token;
    result->name = name.name;
    result->src_p = name.src_p;

    positions.start = result->src_p;

    AST_DECLARATION_TYPE declaration_type = get_declaration_type(lexer);
    result->type = declaration_type;

    // Consume the name
    Token declaration = get_next_token(lexer);
    assert(declaration.type == TOKEN_DECLARATION || declaration.type == TOKEN_COLON, "the token following the name of a declaration must be a declaration or a colon");
    // Consume the declaration or type in variable declaration
    Token declaration_token_type = get_next_token(lexer);
    COMPOUND_TYPE compound_type = token_type_to_compound_type(declaration_token_type.type);


    if (declaration_type == AST_DECLARATION_VARIABLE) {
        result->variable.variable_type = get_type(lexer->parser->type_table, declaration_token_type.name);

        if (!result->variable.variable_type) {
            // TODO: handle out of order declaration
        }

        // TODO: handle type inference if the declaration_token_type is an assignment
        // Consume the type
        Token after_type = get_next_token(lexer);
        result->variable.expression = 0;

        if (after_type.type == TOKEN_ASSIGNMENT) {
            get_next_token(lexer);
            result->variable.expression = parse_binary_expression(lexer, 0);
        }

        positions.last_correct = lexer->previous_token.src_p;
        // Consume the semicolon to start fresh
        require_token_and_report_syntax_error(lexer, token_check, TOKEN_SEMICOLON, positions, "Missing semicolon ';' at the end of the variable declaration", false);
    } else if (declaration_type == AST_DECLARATION_FUNCTION) {
        positions.last_correct = positions.start;

        Src_position last_valid_p = lexer->current_token.src_p;
        require_token_and_report_syntax_error(lexer, token_check, TOKEN_OPEN_PARENTHESIS, positions, "missing open parenthesis '(' in a header of a function declaration", false);
        if (lexer->parser->parsing_errors) return 0;
        positions.last_correct = last_valid_p;
            result->function.param_count = get_param_count(lexer, positions);
            if (lexer->parser->parsing_errors) return 0;
            result->function.params = push_array(lexer->allocator, Parameter, result->function.param_count);

            sfor_count(result->function.params, result->function.param_count) {
                Token t = lexer->current_token;
                it->name = t.name;
                it->src_p = t.src_p;

                t = get_next_token(lexer);
                // At this point this is syntax checked, so this assert should NEVER TRIGGER, keep it as an assert
                assert(t.type == TOKEN_COLON, "this should never trigger. Token after a name in the header of a function declaration must be a colon");

                t = get_next_token(lexer);
                it->type = get_type(type_table(lexer), t.name);

                get_next_token(lexer);

                // +1 because arrays start at 0 and there are (param_count - 1) number of commas so we have to check (param_count - 1) times
                if ((i + 1) < result->function.param_count) {
                    // At this point this is syntax checked, so this assert should NEVER TRIGGER, keep it as an assert
                    assert(lexer->current_token.type == TOKEN_COMMA, "this should never trigger. Parameters in a function declarations must be separated by commas ','");
                    get_next_token(lexer);
                }

                positions.last_correct = lexer->current_token.src_p;
            }
        last_valid_p = lexer->current_token.src_p;
        require_token_and_report_syntax_error(lexer, token_check, TOKEN_CLOSE_PARENTHESIS, positions, "missing close parenthesis ')' in the header of a function declaration", false);
        if (lexer->parser->parsing_errors) return 0;
        positions.last_correct = last_valid_p;

        last_valid_p = lexer->current_token.src_p;
        require_token_and_report_syntax_error(lexer, token_check, TOKEN_RETURN_ARROW, positions, "missing return arrow '->' in the header of a function declaration", false);
        if (lexer->parser->parsing_errors) return 0;
        positions.last_correct = last_valid_p;

        result->function.return_type = get_type(lexer->parser->type_table, lexer->current_token.name);
        result->function.return_src_p = lexer->current_token.src_p;

        if (!result->variable.variable_type) {
            // TODO: handle out of order declaration
            /*
            have a queue and iterate over the queue to check for types, maybe a dependency graph is needed for circularish dependencies, set times_checked to 2 or something and every time the items goes back to the queue decrement the counter, if the counter goes to 0 then throw an error and stop
                struct Pending_declaration {
                    u32 times_checked;
                    Ast_declaration *declaration;
                    Pending_declaration *next;
                }
            */
        }

        // Consume the return type token
        get_next_token(lexer);

        // TODO: Maybe do here the check for open brace to say that a its expected for a function body or something like that
        result->function.block = parse_block(lexer, 0);
    } else if (declaration_type == AST_DECLARATION_COMPOUND) {
        // Consume the struct/union token
        get_next_token(lexer);
        Src_position src_p = lexer->current_token.src_p;
        Compound_parsing_result cpr = parse_compound(lexer);
        result->compound = cpr.compound;
        result->compound->compound_type = compound_type;
        push_type(type_table(lexer), result->name, TYPE_SPEC_NAME, src_p);
    } else if (declaration_type == AST_DECLARATION_ENUM) {
        // Consume the enum token
        get_next_token(lexer);

        positions.last_correct = positions.start;

        result->_enum.enum_type = push_type(type_table(lexer), result->name, TYPE_SPEC_NAME, result->src_p);

        Src_position last_valid_p = lexer->current_token.src_p;
        require_token_and_report_syntax_error(lexer, token_check, TOKEN_OPEN_BRACE, positions, "missing open brace '{' in enum declaration", false);
        if (lexer->parser->parsing_errors) return 0;
        positions.last_correct = last_valid_p;

        {
            result->_enum.item_count = get_enum_items_count(lexer);
            result->_enum.items = push_array(lexer->allocator, Enum_item, result->_enum.item_count);

            sfor_count(result->_enum.items, result->_enum.item_count) {
                Token t = lexer->current_token;

                last_valid_p = lexer->current_token.src_p;
                require_token_and_report_syntax_error(lexer, token_check, TOKEN_NAME, positions, "missing enum value name in enum declaration", false);
                if (lexer->parser->parsing_errors) return 0;
                positions.last_correct = last_valid_p;

                it->name = t.name;
                it->src_p = t.src_p;
                t = lexer->current_token;

                if (t.type == TOKEN_ASSIGNMENT) {
                    get_next_token(lexer);
                    it->value = parse_expression(lexer, 0);
                    if (lexer->parser->parsing_errors) return 0;
                    positions.last_correct = lexer->previous_token.src_p;
                }

                last_valid_p = lexer->current_token.src_p;
                require_token_and_report_syntax_error(lexer, token_check, TOKEN_COMMA, positions, "missing comma ',' in enum declaration", false);
                if (lexer->parser->parsing_errors) return 0;
                positions.last_correct = last_valid_p;
            }
        }

        last_valid_p = lexer->current_token.src_p;
        require_token_and_report_syntax_error(lexer, token_check, TOKEN_CLOSE_BRACE, positions, "missing close brace '}' in enum declaration", false);
        if (lexer->parser->parsing_errors) return 0;
        positions.last_correct = last_valid_p;
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
    bool result = (
           (!parser->finished)
        && (!parser->lexer->parser->parsing_errors)
    );
    return result;
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
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = new_ast_block(lexer->allocator);
    }

    Syntax_error_positions positions;

    result->src_p = lexer->current_token.src_p;
    positions.start = lexer->current_token.type == TOKEN_OPEN_BRACE ? result->src_p : lexer->previous_token.src_p;

    positions.last_correct = lexer->previous_token.src_p;
    require_token_and_report_syntax_error(lexer, token_check, TOKEN_OPEN_BRACE, positions, "missing open brace '{' at the begining of a block", false);
    if (lexer->parser->parsing_errors) return 0;

    Block_parser parser;
    parser.lexer = lexer;
    parser.finished = false;
    Ast_block_creation_iterator it = iterate(result, lexer->allocator);

    while (parsing_block(&parser)) {
        Ast_statement *e = advance(&it);
        parse_statement(lexer, e);
        update_block_parser(&parser);
    }

    if (lexer->parser->parsing_errors) return 0;

    positions.last_correct = lexer->current_token.src_p;
    require_token_and_report_syntax_error(lexer, token_check, TOKEN_CLOSE_BRACE, positions, "missing close brace '}' at the end of a block", false);

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

internal u32 parse_if_else(If_creation_iterator *it) {
    Lexer *lexer = it->lexer;
    u32 result = 0;

    if (it->first_if) {
        If *new_if = push_struct(lexer->allocator, If);

        require_token(lexer, TOKEN_IF, "parse_if");
        it->first_if = false;

            parse_or_expression(lexer, &new_if->condition);

        parse_block(lexer, &new_if->block);

        *it->last = new_if;
        it->last = &new_if->next;

        result = 1;
    } else {
        Token t = lexer->current_token;

        if (t.type == TOKEN_ELSE) {

            t = get_next_token(lexer);

            if (t.type == TOKEN_IF) {
                If *new_if = push_struct(lexer->allocator, If);
                new_if->src_p = t.src_p;

                get_next_token(lexer);

                require_token(lexer, TOKEN_OPEN_PARENTHESIS, "parse_if");
                    parse_binary_expression(lexer, &new_if->condition);
                require_token(lexer, TOKEN_CLOSE_PARENTHESIS, "parse_if");

                parse_block(lexer, &new_if->block);

                *it->last = new_if;
                it->last = &new_if->next;

                result = 1;
            } else {
                it->last_block = true;
                it->result->else_block = parse_block(lexer, 0);
            }
        } else {
            it->last_block = true;
        }
    }

    return result;
}

internal Ast_if *parse_if(Lexer *lexer, Ast_if *result) {
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = push_struct(lexer->allocator, Ast_if);
    }

    If_creation_iterator it = iterate(lexer, result);
    result->src_p = lexer->current_token.src_p;

    while (parsing_if(it)) {
        result->if_count += parse_if_else(&it);
    }

    return result;
}

//
// LOOPS
//

internal Ast_loop *parse_loop(Lexer *lexer, Ast_loop *result=0) {
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = new_ast_loop(lexer->allocator);
    }

    Token loop = lexer->current_token;
    result->src_p = loop.src_p;
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
    Token t1 = lexer->current_token;

    switch (t1.type) {
        case TOKEN_OPEN_BRACE: {
            result = AST_STATEMENT_BLOCK;
        } break;
        case TOKEN_IF: {
            result = AST_STATEMENT_IF;
        } break;
        case TOKEN_OPEN_PARENTHESIS:
        case TOKEN_LITERAL_U32: {
            result = AST_STATEMENT_EXPRESSION;
        } break;
        case TOKEN_NAME: {
            // TODO: do something with the custom types here, the token will be TOKEN_NAME have an if (is_type(token)) that checks in the types hash table if that type exists
            result = AST_STATEMENT_EXPRESSION;
            Token t2 = get_next_token(lexer);

            if (t2.type == TOKEN_COLON) {
                result = AST_STATEMENT_DECLARATION;
            }
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
    if (lexer->parser->parsing_errors) return 0;

    if (!result) {
        result = new_ast_statement(lexer->allocator);
    }

    Token t = lexer->current_token;
    AST_STATEMENT_TYPE type = get_statement_type(lexer);
    result->type = type;
    result->src_p = t.src_p;

    Syntax_error_positions positions;
    positions.start = result->src_p;

    switch (result->type) {
        case AST_STATEMENT_BLOCK: {
            result->block_statement = parse_block(lexer, 0);
        } break;
        case AST_STATEMENT_IF: {
            result->if_statement = parse_if(lexer, 0);
        } break;
        case AST_STATEMENT_EXPRESSION: {
            result->expression_statement = parse_binary_expression(lexer, 0);
            if (lexer->parser->parsing_errors) return 0;
            positions.last_correct = lexer->previous_token.src_p;
            require_token_and_report_syntax_error(lexer, token_check, TOKEN_SEMICOLON, positions, "Missing semicolon ';' at the end of an expression", false);
        } break;
        case AST_STATEMENT_DECLARATION: {
            result->declaration_statement = parse_declaration(lexer, 0);
        } break;
        case AST_STATEMENT_LOOP: {
            result->loop_statement = parse_loop(lexer);
        } break;
        case AST_STATEMENT_BREAK:{
            Src_position src_p = lexer->current_token.src_p;
            require_token(lexer, TOKEN_BREAK, "parsing break statement");
            positions.last_correct = lexer->previous_token.src_p;
            require_token_and_report_syntax_error(lexer, token_check, TOKEN_SEMICOLON, positions, "Missing semicolon ';' at the end of a break statement", false);
        } break;
        case AST_STATEMENT_CONTINUE: {
            Src_position src_p = lexer->current_token.src_p;
            require_token(lexer, TOKEN_CONTINUE, "parsing continue statement");
            positions.last_correct = lexer->previous_token.src_p;
            require_token_and_report_syntax_error(lexer, token_check, TOKEN_SEMICOLON, positions, "Missing semicolon ';' at the end of continue statement", false);
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
        printf("%d[%d:%d]: %s", token.src_p.line, token.src_p.c0, token.src_p.cf, to_string(token.type));

        switch (token.type) {
            case TOKEN_NAME: {
                printf("<%.*s>", token.name.count, token.name.buffer);
                assert(token.name.count == (token.src_p.cf - token.src_p.c0), "the char offset selection in get_next_token is wrong");
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
