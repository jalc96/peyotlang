internal void create_bytecode(Bytecode_generator *generator, Ast_declaration *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_statement *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_expression *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_if *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_loop *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_block *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_program *ast);


internal void create_bytecode(Bytecode_generator *generator, Function *function) {
    sfor_count (function->params, function->param_count) {
    }

    create_bytecode(generator, function->block);
}

internal void create_bytecode(Bytecode_generator *generator, Ast_declaration *ast) {
    Type_spec_table *type_table = generator->type_table;

    switch (ast->type) {
        case AST_DECLARATION_VARIABLE: {
            if (ast->variable.expression) {
                create_bytecode(generator, ast->variable.expression);
            }
        } break;
        case AST_DECLARATION_OPERATOR: {
            create_bytecode(generator, ast->_operator.declaration);
        } break;
        case AST_DECLARATION_FUNCTION: {
            create_bytecode(generator, ast->function);
        } break;
        case AST_DECLARATION_COMPOUND: {} break;
        case AST_DECLARATION_ENUM: {} break;
        case AST_DECLARATION_TYPEDEF: {} break;

        invalid_default_case_msg("ast_declaration create_bytecode missing type");
    }
}

internal void create_bytecode(Bytecode_generator *generator, Ast_statement *ast) {
    Type_spec_table *type_table = generator->type_table;

    switch (ast->type) {
        case AST_STATEMENT_BLOCK: {
            // TODO: current_scope here
            create_bytecode(generator, ast->block_statement.block/*, here*/);
        } break;
        case AST_STATEMENT_IF: {
            create_bytecode(generator, ast->if_statement);
        } break;
        case AST_STATEMENT_LOOP: {
            create_bytecode(generator, ast->loop_statement);
        } break;
        case AST_STATEMENT_EXPRESSION: {
            create_bytecode(generator, ast->expression_statement);
        } break;
        case AST_STATEMENT_DECLARATION: {
            create_bytecode(generator, ast->declaration_statement);
        } break;
        case AST_STATEMENT_BREAK:
        case AST_STATEMENT_CONTINUE: {
        } break;
        case AST_STATEMENT_RETURN: {
        } break;

        case AST_STATEMENT_SIZEOF: {
            create_bytecode(generator, ast->sizeof_statement.expression);
        } break;
        case AST_STATEMENT_OFFSETOF: {
            create_bytecode(generator, ast->offsetof_statement.expression);
        } break;
        case AST_STATEMENT_TYPEOF: {
            create_bytecode(generator, ast->type_statement.expression);
        } break;

        invalid_default_case_msg("ast_statement create_bytecode missing type");
    }
}

