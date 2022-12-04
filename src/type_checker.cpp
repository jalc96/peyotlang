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

// TODO: do we really need this list??? maybe just store the type_name and then query it in the typecheck, then we dont need the out of order call
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
                    ast->_operator.declaration->return_type = type;
                } else if (ast->type == AST_DECLARATION_FUNCTION) {
                    ast->function->return_type = type;
                } else if (ast->type == AST_DECLARATION_COMPOUND) {

                } else if (ast->type == AST_DECLARATION_ENUM) {

                }
            } break;

            case PENDING_TYPE_STATEMENT: {
                Ast_statement *node_statement;
            } break;

            case PENDING_TYPE_EXPRESSION: {
                Ast_expression *node_expression;
            } break;

            case PENDING_TYPE_IF: {
                Ast_if *node_if;
            } break;

            case PENDING_TYPE_LOOP: {
                Ast_loop *node_loop;
            } break;

            case PENDING_TYPE_FUNCTION_PARAMETER: {
                Parameter *parameter = it->node_parameter;
                parameter->type = type;
            } break;

            invalid_default_case_msg("missing out of order declaration pending type");
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

internal void report_no_return_for_function(Lexer *lexer, Ast_declaration *ast) {
    // TODO: Handle multiline function declaration for this error
    lexer->parser->type_errors = true;
    Str_buffer *eb = &lexer->parser->error_buffer;

    Src_position src_p = ast->src_p;
    u32 l0 = find_first_from_position(lexer->source, src_p.c0, '\n', true) + skip_new_line;
    u32 lf = find_first_from_position(lexer->source, src_p.cf, '\n', false);

    str line = slice(lexer->source, l0, lf);
    str rt_name = ast->function->return_type->name;

    Split_at a = split_at(line, src_p.c0 - l0);
    str prev = a.p1;

    Split_at b = split_at(a.p2, length(src_p));
    str f_name = b.p1;

    u32 return_type_p_rebased = ast->function->return_src_p.c0 - (l0 + length(a.p1) + length(b.p1));
    Split_at c = split_at(b.p2, return_type_p_rebased);
    str name_to_return = c.p1;

    Split_at d = split_at(c.p2, length(rt_name));
    str return_type = d.p1;
    str rest = d.p2;


    log_error(eb, STATIC_RED("TYPE ERROR"), 0);
    log_error(eb, ": ");
    log_error(eb, STATIC_COLOR("%.*s", 100, 255, 100), STR_PRINT(ast->name));
    log_error(eb, " must return a ");
    log_error(eb, STATIC_COLOR("%.*s", 100, 100, 255), STR_PRINT(rt_name));
    log_error(eb, " value\n");


    log_error(eb, "    %d:%.*s", src_p.line, STR_PRINT(prev));
    log_error(eb, STATIC_COLOR("%.*s", 100, 255, 100), STR_PRINT(f_name));
    log_error(eb, "%.*s", STR_PRINT(name_to_return));
    log_error(eb, STATIC_COLOR("%.*s", 100, 100, 255), STR_PRINT(return_type));
    log_error(eb, "%.*s\n", STR_PRINT(rest));
}

internal void report_break_continue_outside_loop(Lexer *lexer, Ast_statement *ast) {
    lexer->parser->type_errors = true;
    Str_buffer *eb = &lexer->parser->error_buffer;
    str name;
    Src_position src_p = ast->src_p;

    if (ast->type == AST_STATEMENT_BREAK) {
        name = STATIC_STR("break");
    } else {
        assert(ast->type == AST_STATEMENT_CONTINUE, "this should never fire, ast->type was not break or continue in the break continue report");
        name = STATIC_STR("continue");
    }

    u32 l0 = find_first_from_position(lexer->source, src_p.c0, '\n', true) + skip_new_line;
    u32 lf = find_first_from_position(lexer->source, src_p.cf, '\n', false);

    str line = slice(lexer->source, l0, lf);

    Split_at a = split_at(line, src_p.c0 - l0);
    str prev = a.p1;

    Split_at b = split_at(a.p2, length(name));
    str post = b.p2;

    log_error(eb, STATIC_RED("TYPE ERROR"), 0);
    log_error(eb, ": ");
    log_error(eb, STATIC_COLOR("%.*s", 100, 100, 255), STR_PRINT(name));
    log_error(eb, " outside of a loop\n");


    log_error(eb, "    %d:%.*s", src_p.line, STR_PRINT(prev));
    log_error(eb, STATIC_COLOR("%.*s", 100, 100, 255), STR_PRINT(name));
    log_error(eb, "%.*s\n", STR_PRINT(post));
}

