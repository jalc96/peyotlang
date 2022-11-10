internal Pending_type *push_back(Parser *parser, Pending_type *new_node) {
    Pending_type *sentinel = &parser->sentinel;

    new_node->previous = sentinel->previous;
    new_node->next = sentinel;

    sentinel->previous->next = new_node;
    sentinel->previous = new_node;

    return new_node;
}

internal void pop(Parser *parser, Pending_type *node) {
    // TODO: have a free list
    node->previous->next = node->next;
    node->next->previous = node->previous;
}

internal void push_pending_type(Parser *parser, Ast_declaration *pending_to_type, str type_name, Src_position type_name_location) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_DECLARATION;
    new_node->node_declaration = pending_to_type;
    new_node->type_name_location = type_name_location;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Ast_statement *pending_to_type, str type_name, Src_position type_name_location) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_STATEMENT;
    new_node->node_statement = pending_to_type;
    new_node->type_name_location = type_name_location;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Ast_expression *pending_to_type, str type_name, Src_position type_name_location) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_EXPRESSION;
    new_node->node_expression = pending_to_type;
    new_node->type_name_location = type_name_location;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Ast_if *pending_to_type, str type_name, Src_position type_name_location) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_IF;
    new_node->node_if = pending_to_type;
    new_node->type_name_location = type_name_location;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Ast_loop *pending_to_type, str type_name, Src_position type_name_location) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_LOOP;
    new_node->node_loop = pending_to_type;
    new_node->type_name_location = type_name_location;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Parameter *pending_to_type, str type_name, Src_position type_name_location) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_FUNCTION_PARAMETER;
    new_node->node_parameter = pending_to_type;
    new_node->type_name_location = type_name_location;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Member *pending_to_type, str type_name, Src_position type_name_location) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_MEMBER;
    new_node->node_member = pending_to_type;
    new_node->type_name_location = type_name_location;
    push_back(parser, new_node);
}

/*
have a queue and iterate over the queue to check for types, maybe a dependency graph is needed for circularish dependencies, set times_checked to 2 or something and every time the items goes back to the queue decrement the counter, if the counter goes to 0 then throw an error and stop
*/

internal void out_of_order_declaration(Parser *parser) {
    Pending_type *prev = 0;
    Pending_type *it = parser->sentinel.next;

    while (1) {
        // TODO: this is a very crappy doubly linked list

        if (parser->sentinel.next == &parser->sentinel) {
            break;
        }

        if (it == &parser->sentinel) {
            it = it->next;
            continue;
        }

        if (it->times_checked >= 3) {
            break;
        }

        Type_spec *type = get(parser->type_table, it->type_name);
        it->times_checked++;

        if (!type) {
            pop(parser, it);
            push_back(parser, it);

            it = it->next;

            continue;
        }

        switch (it->type) {
            case PENDING_TYPE_DECLARATION: {
                Ast_declaration *ast = it->node_declaration;

                if (ast->type == AST_DECLARATION_VARIABLE) {

                } else if (ast->type == AST_DECLARATION_FUNCTION) {
                    ast->function.return_type = type;
                } else if (ast->type == AST_DECLARATION_COMPOUND) {

                } else if (ast->type == AST_DECLARATION_ENUM) {

                }
            }break;

            case PENDING_TYPE_STATEMENT: {
                Ast_statement *node_statement;
            }break;

            case PENDING_TYPE_EXPRESSION: {
                Ast_expression *node_expression;
            }break;

            case PENDING_TYPE_IF: {
                Ast_if *node_if;
            }break;

            case PENDING_TYPE_LOOP: {
                Ast_loop *node_loop;
            }break;
        }

        pop(parser, it);

        it = it->next;
    }
}

internal Src_position get_sub_tree_width(Ast_expression *ast) {
    Src_position result;

    if (is_unary(ast->type)) {
        result = merge(ast->src_p, get_sub_tree_width(ast->binary.right));
    } else if (is_binary(ast->type)) {
        result = merge(get_sub_tree_width(ast->binary.left), get_sub_tree_width(ast->binary.right));
    } else {
        assert(is_leaf(ast->type), "get_sub_tree_width else must be a leaf node");

        result = ast->src_p;
    }

    return result;
}