internal void create_bytecode(Bytecode_generator *generator, Ast_expression *ast) {
    // Symbol_table *current_scope;
    Type_spec_table *type_table = generator->type_table;

    if (is_leaf(ast->type)) {
        switch (ast->type) {
            case AST_EXPRESSION_LITERAL_TYPE: {
            } break;
            case AST_EXPRESSION_LITERAL_CHAR: {
            } break;
            case AST_EXPRESSION_LITERAL_STR: {
                // TODO: add to the string pool
            } break;
            case AST_EXPRESSION_LITERAL_INTEGER: {
            } break;
            case AST_EXPRESSION_LITERAL_FLOAT: {
            } break;
            case AST_EXPRESSION_NAME: {
                if (get(type_table, ast->name)) {
                    // is a type
                    get(type_table, STATIC_STR("u32"));
                } else {
                    // is a name
                    // get_type_of_name(current_scope, type_table, ast->name);
                }
            } break;
            case AST_EXPRESSION_MEMBER: {
                // Symbol *symbol = get(current_scope, ast->name);
                // Type_spec *type = get(type_table, symbol->type_name);
                // assert(type, "compiler error: type not found in bytecode generation");

                // Type_spec *base = get_base(type);
                // Member_info *member_info = get(base->member_info_table, ast->binary.right->name);
                // assert(member_info, "compiler error: member_info not found in bytecode generation");

                // get(type_table, member_info->type_name);
            } break;
            case AST_EXPRESSION_FUNCTION_CALL: {} break;
        }
    } else if (is_binary(ast->type)) {
        create_bytecode(generator, ast->binary.left);
        create_bytecode(generator, ast->binary.right);

        if (is_arithmetic(ast->type) || is_relational(ast->type)) {
            // Operator *op = get(operator_table, to_op_token_type(ast->type), l->name, r->name, op_type->name);
            // assert(op, "compiler error: operator not found in bytecode generation");
        } else if (is_assignment(ast->type)) {
        } else if (is_boolean(ast->type)) {
        } else if (is_bit_operator(ast->type)) {
        }
    } else if (is_unary(ast->type)) {
    } else if (ast->type == AST_EXPRESSION_TYPEOF) {
        Ast_expression *e = ast->statement->type_statement.expression;
        Type_spec *type = 0;
        Symbol *symbol = 0;

        if (e->type == AST_EXPRESSION_MEMBER) {
            create_bytecode(generator, e);
        } else if (e->type == AST_EXPRESSION_LITERAL_TYPE) {
            // type = get(type_table, name);
        } else {
            assert(e->type == AST_EXPRESSION_NAME, "MAKE ERROR type() argument can only be a member of a struct or a name or a type");
            type = get(type_table, e->name);
            // symbol = get(current_scope, e->name);
        }

        get(type_table, STATIC_STR("u32"));
    } else if (ast->type == AST_EXPRESSION_SIZEOF) {
        str name = ast->statement->sizeof_statement.name;
        Ast_expression *e = ast->statement->sizeof_statement.expression;
        Type_spec *type = 0;
        Symbol *symbol = 0;

        if (e->type == AST_EXPRESSION_MEMBER) {
            create_bytecode(generator, e);
        } else if (e->type == AST_EXPRESSION_LITERAL_TYPE) {
            type = get(type_table, name);
        } else {
            assert(e->type == AST_EXPRESSION_NAME, "MAKE ERROR sizeof() argument can only be a member of a struct or a name or a type");
            type = get(type_table, e->name);
            // symbol = get(current_scope, e->name);
        }

        get(type_table, STATIC_STR("u32"));
    } else if (ast->type == AST_EXPRESSION_OFFSETOF) {
        str type_name = ast->statement->offsetof_statement.type_name;
        str member_name = ast->statement->offsetof_statement.member_name;
        Type_spec *type = get(type_table, type_name);
        assert(type, "compiler error: type not found in bytecode generation")

        Type_spec *base = get_base(type);
        Member_info *member_info = get(base->member_info_table, member_name);

        get(type_table, STATIC_STR("u32"));
    }
}

internal void create_bytecode(Bytecode_generator *generator, Ast_if *ast) {
    If *ifs = ast->ifs;

    lfor (ifs) {
        create_bytecode(generator, &it->condition);

        create_bytecode(generator, &it->block);
    }

    if (ast->else_block) {
        create_bytecode(generator, ast->else_block);
    }
}

internal void create_bytecode(Bytecode_generator *generator, Ast_loop *ast) {
    if (ast->pre) {
        create_bytecode(generator, ast->pre);
    }

    create_bytecode(generator, ast->condition);

    if (ast->post) {
        create_bytecode(generator, ast->post);
    }

    create_bytecode(generator, ast->block);
}

internal void create_bytecode(Bytecode_generator *generator, Ast_block *ast) {
    while (ast) {
        sfor_count (ast->statements, ast->statement_count) {
            create_bytecode(generator, it);
        }

        ast = ast->next;
    }
}

internal void create_bytecode(Bytecode_generator *generator, Ast_program *ast) {
    while (ast) {
        sfor_count (ast->declarations, ast->declaration_count) {
            create_bytecode(generator, it);
        }

        ast = ast->next;
    }
}