internal void report_missmatched_return_types(Lexer *lexer, Ast_statement *ast, Type_spec *function_type, Type_spec *return_type) {
    lexer->parser->type_errors = true;
    Str_buffer *eb = &lexer->parser->error_buffer;

    Src_position lp = ast->return_statement.function->function->return_src_p;
    Src_position rp = {};

    if (ast->return_statement.return_expression) {
        // return <NAME>;
        rp = get_sub_tree_width(ast->return_statement.return_expression);
    } else {
        // return;
        rp = ast->src_p;
        rp.cf = rp.c0;
    }



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
    log_error(eb, ": missmatch in return type, function return type is ");
    log_error(eb, STATIC_COLOR("%.*s", 100, 255, 100), STR_PRINT(function_type->name));
    log_error(eb, " and returned type is ");
    log_error(eb, STATIC_COLOR("%.*s\n", 100, 100, 255), STR_PRINT(return_type->name));

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

internal void report_member_not_found(Lexer *lexer, str type_name, str member_name, Src_position src_p, bool highlight_the_variable_name) {
    lexer->parser->type_errors = true;
    Str_buffer *eb = &lexer->parser->error_buffer;

    u32 l0 = find_first_from_position(lexer->source, src_p.c0, '\n', true) + skip_new_line;
    u32 lf = find_first_from_position(lexer->source, src_p.cf, '\n', false);

    str line = slice(lexer->source, l0, lf);
    Split_at a = split_at(line, src_p.c0 - l0);
    str pre = a.p1;
    Split_at b = split_at(a.p2, length(src_p));
    str name = b.p1;
    str post = b.p2;

    log_error(eb, STATIC_RED("TYPE ERROR"));
    log_error(eb, ": variable");
    log_error(eb, STATIC_RED(" %.*s "), STR_PRINT(type_name));
    log_error(eb, "doesnt have");
    log_error(eb, STATIC_RED(" %.*s "), STR_PRINT(member_name));
    log_error(eb, "member\n");

    log_error(eb, "    %d:%.*s", src_p.line, STR_PRINT(pre));

    if (highlight_the_variable_name) {
        log_error(eb, STATIC_RED("%.*s"), STR_PRINT(name));
    } else {
        log_error(eb, "%.*s", STR_PRINT(name));
    }

    log_error(eb, "%.*s\n", STR_PRINT(post));
}

internal void type_check(Lexer *lexer, Ast_declaration *ast);
internal void type_check(Lexer *lexer, Ast_statement *ast, Ast_declaration *ast_function, Ast_statement *ast_loop);
internal void type_check(Lexer *lexer, Ast_expression *ast, bool need_lvalue, Type_spec *l_type);
internal void type_check(Lexer *lexer, Ast_if *ast, Ast_declaration *ast_function, Ast_statement *ast_loop);
internal void type_check(Lexer *lexer, Ast_loop *ast, Ast_declaration *ast_function, Ast_statement *ast_loop);
internal void type_check(Lexer *lexer, Ast_block *ast, bool create_scope, Ast_declaration *ast_function, Ast_statement *ast_loop);

internal Type_spec *get_type_of_name(Symbol_table *symbol_table, Type_spec_table *type_table, str name) {
    Type_spec *result = {};

    Symbol *s = get(symbol_table, name);

    if (s) {
        result = get(type_table, s->type_name);
    }

    return result;
}

internal Type_spec *get_type_of_name(Lexer *lexer, str name) {
    Type_spec *result = get_type_of_name(lexer->parser->current_scope, lexer->parser->type_table, name);
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

internal Type_spec *get_type(Lexer *lexer, Ast_expression *ast, bool need_lvalue, Type_spec *l_type) {
    // TODO: have a table of inplicit casts and check here for those like asigning a u8 to an u32 variable or something like that
    Type_spec *result = 0;
    Parser *parser = lexer->parser;
    Symbol_table *current_scope = parser->current_scope;
    Type_spec_table *type_table = parser->type_table;

    if (is_leaf(ast->type)) {
        switch (ast->type) {
            case AST_EXPRESSION_LITERAL_TYPE: {
                result = get(type_table, ast->name);
            } break;
            case AST_EXPRESSION_LITERAL_CHAR: {
                result = get(type_table, STATIC_STR("char"));
            } break;
            case AST_EXPRESSION_LITERAL_STR: {
                result = get(type_table, STATIC_STR("str"));
            } break;
            case AST_EXPRESSION_LITERAL_INTEGER: {
                result = get(type_table, STATIC_STR("u32"));
            } break;
            case AST_EXPRESSION_LITERAL_FLOAT: {
                result = get(type_table, STATIC_STR("f32"));
            } break;
            case AST_EXPRESSION_NAME: {
                if (get(type_table, ast->name)) {
                    result = get(type_table, STATIC_STR("u32"));
                } else {
                    // is a name
                    result = get_type_of_name(current_scope, type_table, ast->name);

                    if (!result) {
                        // NOTE(Juan Antonio) 2022-11-08: this has an inplicit call to the symbol table, so if 0 is return then the symbol doesnt exists
                        report_undeclared_identifier(lexer, ast->name, ast->src_p);
                    }
                }
            } break;
            case AST_EXPRESSION_MEMBER: {
                Symbol *symbol = get(current_scope, ast->name);
                Type_spec *type = get(type_table, symbol->type_name);

                if (type) {
                    Type_spec *base = get_base(type);
                    Member_info *member_info = get(base->member_info_table, ast->binary.right->name);

                    if (member_info) {
                        result = get(type_table, member_info->type_name);
                    } else {
                        report_member_not_found(lexer, ast->name, ast->binary.right->name, ast->src_p, true);
                    }
                } else {
                    // TODO: if i store the type_names instead of the typespecs then its here where the error should be reported
                    assert(false, "undefined type this should never trigger because of the out of order declaration checks");
                }
            } break;
            case AST_EXPRESSION_FUNCTION_CALL: {} break;
        }
    } else if (is_binary(ast->type)) {
        // TODO: should we pass here the l_value?? i think so
        Type_spec *l = get_type(lexer, ast->binary.left, need_lvalue, l_type);
        // The l-value is used for operator finding
        Type_spec *r = get_type(lexer, ast->binary.right, true, l);

        if (type_errors(parser)) {return 0;}

        if (is_arithmetic(ast->type) || is_relational(ast->type)) {
            // TODO: in the future check here if there is an operator that accepts these 2 types
            // TODO: with the operator overload the any_equals check is not necessary
            if (any_equals(l, r)) {
                Type_spec *op_type;

                if (need_lvalue) {
                    op_type = l_type;
                } else {
                    op_type = l;
                }

                if (need_lvalue && !l_type) {
                    not_implemented;
                    // error: ambiguous expression or something like that
                } else {
                    Operator *op = get(parser->operator_table, to_op_token_type(ast->type), l->name, r->name, op_type->name);

                    if (op) {
                        result = l_type;
                    } else {
                        // report_operator_not_found(lexer, ast, l, r);
                        debug("error no operator found")
                    }
                }
            } else {
                report_binary_expression_missmatch_type_error(lexer, ast, l, r);
            }
        } else if (is_assignment(ast->type)) {
            // TODO: in the future check here if there is an operator that accepts these 2 types
            // TODO: here we lose the hability to have more return types for operator overload in the r-value
            // for example if operator + (a :u32, b: u32) -> u32
            // and            operator + (a :u32, b: u32) -> f32
            // then here we dont know which one to use
            // SOLUTION: use the l_type parameter
            if (any_equals(l, r)) {
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
        // TODO: check here if is a post/pre and only allow the right node to be a non pre/post to avoid a++++++++;
        Type_spec *r = get_type(lexer, ast->binary.right, false, 0);

        if (type_errors(parser)) {return 0;}

        if (ast->type == AST_EXPRESSION_UNARY_SUB) {
            if (is_unsigned(r)) {
                r = to_signed(lexer->parser->type_table, r);
            }
        }

        result = r;
    } else if (ast->type == AST_EXPRESSION_TYPEOF) {
        str name = ast->statement->type_statement.name;
        Ast_expression *e = ast->statement->type_statement.expression;
        Type_spec *type = 0;
        Symbol *symbol = 0;

        if (e->type == AST_EXPRESSION_MEMBER) {
            type = get_type(lexer, e, false, 0);
        } else if (e->type == AST_EXPRESSION_LITERAL_TYPE) {
            type = get(type_table, name);
        } else {
            assert(e->type == AST_EXPRESSION_NAME, "MAKE ERROR type() argument can only be a member of a struct or a name or a type");
            type = get(type_table, e->name);
            symbol = get(current_scope, e->name);
        }

        if (type_errors(lexer->parser)) {return 0;}

        if (!type && !symbol) {
            report_undeclared_identifier(lexer, name, e->src_p);
            result = 0;
        } else {
            result = get(type_table, STATIC_STR("u32"));
        }
    } else if (ast->type == AST_EXPRESSION_SIZEOF) {
        str name = ast->statement->sizeof_statement.name;
        Ast_expression *e = ast->statement->sizeof_statement.expression;
        Type_spec *type = 0;
        Symbol *symbol = 0;

        if (e->type == AST_EXPRESSION_MEMBER) {
            type = get_type(lexer, e, false, 0);
        } else if (e->type == AST_EXPRESSION_LITERAL_TYPE) {
            type = get(type_table, name);
        } else {
            assert(e->type == AST_EXPRESSION_NAME, "MAKE ERROR sizeof() argument can only be a member of a struct or a name or a type");
            type = get(type_table, e->name);
            symbol = get(current_scope, e->name);
        }

        if (type_errors(lexer->parser)) {return 0;}

        if (!type && !symbol) {
            report_undeclared_identifier(lexer, name, e->src_p);
            result = 0;
        } else {
            result = get(type_table, STATIC_STR("u32"));
        }
    } else if (ast->type == AST_EXPRESSION_OFFSETOF) {
        str type_name = ast->statement->offsetof_statement.type_name;
        str member_name = ast->statement->offsetof_statement.member_name;
        Type_spec *type = get(lexer->parser->type_table, type_name);

        if (type) {
            Type_spec *base = get_base(type);
            Member_info *member_info = get(base->member_info_table, member_name);

            if (member_info) {
                result = get(type_table, STATIC_STR("u32"));
            } else {
                report_member_not_found(lexer, type_name, member_name, ast->statement->offsetof_statement.member_src_p, false);
            }
        } else {
            report_undeclared_identifier(lexer, type_name, ast->statement->offsetof_statement.type_src_p);
        }
    }

    return result;
}

internal Member *get_member(Member *members, str name) {
    Member *result = 0;

    lfor (members) {
        if (equals(it->name, name)) {
            result = it;
            break;
        }
    }

    return result;
}

internal u32 get_compound_size(Type_spec_table *type_table, str type_name, Ast_declaration *ast=0) {
    u32 result = 0;
    bool is_union = ast && (ast->compound->compound_type == COMPOUND_UNION);
    Type_spec *compound = get(type_table, type_name);

    sfor (compound->member_info_table) {
        // TODO: should this loop be made with the members array of the ast_declaration.compound????
        Member_info *inner = *it;

        while (inner) {
            Type_spec *member = get(type_table, inner->type_name);

            if (member->type == TYPE_SPEC_NAME) {
                if (is_union) {
                    result = al_max(result, member->size);
                } else {
                    result += member->size;
                }
            } else if (member->type == TYPE_SPEC_COMPOUND) {
                Member *ast_member = get_member(ast->compound->members, member->name);

                if (is_union) {
                    result = al_max(result, get_compound_size(type_table, member->name));
                } else {
                    result += get_compound_size(type_table, member->name);
                }
            }

            inner = inner->next;
        }
    }

    return result;
}

internal void type_check(Lexer *lexer, Function *function, Ast_declaration *ast_function) {
    Parser *parser = lexer->parser;
    Memory_pool mp = {};
    Symbol_table *new_scope = new_symbol_table(&mp);
    new_scope->next = parser->current_scope;
    parser->current_scope = new_scope;

    sfor_count (function->params, function->param_count) {
        put(parser->current_scope, it->name, it->type->name, it->src_p);
    }

    // NOTE(Juan Antonio) 2022-11-09: false in create_scope because in the case of a function the parameters in the header are in the same scope level as the first level scope inside the function body
    type_check(lexer, function->block, false, ast_function, 0);
    parser->current_scope = parser->current_scope->next;
    clear(&mp);

    if (function->needs_explicit_return && !function->has_explicit_return) {
        report_no_return_for_function(lexer, ast_function);
    }
}

internal void type_check(Lexer *lexer, Ast_declaration *ast) {
    Parser *parser = lexer->parser;
    Symbol_table *current_scope = parser->current_scope;
    Type_spec_table *type_table = parser->type_table;

    switch (ast->type) {
        case AST_DECLARATION_VARIABLE: {
            if (ast->variable.do_inference) {
                // TODO: here we dont pass the l-value to the r-value so if its not clear the type there an error must be returned "ambiguous stuff..."
                ast->variable.variable_type = get_type(lexer, ast->variable.expression, false, 0);
            }

            Symbol *s = get(current_scope, ast->name);

            if (s) {
                // TODO: do something here for variable shadowing maybe in the get function above have a flag bool only_in_current_scope and allow to redeclare variables in deeper scopes just check for redefinitions in the same scope level
                report_variable_redefinition(lexer, ast, s->src_p);
            }

            if (type_errors(parser)) {return;}

            put(current_scope, ast->name, ast->variable.variable_type->name, ast->src_p);

            if (ast->variable.expression) {
                Type_spec *variable = get_type_of_name(lexer, ast->name);
                Type_spec *value = get_type(lexer, ast->variable.expression, true, variable);

                if (type_errors(parser)) {return;}

                if (!any_equals(variable, value)) {
                    report_declaration_missmatch_type_error(lexer, ast, variable, value);
                }
            }
        } break;
        case AST_DECLARATION_OPERATOR: {
            Operator_table *operator_table = parser->operator_table;
            Function *f = ast->_operator.declaration;
            str op1 = f->params[0].type->name;
            str op2 = f->params[1].type->name;
            Operator *op = new_operator(lexer->allocator, ast->_operator.operator_token, op1, op2, f->return_type_name, ast);
            put(operator_table, op);
            type_check(lexer, ast->_operator.declaration, ast);
            if (type_errors(lexer->parser)) {return;}
        } break;
        case AST_DECLARATION_FUNCTION: {
            put(current_scope, ast->name, ast->function->return_type->name, ast->src_p);
            type_check(lexer, ast->function, ast);
            if (type_errors(lexer->parser)) {return;}
        } break;
        case AST_DECLARATION_COMPOUND: {
            Type_spec *compound = get(type_table, ast->name);
            compound->size = get_compound_size(type_table, ast->name, ast);
        } break;
        case AST_DECLARATION_ENUM: {} break;
        case AST_DECLARATION_TYPEDEF: {} break;

        invalid_default_case_msg("ast_declaration type_check missing type");
    }
}

internal void type_check(Lexer *lexer, Ast_statement *ast, Ast_declaration *ast_function, Ast_statement *ast_loop) {
    Type_spec_table *type_table = lexer->parser->type_table;

    switch (ast->type) {
        case AST_STATEMENT_BLOCK: {
            type_check(lexer, ast->block_statement, true, ast_function, ast_loop);
        } break;
        case AST_STATEMENT_IF: {
            type_check(lexer, ast->if_statement, ast_function, ast_loop);
        } break;
        case AST_STATEMENT_LOOP: {
            type_check(lexer, ast->loop_statement, ast_function, ast);
        } break;
        case AST_STATEMENT_EXPRESSION: {
            type_check(lexer, ast->expression_statement, false, 0);
        } break;
        case AST_STATEMENT_DECLARATION: {
            type_check(lexer, ast->declaration_statement);
        } break;
        case AST_STATEMENT_BREAK:
        case AST_STATEMENT_CONTINUE: {
            if (!ast_loop) {
                report_break_continue_outside_loop(lexer, ast);
            } else {
                ast->break_continue_loop = ast_loop;
            }
        } break;
        case AST_STATEMENT_RETURN: {
            ast->return_statement.function = ast_function;
            Function *function;

            if (ast_function->type == AST_DECLARATION_FUNCTION) {
                function = ast_function->function;
            } else {
                assert(ast_function->type == AST_DECLARATION_OPERATOR, "this should never fire, ast_function->type must be FUNCTION or OPERATOR in the return statement");
                function = ast_function->_operator.declaration;
            }

            function->has_explicit_return = true;

            Type_spec *function_return_type = function->return_type;
            Type_spec *return_type;

            if (ast->return_statement.return_expression) {
                // Here with function->return_type parameter we asume that we want to return the function return type for the operator overload
                return_type = get_type(lexer, ast->return_statement.return_expression, true, function->return_type);
            } else {
                return_type = get(type_table, STATIC_STR("void"));
            }

            if (function->needs_explicit_return) {
                // -> name
                if (!equals(return_type, function_return_type)) {
                    report_missmatched_return_types(lexer, ast, function_return_type, return_type);
                }
            } else {
                // -> void
                if (!equals(return_type, function_return_type)) {
                    report_missmatched_return_types(lexer, ast, function_return_type, return_type);
                }
            }
        } break;

        case AST_STATEMENT_SIZEOF: {
            type_check(lexer, ast->sizeof_statement.expression, false, 0);
        } break;
        case AST_STATEMENT_OFFSETOF: {
            type_check(lexer, ast->offsetof_statement.expression, false, 0);
        } break;
        case AST_STATEMENT_TYPEOF: {
            type_check(lexer, ast->type_statement.expression, false, 0);
        } break;

        invalid_default_case_msg("ast_statement type_check missing type");
    }

    if (type_errors(lexer->parser)) {return;}
}

internal void type_check(Lexer *lexer, Ast_expression *ast, bool need_lvalue, Type_spec *l_type) {
    get_type(lexer, ast, need_lvalue, l_type);
}

internal void type_check(Lexer *lexer, Ast_if *ast, Ast_declaration *ast_function, Ast_statement *ast_loop) {
    If *ifs = ast->ifs;

    lfor (ifs) {
        type_check(lexer, &it->condition, true, get(lexer->parser->type_table, STATIC_STR("bool")));
        if (type_errors(lexer->parser)) {return;}

        type_check(lexer, &it->block, true, ast_function, ast_loop);
        if (type_errors(lexer->parser)) {return;}
    }

    if (ast->else_block) {
        type_check(lexer, ast->else_block, true, ast_function, ast_loop);
    }
}

internal void type_check(Lexer *lexer, Ast_loop *ast, Ast_declaration *ast_function, Ast_statement *ast_loop) {
    if (ast->pre) {
        type_check(lexer, ast->pre);
        if (type_errors(lexer->parser)) {return;}
    }

    type_check(lexer, ast->condition, true, get(lexer->parser->type_table, STATIC_STR("bool")));
    if (type_errors(lexer->parser)) {return;}

    if (ast->post) {
        type_check(lexer, ast->post, false, 0);
        if (type_errors(lexer->parser)) {return;}
    }

    type_check(lexer, ast->block, true, ast_function, ast_loop);
}

internal void type_check(Lexer *lexer, Ast_block *ast, bool create_scope, Ast_declaration *ast_function, Ast_statement *ast_loop) {
    // NOTE(Juan Antonio) 2022-11-09: the create_scope parameter is actually for the function parameters to have them in the same scope as the variables in the function block to not allow to shadow the parameters in the first level of the function scope
    Memory_pool mp = {};

    if (create_scope) {
        Symbol_table *new_scope = new_symbol_table(&mp);
        new_scope->next = lexer->parser->current_scope;
        lexer->parser->current_scope = new_scope;
    }

    while (ast) {
        sfor_count (ast->statements, ast->statement_count) {
            type_check(lexer, it, ast_function, ast_loop);
            if (type_errors(lexer->parser)) { return; }
        }

        ast = ast->next;
    }

    if (create_scope) {
        lexer->parser->current_scope = lexer->parser->current_scope->next;
    }

    clear(&mp);
}

internal void type_check(Lexer *lexer, Ast_program *ast) {
    while (ast) {
        sfor_count (ast->declarations, ast->declaration_count) {
            type_check(lexer, it);
            if (type_errors(lexer->parser)) {return;}
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