internal void report_binary_expression_missmatch_type_error(Lexer *lexer, Ast_expression *ast, Type_spec *l, Type_spec *r) {
    lexer->parser->type_errors = true;
    Str_buffer *eb = &lexer->parser->error_buffer;

    Src_position lp = get_sub_tree_width(ast->binary.left);
    Src_position rp = get_sub_tree_width(ast->binary.right);





    // THIS PART DOWN HERE LOOKS THAT ITS INDEPENDANT FROM THE TOP, DO MORE REPORT FUNCTIONS AND UNIFY IF ITS THE SAME

    u32 l0 = find_first_from_position(lexer->source, lp.c0, '\n', true) + skip_new_line;
    u32 lf = find_first_from_position(lexer->source, rp.cf, '\n', false);

    str line = slice(lexer->source, l0, lf);

    Split_at a = split_at(line, lp.c0 - l0);
    str prev = a.p1;

    Split_at b = split_at(a.p2, length(lp));
    str first_type = b.p1;

    u32 base = l0 + length(prev) + length(first_type);
    Split_at c = split_at(b.p2, rp.c0 - base);
    str operand = c.p1;
    Split_at d = split_at(c.p2, length(rp));
    str second_type = d.p1;
    str rest = d.p2;

    // TODO: handle multiline expressions


    log_error(eb, STATIC_RED("TYPE ERROR"), 0);
    log_error(eb, ": no valid operator between ");
    log_error(eb, STATIC_COLOR("%.*s", 100, 255, 100), STR_PRINT(l->name));
    log_error(eb, " and ");
    log_error(eb, STATIC_COLOR("%.*s\n", 100, 100, 255), STR_PRINT(r->name));

    log_error(eb, "    %d:%.*s", lp.line, STR_PRINT(prev));
    log_error(eb, STATIC_COLOR("%.*s", 100, 255, 100), STR_PRINT(first_type));
    log_error(eb, "%.*s", STR_PRINT(operand));
    log_error(eb, STATIC_COLOR("%.*s", 100, 100, 255), STR_PRINT(second_type));
    log_error(eb, "%.*s\n", STR_PRINT(rest));
}

internal void report_declaration_missmatch_type_error(Lexer *lexer, Ast_declaration *ast, Type_spec *l, Type_spec *r) {
    lexer->parser->type_errors = true;
    Str_buffer *eb = &lexer->parser->error_buffer;

    Src_position lp = ast->src_p;
    Src_position rp = get_sub_tree_width(ast->variable.expression);



    // THIS PART DOWN HERE LOOKS THAT ITS INDEPENDANT FROM THE TOP, DO MORE REPORT FUNCTIONS AND UNIFY IF ITS THE SAME

    u32 l0 = find_first_from_position(lexer->source, lp.c0, '\n', true) + skip_new_line;
    u32 lf = find_first_from_position(lexer->source, rp.cf, '\n', false);

    str line = slice(lexer->source, l0, lf);

    Split_at a = split_at(line, lp.c0 - l0);
    str prev = a.p1;

    Split_at b = split_at(a.p2, length(lp));
    str first_type = b.p1;

    u32 base = l0 + length(prev) + length(first_type);
    Split_at c = split_at(b.p2, rp.c0 - base);
    str operand = c.p1;
    Split_at d = split_at(c.p2, length(rp));
    str second_type = d.p1;
    str rest = d.p2;

    // TODO: handle multiline expressions


    log_error(eb, STATIC_RED("TYPE ERROR"), 0);
    log_error(eb, ": no valid operator between ");
    log_error(eb, STATIC_COLOR("%.*s", 100, 255, 100), STR_PRINT(l->name));
    log_error(eb, " and ");
    log_error(eb, STATIC_COLOR("%.*s\n", 100, 100, 255), STR_PRINT(r->name));

    log_error(eb, "    %d:%.*s", lp.line, STR_PRINT(prev));
    log_error(eb, STATIC_COLOR("%.*s", 100, 255, 100), STR_PRINT(first_type));
    log_error(eb, "%.*s", STR_PRINT(operand));
    log_error(eb, STATIC_COLOR("%.*s", 100, 100, 255), STR_PRINT(second_type));
    log_error(eb, "%.*s\n", STR_PRINT(rest));
}

internal void report_undeclared_identifier(Lexer *lexer, str name, Src_position src_p) {
    lexer->parser->type_errors = true;
    Str_buffer *eb = &lexer->parser->error_buffer;

    Src_position lp = src_p;



    // THIS PART DOWN HERE LOOKS THAT ITS INDEPENDANT FROM THE TOP, DO MORE REPORT FUNCTIONS AND UNIFY IF ITS THE SAME

    u32 l0 = find_first_from_position(lexer->source, lp.c0, '\n', true) + skip_new_line;
    u32 lf = find_first_from_position(lexer->source, lp.cf, '\n', false);

    str line = slice(lexer->source, l0, lf);
    Split_at a = split_at(line, src_p.c0 - l0);
    str pre = a.p1;
    Split_at b = split_at(a.p2, length(src_p));
    str undeclared = b.p1;
    str post = b.p2;

    log_error(eb, STATIC_RED("TYPE ERROR"), 0);
    log_error(eb, ": undeclared identifier ");
    log_error(eb, STATIC_RED("%.*s\n"), STR_PRINT(name));

    log_error(eb, "    %d:%.*s", lp.line, STR_PRINT(pre));
    log_error(eb, STATIC_RED("%.*s"), STR_PRINT(undeclared));
    log_error(eb, "%.*s\n", STR_PRINT(post));
}

