enum BYTECODE_RESULT_TYPE {
    EB_NULL,

    E_LITERAL,
    E_REGISTER,
    E_MEMORY,

    EB_COUNT,
};

struct Bytecode_result {
    BYTECODE_RESULT_TYPE type;

    union {
        u64 _u64;
        u32 r;
        Address _address;
    };
};

internal Bytecode_result new_expression_bytecode_result(u64 _u64) {
    Bytecode_result result;
    result.type = E_LITERAL;
    result._u64 = _u64;
    return result;
}

internal Bytecode_result new_expression_bytecode_result(u32 r) {
    Bytecode_result result;
    result.type = E_REGISTER;
    result.r = r;
    return result;
}

internal Bytecode_result new_expression_bytecode_result(Address _address) {
    Bytecode_result result;
    result.type = E_MEMORY;
    result._address = _address;
    return result;
}

// TODO: address is probably deprecated
internal void create_bytecode(Bytecode_generator *generator, Ast_declaration *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_statement *ast);
internal Bytecode_result create_bytecode(Bytecode_generator *generator, Ast_expression *ast, u64 address);
internal void create_bytecode(Bytecode_generator *generator, Ast_if *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_loop *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_block *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_program *ast);

internal void emit_load_value_to_stack(Bytecode_generator *generator, u64 address, u64 value) {
    // Bytecode_instruction *push_stack = next(generator);
    // push_stack->instruction = MOVI;
    // push_stack->destination._address = address;
    // push_stack->source._u64 = value;
}

internal void emit_load_value_to_register_from_memory(Bytecode_generator *generator, u32 r, u64 address) {
    // Bytecode_instruction *result = next(generator);
    // result->instruction = MOVM;
    // result->destination.r = r;
    // result->source._address = address;
}

internal void emit_mov_to_address(Bytecode_generator *generator, Address dst, Bytecode_result src) {
    Bytecode_instruction *result = next(generator);
    result->instruction = MOVR;
    result->destination = new_operand(ADDRESS, dst);

    if (src.type == E_LITERAL) {
        result->source = new_operand(_QWORD, src._u64);
    } else {
        result->source = new_operand(REGISTER_ID, src.r);
    }
}

internal void emit_mov_to_register(Bytecode_generator *generator, u32 dst, Bytecode_result src) {
    if (src.type == E_REGISTER) {
        // Avoid MOVR R1, R1
        if (dst == src.r) {
            // TODO: make this check elsewhere???
            return;
        }
    }

    Bytecode_instruction *result = next(generator);
    result->destination = new_operand(REGISTER_ID, dst);

    if (src.type == E_LITERAL) {
        result->instruction = MOVI;
        result->source = new_operand(_QWORD, src._u64);
    } else if (src.type == E_MEMORY) {
        result->instruction = MOVM;
        result->source = new_operand(ADDRESS, src._address);
    } else {
        result->instruction = MOVR;
        result->source = new_operand(REGISTER_ID, src.r);
    }
}

internal void emit_op(Bytecode_generator *generator, BYTECODE_INSTRUCTION op, u32 r1, u32 r2) {
    Bytecode_instruction *result = next(generator);
    result->instruction = op;
    result->destination = new_register_operand(r1);
    result->source = new_register_operand(r2);
}

internal void emit_call(Bytecode_generator *generator, str *function_name) {
    // TODO: probably put here the push and pop stuff for the program counter when a function call happens
    Bytecode_instruction *result = next(generator);
    result->instruction = CALL;
    result->destination = new_operand(FUNCTION_NAME, function_name);
}

internal void create_bytecode(Bytecode_generator *generator, Function *function) {
    sfor_count (function->params, function->param_count) {
    }

    create_bytecode(generator, function->block);
}

