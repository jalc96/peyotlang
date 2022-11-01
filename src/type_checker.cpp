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

internal void push_pending_type(Parser *parser, Ast_declaration *pending_to_type, str type_name) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_DECLARATION;
    new_node->node_declaration = pending_to_type;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Ast_statement *pending_to_type, str type_name) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_STATEMENT;
    new_node->node_statement = pending_to_type;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Ast_expression *pending_to_type, str type_name) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_EXPRESSION;
    new_node->node_expression = pending_to_type;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Ast_if *pending_to_type, str type_name) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_IF;
    new_node->node_if = pending_to_type;
    push_back(parser, new_node);
}

internal void push_pending_type(Parser *parser, Ast_loop *pending_to_type, str type_name) {
    Pending_type *new_node = new_pending_type(parser->allocator, type_name);
    new_node->type = PENDING_TYPE_LOOP;
    new_node->node_loop = pending_to_type;
    push_back(parser, new_node);
}

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

        if (it->times_checked > 3) {
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
