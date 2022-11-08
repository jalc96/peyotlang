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

    Split_at b = split_at(a.p2, lp.cf - lp.c0);
    str first_type = b.p1;

    u32 base = l0 + length(prev) + length(first_type);
    Split_at c = split_at(b.p2, rp.c0 - base);
    str operand = c.p1;
    Split_at d = split_at(c.p2, rp.cf - rp.c0);
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
    log_error(eb, "%.*s", STR_PRINT(rest));
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

    Split_at b = split_at(a.p2, lp.cf - lp.c0);
    str first_type = b.p1;

    u32 base = l0 + length(prev) + length(first_type);
    Split_at c = split_at(b.p2, rp.c0 - base);
    str operand = c.p1;
    Split_at d = split_at(c.p2, rp.cf - rp.c0);
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
    log_error(eb, "%.*s", STR_PRINT(rest));
}

internal void type_check(Lexer *lexer, Ast_declaration *ast);
internal void type_check(Lexer *lexer, Ast_statement *ast);
internal void type_check(Lexer *lexer, Ast_expression *ast);
internal void type_check(Lexer *lexer, Ast_if *ast);
internal void type_check(Lexer *lexer, Ast_loop *ast);
internal void type_check(Lexer *lexer, Ast_block *ast);

internal Type_spec *get_type(Symbol_table *symbol_table, Type_spec_table *type_table, str name) {
    Type_spec *result = {};

    lfor (symbol_table) {
        Symbol *s = get(it, name);

        if (s) {
            result = get(type_table, s->type_name);
            break;
        }
    }

    return result;
}

internal Type_spec *get_type(Lexer *lexer, str name) {
    Type_spec *result = get_type(lexer->parser->current_scope, lexer->parser->type_table, name);
    return result;
}

internal Type_spec *get_type(Lexer *lexer, Ast_expression *ast) {
    Type_spec *result = 0;
    Symbol_table *current_scope = lexer->parser->current_scope;
    Type_spec_table *type_table = lexer->parser->type_table;

    if (is_leaf(ast->type)) {
        switch (ast->type) {
            case AST_EXPRESSION_LITERAL_U32: {
                result = get(type_table, STATIC_STR("u32"));
            } break;
            case AST_EXPRESSION_NAME: {
                result = get_type(current_scope, type_table, ast->name);
            } break;
            case AST_EXPRESSION_MEMBER: {} break;
            case AST_EXPRESSION_FUNCTION_CALL: {} break;
        }
    } else if (is_binary(ast->type)) {
        Type_spec *l = get_type(lexer, ast->binary.left);
        Type_spec *r = get_type(lexer, ast->binary.right);

        if (type_errors(lexer->parser)) {return 0;}

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
        result = r;

    }

    return result;
}

internal void type_check(Lexer *lexer, Ast_declaration *ast) {
    switch (ast->type) {
        case AST_DECLARATION_VARIABLE: {
            // TODO: check if already declared
            put(lexer->parser->current_scope, ast->name, ast->variable.variable_type->name);

            if (ast->variable.expression) {
                Type_spec *variable = get_type(lexer, ast->name);
                Type_spec *value = get_type(lexer, ast->variable.expression);

                if (type_errors(lexer->parser)) {return;}

                if (!equals(variable, value)) {
                    report_declaration_missmatch_type_error(lexer, ast, variable, value);
                }
            }
        } break;
        case AST_DECLARATION_FUNCTION: {
            // TODO: put the functions in the global symbol table
            // NOTE(Juan Antonio) 2022-11-08: in the case of a function we add 1 extra scope because blocks always create a new scope
            Parser *parser = lexer->parser;
            Memory_pool mp = {};
            Symbol_table *new_scope = new_symbol_table(&mp);
            new_scope->next = parser->current_scope;
            parser->current_scope = new_scope;

            sfor_count (ast->function.params, ast->function.param_count) {
                put(parser->current_scope, it->name, it->type->name);
            }

            type_check(lexer, ast->function.block);
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
            type_check(lexer, ast->block_statement);
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
    return;

    // TODO: is all of this useless?
    switch (ast->type) {
        case AST_EXPRESSION_LITERAL_U32: {} break;
        case AST_EXPRESSION_NAME: {} break;
        case AST_EXPRESSION_MEMBER: {} break;
        case AST_EXPRESSION_FUNCTION_CALL: {} break;
        case AST_EXPRESSION_UNARY_SUB: {} break;
        case AST_EXPRESSION_BINARY_ADD: {
            Type_spec *l = get_type(lexer, ast->binary.left);
            Type_spec *r = get_type(lexer, ast->binary.right);
        } break;
        case AST_EXPRESSION_BINARY_SUB: {} break;
        case AST_EXPRESSION_BINARY_MUL: {} break;
        case AST_EXPRESSION_BINARY_DIV: {} break;
        case AST_EXPRESSION_BINARY_MOD: {} break;
        case AST_EXPRESSION_BINARY_EQUALS: {} break;
        case AST_EXPRESSION_BINARY_NOT_EQUALS: {} break;
        case AST_EXPRESSION_BINARY_GREATER_THAN: {} break;
        case AST_EXPRESSION_BINARY_GREATER_THAN_OR_EQUALS: {} break;
        case AST_EXPRESSION_BINARY_LESS_THAN: {} break;
        case AST_EXPRESSION_BINARY_LESS_THAN_OR_EQUALS: {} break;
        case AST_EXPRESSION_UNARY_LOGICAL_NOT: {} break;
        case AST_EXPRESSION_BINARY_LOGICAL_OR: {} break;
        case AST_EXPRESSION_BINARY_LOGICAL_AND: {} break;
        case AST_EXPRESSION_UNARY_BITWISE_NOT: {} break;
        case AST_EXPRESSION_BINARY_BITWISE_OR: {} break;
        case AST_EXPRESSION_BINARY_BITWISE_AND: {} break;
        case AST_EXPRESSION_BINARY_ASSIGNMENT: {
            Type_spec *l = get_type(lexer, ast->binary.left);
            Type_spec *r = get_type(lexer, ast->binary.right);
        } break;
    }
}

internal void type_check(Lexer *lexer, Ast_if *ast) {
    If *ifs = ast->ifs;

    while (ifs) {
        type_check(lexer, &ifs->condition);
        type_check(lexer, &ifs->block);
    }

    if (ast->else_block) {
        type_check(lexer, ast->else_block);
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

    type_check(lexer, ast->block);
}

internal void type_check(Lexer *lexer, Ast_block *ast) {
    Memory_pool mp = {};
    Symbol_table *new_scope = new_symbol_table(&mp);
    new_scope->next = lexer->parser->current_scope;
    lexer->parser->current_scope = new_scope;

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
