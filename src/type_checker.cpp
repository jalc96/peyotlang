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

        Type_spec *type = get_type(parser->type_table, it->type_name);
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


internal void type_check(Lexer *lexer, Ast_declaration *ast) {

}

internal void type_check(Lexer *lexer, Ast_statement *ast) {

}

internal void type_check(Lexer *lexer, Ast_expression *ast) {

}

internal void type_check(Lexer *lexer, Ast_if *ast) {

}

internal void type_check(Lexer *lexer, Ast_loop *ast) {

}

internal void type_check(Lexer *lexer, Ast_block *ast) {

}

internal void type_check(Lexer *lexer, Ast_program *ast) {
    while (ast) {
        sfor_count (ast->declarations, ast->count) {
            type_check(lexer, it);
        }

        ast = ast->next;
    }
}

internal void report_type_errors(Lexer *lexer) {
    Parser *parser = lexer->parser;
    Pending_type *it = parser->sentinel.next;
    Str_buffer *eb = &parser->error_buffer;

    while (it != &parser->sentinel) {
        log_error(eb, STATIC_RED("TYPE ERROR"), 0);
        log_error(eb, ": undeclared type <%.*s>\n", it->type_name.count, it->type_name.buffer);

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

        log_error(eb, "    %d:%.*s", p.line, a.p1.count, a.p1.buffer);
        log_error(eb, STATIC_RED("%.*s"), b.p1.count, b.p1.buffer);
        log_error(eb, "%.*s\n", b.p2.count, b.p2.buffer);

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