internal void report_variable_redefinition(Lexer *lexer, Ast_declaration *ast, Src_position previous_definition_p) {
    lexer->parser->type_errors = true;
    Str_buffer *eb = &lexer->parser->error_buffer;

    u32 pl0 = find_first_from_position(lexer->source, previous_definition_p.c0, '\n', true) + skip_new_line;
    u32 plf = find_first_from_position(lexer->source, previous_definition_p.cf, '\n', false);

    str previous_definition_line = slice(lexer->source, pl0, plf);

    u32 rl0 = find_first_from_position(lexer->source, ast->src_p.c0, '\n', true) + skip_new_line;
    u32 rlf = find_first_from_position(lexer->source, ast->src_p.cf, '\n', false);

    str redefinition_line = slice(lexer->source, rl0, rlf);

    Split_at a = split_at(previous_definition_line, previous_definition_p.c0 - pl0);
    Split_at b = split_at(a.p2, length(previous_definition_p));

    str pd_prev = a.p1;
    str pd_name = b.p1;
    str pd_post = b.p2;

    a = split_at(redefinition_line, ast->src_p.c0 - rl0);
    b = split_at(a.p2, length(ast->src_p));

    str rd_prev = a.p1;
    str rd_name = b.p1;
    str rd_post = b.p2;

    log_error(eb, STATIC_RED("TYPE ERROR"), 0);
    log_error(eb, ": variable redefinition, ");
    log_error(eb, STATIC_RED("%.*s\n"), STR_PRINT(ast->name));

    log_error(eb, "    %d:%.*s", ast->src_p.line, STR_PRINT(rd_prev));
    log_error(eb, STATIC_RED("%.*s"), STR_PRINT(rd_name));
    log_error(eb, "%.*s\n", STR_PRINT(rd_post));


    log_error(eb, "Was already defined in line %d\n", previous_definition_p.line);

    log_error(eb, "    %d:%.*s", previous_definition_p.line, STR_PRINT(pd_prev));
    log_error(eb, STATIC_CYAN("%.*s"), STR_PRINT(pd_name));
    log_error(eb, "%.*s\n", STR_PRINT(pd_post));
}

internal void type_check(Lexer *lexer, Ast_declaration *ast);
internal void type_check(Lexer *lexer, Ast_statement *ast);
internal void type_check(Lexer *lexer, Ast_expression *ast);
internal void type_check(Lexer *lexer, Ast_if *ast);
internal void type_check(Lexer *lexer, Ast_loop *ast);
internal void type_check(Lexer *lexer, Ast_block *ast, bool create_scope);

internal Type_spec *get_type(Symbol_table *symbol_table, Type_spec_table *type_table, str name) {
    Type_spec *result = {};

    Symbol *s = get(symbol_table, name);

    if (s) {
        result = get(type_table, s->type_name);
    }

    return result;
}

internal Type_spec *get_type(Lexer *lexer, str name) {
    Type_spec *result = get_type(lexer->parser->current_scope, lexer->parser->type_table, name);
    return result;
}

internal bool is_unsigned(Type_spec *type) {
    str unsigned_types[] = {
        STATIC_STR("u8"),
        STATIC_STR("u16"),
        STATIC_STR("u32"),
        STATIC_STR("u64"),
    };

    sfor (unsigned_types) {
        if (equals(*it, type->name)) {
            return true;
        }
    }

    return false;
}

internal Type_spec *to_signed(Type_spec_table *type_table, Type_spec *unsigned_type) {
    Type_spec *result;

    if (equals(STATIC_STR("u8"), unsigned_type->name)) {
        result = get(type_table, STATIC_STR("s8"));
    } else if (equals(STATIC_STR("u16"), unsigned_type->name)) {
        result = get(type_table, STATIC_STR("s16"));
    } else if (equals(STATIC_STR("u32"), unsigned_type->name)) {
        result = get(type_table, STATIC_STR("s32"));
    } else if (equals(STATIC_STR("u64"), unsigned_type->name)) {
        result = get(type_table, STATIC_STR("s64"));
    }

    return result;
}