internal void create_bytecode(Bytecode_generator *generator, Ast_declaration *ast) {
    Type_spec_table *type_table = generator->type_table;
    Symbol_table *current_scope = generator->current_scope;

    switch (ast->type) {
        case AST_DECLARATION_VARIABLE: {
            Type_spec *type = ast->variable.variable_type;
            u64 address = push_stack(generator, type->size);
            put(current_scope, ast->name, type->name, address);

            if (ast->variable.expression) {
                Bytecode_result value = create_bytecode(generator, ast->variable.expression, address);
                Address dst = new_address(RBP, (s32)address);
                emit_mov_to_address(generator, dst, value);
                print(current_scope, true);
            }
        } break;
        case AST_DECLARATION_OPERATOR: {
            create_bytecode(generator, ast->_operator.declaration);
        } break;
        case AST_DECLARATION_FUNCTION: {
            push_new_scope(generator->allocator, &generator->current_scope);
            create_bytecode(generator, ast->function);
            pop_scope(&generator->current_scope);
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
            // u64 address = push_stack(generator, 8);
            create_bytecode(generator, ast->expression_statement, 0);
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
            // create_bytecode(generator, ast->sizeof_statement.expression);
        } break;
        case AST_STATEMENT_OFFSETOF: {
            // create_bytecode(generator, ast->offsetof_statement.expression);
        } break;
        case AST_STATEMENT_TYPEOF: {
            // create_bytecode(generator, ast->type_statement.expression);
        } break;

        invalid_default_case_msg("ast_statement create_bytecode missing type");
    }
}

internal Bytecode_result create_bytecode(Bytecode_generator *generator, Ast_expression *ast, u64 address) {
    Bytecode_result result = {};
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
                result.type = E_LITERAL;
                result._u64 = ast->u64_value;
            } break;
            case AST_EXPRESSION_LITERAL_FLOAT: {
                // emit_int(generator, ast->u64_value);
            } break;
            case AST_EXPRESSION_NAME: {
                if (get(type_table, ast->name)) {
                    // is a type
                    get(type_table, STATIC_STR("u32"));
                } else {
                    // is a name
                    Symbol *s = get(generator->current_scope, ast->name);
                    Address _address = new_address(RBP, s->stack_offset);

                    result.type = E_MEMORY;
                    result._address = _address;
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
        if (is_assignment(ast->type)) {
            Bytecode_result r = create_bytecode(generator, ast->binary.right, 0);

            Symbol *s = get(generator->current_scope, ast->binary.left->name);
            Address dst = new_address(RBP, s->stack_offset);
            emit_mov_to_address(generator, dst, r);
        } else {
            Bytecode_result l = create_bytecode(generator, ast->binary.left, 0);
            u32 r1 = l.r;

            r1 = new_register(generator);
            emit_mov_to_register(generator, r1, l);


            Bytecode_result r = create_bytecode(generator, ast->binary.right, 0);
            u32 r2 = r.r;

            r2 = new_register(generator);
            emit_mov_to_register(generator, r2, r);

            result.type = E_REGISTER;

            if (is_arithmetic(ast->type)) {
                // BYTECODE_INSTRUCTION instruction = ast_type_to_arithmetic_bytecode_instruction(ast->type);
                // emit_op(generator, instruction, r1, r2);
                push_stack_call(generator);
                {
                    u64 size_1 = ast->binary.left->op_type->size;
                    u64 address = push_stack(generator, size_1);
                    Address dst1 = new_address(RBP, (s32)address);
                    Bytecode_result v1 = new_expression_bytecode_result(r1);
                    emit_mov_to_address(generator, dst1, v1);

                    u64 size_2 = ast->binary.right->op_type->size;
                    address = push_stack(generator, size_2);
                    Address dst2 = new_address(RBP, (s32)address);
                    Bytecode_result v2 = new_expression_bytecode_result(r2);
                    emit_mov_to_address(generator, dst2, v2);
                }
                pop_stack_call(generator);

                Type_spec *lt = ast->binary.left->op_type;
                Type_spec *rt = ast->binary.right->op_type;
                Operator *op = get(generator->operator_table, to_op_token_type(ast->type), lt->name, rt->name);
                str *operator_name = to_call_string(op);
                debug(*operator_name)
                emit_call(generator, operator_name);
                result.r = 1;


                // assert(op, "compiler error: operator not found in bytecode generation");

                // TODO: for comparisons maybe have a flag bitfield to check in the jumps
            // } else if (is_relational(ast->type)) {
            // } else if (is_boolean(ast->type)) {
            // } else if (is_bit_operator(ast->type)) {
            }
        }
    } else if (is_unary(ast->type)) {
    } else if (ast->type == AST_EXPRESSION_TYPEOF) {
        Ast_expression *e = ast->statement->type_statement.expression;
        Type_spec *type = e->op_type;
        assert(type, "in bytecode generation the ast type must be set for typeof() directive");

        if (e->type == AST_EXPRESSION_MEMBER) {
            // create_bytecode(generator, e);
        } else if (e->type == AST_EXPRESSION_LITERAL_TYPE) {
            // type = get(type_table, name);
        } else {
            assert(e->type == AST_EXPRESSION_NAME, "MAKE ERROR type() argument can only be a member of a struct or a name or a type");
            // type = get(type_table, e->name);
        }

        result.type = E_LITERAL;
        result._u64 = type->id;
    } else if (ast->type == AST_EXPRESSION_SIZEOF) {
        Ast_expression *e = ast->statement->sizeof_statement.expression;
        Type_spec *type = e->op_type;
        assert(type, "in bytecode generation the ast type must be set for sizeof() directive");

        if (e->type == AST_EXPRESSION_MEMBER) {
            // create_bytecode(generator, e);
        } else if (e->type == AST_EXPRESSION_LITERAL_TYPE) {
            // type = get(type_table, name);
        } else {
            assert(e->type == AST_EXPRESSION_NAME, "MAKE ERROR sizeof() argument can only be a member of a struct or a name or a type");
            // type = get(type_table, e->name);
        }

        result.type = E_LITERAL;
        result._u64 = type->size;
    } else if (ast->type == AST_EXPRESSION_OFFSETOF) {
        str type_name = ast->statement->offsetof_statement.type_name;
        str member_name = ast->statement->offsetof_statement.member_name;
        Type_spec *type = get(type_table, type_name);
        assert(type, "compiler error: type not found in bytecode generation")

        Type_spec *base = get_base(type);
        Member_info *member_info = get(base->member_info_table, member_name);

        get(type_table, STATIC_STR("u32"));
    }

    return result;
}