internal Type_spec *get_type(Lexer *lexer, Ast_expression *ast) {
    // TODO: have a table of inplicit casts and check here for those like asigning a u8 to an u32 variable or something like that
    Type_spec *result = 0;
    Parser *parser = lexer->parser;
    Symbol_table *current_scope = parser->current_scope;
    Type_spec_table *type_table = parser->type_table;

    if (is_leaf(ast->type)) {
        switch (ast->type) {
            case AST_EXPRESSION_LITERAL_INTEGER: {
                result = get(type_table, STATIC_STR("u32"));
            } break;
            case AST_EXPRESSION_LITERAL_FLOAT: {
                result = get(type_table, STATIC_STR("f32"));
            } break;
            case AST_EXPRESSION_NAME: {
                result = get_type(current_scope, type_table, ast->name);

                if (!result) {
                    // NOTE(Juan Antonio) 2022-11-08: this has an inplicit call to the symbol table, so if 0 is return then the symbol doesnt exists
                    report_undeclared_identifier(lexer, ast->name, ast->src_p);
                }
            } break;
            case AST_EXPRESSION_MEMBER: {} break;
            case AST_EXPRESSION_FUNCTION_CALL: {} break;
        }
    } else if (is_binary(ast->type)) {
        Type_spec *l = get_type(lexer, ast->binary.left);
        Type_spec *r = get_type(lexer, ast->binary.right);

        if (type_errors(parser)) {return 0;}

        if (is_arithmetic(ast->type) || is_relational(ast->type)) {
            // TODO: in the future check here if there is an operator that accepts these 2 types
            if (equals(l, r)) {
                result = l;
            } else {
                report_binary_expression_missmatch_type_error(lexer, ast, l, r);
            }
        } else if (is_assignment(ast->type)) {
            // TODO: in the future check here if there is an operator that accepts these 2 types
            if (equals(l, r)) {
                result = l;
            } else {
                report_binary_expression_missmatch_type_error(lexer, ast, l, r);
            }
        } else if (is_boolean(ast->type)) {
            // TODO: check if both sides are bool
        } else if (is_bit_operator(ast->type)) {
            // TODO: check for correct types (left has to be s32 or u32 or something like that and right aswell)
        }
    } else if (is_unary(ast->type)) {
        Type_spec *r = get_type(lexer, ast->binary.right);

        if (type_errors(parser)) {return 0;}

        if (ast->type == AST_EXPRESSION_UNARY_SUB) {
            if (is_unsigned(r)) {
                r = to_signed(lexer->parser->type_table, r);
            }
        }

        result = r;
    }

    return result;
}

internal void type_check(Lexer *lexer, Ast_declaration *ast) {
    Parser *parser = lexer->parser;

    switch (ast->type) {
        case AST_DECLARATION_VARIABLE: {
            Symbol *s = get(parser->current_scope, ast->name);

            if (s) {
                // TODO: do something here for variable shadowing maybe in the get function above have a flag bool only_in_current_scope and allow to redeclare variables in deeper scopes just check for redefinitions in the same scope level
                report_variable_redefinition(lexer, ast, s->src_p);
            }

            if (type_errors(parser)) {return;}

            put(parser->current_scope, ast->name, ast->variable.variable_type->name, ast->src_p);

            if (ast->variable.expression) {
                Type_spec *variable = get_type(lexer, ast->name);
                Type_spec *value = get_type(lexer, ast->variable.expression);

                if (type_errors(parser)) {return;}

                if (!equals(variable, value)) {
                    report_declaration_missmatch_type_error(lexer, ast, variable, value);
                }
            }
        } break;
        case AST_DECLARATION_FUNCTION: {
            put(parser->current_scope, ast->name, ast->function.return_type->name, ast->src_p);

            Memory_pool mp = {};
            Symbol_table *new_scope = new_symbol_table(&mp);
            new_scope->next = parser->current_scope;
            parser->current_scope = new_scope;

            sfor_count (ast->function.params, ast->function.param_count) {
                put(parser->current_scope, it->name, it->type->name, it->src_p);
            }

            // NOTE(Juan Antonio) 2022-11-09: false in chreate_scopre because in the case of a function the parameters in the header are in the same scope level as the first level scope inside the function body
            type_check(lexer, ast->function.block, false);
            parser->current_scope = parser->current_scope->next;
            clear(&mp);
        } break;
        case AST_DECLARATION_COMPOUND: {} break;
        case AST_DECLARATION_ENUM: {} break;

        invalid_default_case_msg("ast_declaration type_check missing type");
    }
}