internal void create_bytecode(Bytecode_generator *generator, Ast_if *ast) {
    If *ifs = ast->ifs;

    lfor (ifs) {
        // create_bytecode(generator, &it->condition);

        // create_bytecode(generator, &it->block);
    }

    if (ast->else_block) {
        // create_bytecode(generator, ast->else_block);
    }
}

internal void create_bytecode(Bytecode_generator *generator, Ast_loop *ast) {
    push_new_scope(generator->allocator, &generator->current_scope);

    if (ast->pre) {
        create_bytecode(generator, ast->pre);
    }

    // create_bytecode(generator, ast->condition);

    if (ast->post) {
        // create_bytecode(generator, ast->post);
    }

    create_bytecode(generator, ast->block);
    pop_scope(&generator->current_scope);
}

internal void create_bytecode(Bytecode_generator *generator, Ast_block *ast) {
    // TODO: maybe add the create_scope parameter like in the type checker??
    push_new_scope(generator->allocator, &generator->current_scope);

    while (ast) {
        sfor_count (ast->statements, ast->statement_count) {
            create_bytecode(generator, it);
        }

        ast = ast->next;
    }

    pop_scope(&generator->current_scope);
}

internal void create_bytecode(Bytecode_generator *generator, Ast_program *ast) {
    while (ast) {
        sfor_count (ast->declarations, ast->declaration_count) {
            create_bytecode(generator, it);
        }

        ast = ast->next;
    }
}