internal void type_check(Lexer *lexer, Ast_statement *ast) {
    switch (ast->type) {
        case AST_STATEMENT_BLOCK: {
            type_check(lexer, ast->block_statement, true);
        } break;
        case AST_STATEMENT_IF: {
            type_check(lexer, ast->if_statement);
        } break;
        case AST_STATEMENT_LOOP: {
            type_check(lexer, ast->loop_statement);
        } break;
        case AST_STATEMENT_EXPRESSION: {
            type_check(lexer, ast->expression_statement);
        } break;
        case AST_STATEMENT_DECLARATION: {
            type_check(lexer, ast->declaration_statement);
        } break;
        case AST_STATEMENT_BREAK: {} break;
        case AST_STATEMENT_CONTINUE: {} break;
        case AST_STATEMENT_RETURN: {} break;

        invalid_default_case_msg("ast_statement type_check missing type");
    }
}

internal void type_check(Lexer *lexer, Ast_expression *ast) {
    get_type(lexer, ast);
}

internal void type_check(Lexer *lexer, Ast_if *ast) {
    If *ifs = ast->ifs;

    while (ifs) {
        type_check(lexer, &ifs->condition);
        type_check(lexer, &ifs->block, true);
    }

    if (ast->else_block) {
        type_check(lexer, ast->else_block, true);
    }
}

internal void type_check(Lexer *lexer, Ast_loop *ast) {
    if (ast->pre) {
        type_check(lexer, ast->pre);
    }

    type_check(lexer, ast->condition);

    if (ast->post) {
        type_check(lexer, ast->post);
    }

    type_check(lexer, ast->block, true);
}

internal void type_check(Lexer *lexer, Ast_block *ast, bool create_scope) {
    // NOTE(Juan Antonio) 2022-11-09: the create_scope parameter is actually for the function parameters to have them in the same scope as the variables in the function block to not allow to shadow the parameters in the first level of the function scope
    Memory_pool mp = {};

    if (create_scope) {
        Symbol_table *new_scope = new_symbol_table(&mp);
        new_scope->next = lexer->parser->current_scope;
        lexer->parser->current_scope = new_scope;
    }

    while (ast) {
        sfor_count (ast->statements, ast->statement_count) {
            type_check(lexer, it);
        }

        ast = ast->next;
    }

    lexer->parser->current_scope = lexer->parser->current_scope->next;
    clear(&mp);
}

internal void type_check(Lexer *lexer, Ast_program *ast) {
    while (ast) {
        sfor_count (ast->declarations, ast->declaration_count) {
            type_check(lexer, it);
        }

        ast = ast->next;
    }
}

internal void report_type_declaration_errors(Lexer *lexer) {
    Parser *parser = lexer->parser;
    Pending_type *it = parser->sentinel.next;
    Str_buffer *eb = &parser->error_buffer;

    while (it != &parser->sentinel) {
        Src_position p = it->type_name_location;

        u32 l0 = find_first_from_position(lexer->source, p.c0, '\n', true) + skip_new_line;
        u32 lf = find_first_from_position(lexer->source, p.cf, '\n', false);

        str line = slice(lexer->source, l0, lf);
        u32 position_in_line = p.c0 - l0;
        u32 type_name_length = p.cf - p.c0;
        debug(type_name_length)

        u32 where_the_missing_type_begins = p.c0 - l0;
        u32 where_the_missing_type_ends = p.cf - p.c0;
        Split_at a = split_at(line, where_the_missing_type_begins);
        Split_at b = split_at(a.p2, where_the_missing_type_ends);

        log_error(eb, STATIC_RED("TYPE ERROR"), 0);
        log_error(eb, ": undeclared type <%.*s>\n", STR_PRINT(it->type_name));
        log_error(eb, "    %d:%.*s", p.line, STR_PRINT(a.p1));
        log_error(eb, STATIC_RED("%.*s"), STR_PRINT(b.p1));
        log_error(eb, "%.*s\n", STR_PRINT(b.p2));

        // log_error(eb, "    %d:%.*s\n", p.line, line.count, line.buffer);
        // log_error(eb, "    %*s%*s\n", position_in_line, "", type_name_length, "^");
        // log_error(eb, "    %*s", position_in_line, "");

        // for_count(type_name_length) {
        //     log_error(eb, "^");
        // }

        // log_error(eb, "\n");


        it = it->next;
    }

    printf("%.*s\n", eb->head, eb->buffer);
}

internal void report_type_errors(Lexer *lexer) {
    Parser *parser = lexer->parser;
    Str_buffer *eb = &parser->error_buffer;

    printf("%.*s\n", eb->head, eb->buffer);
}
