enum BYTECODE_RESULT_TYPE {
    EB_NULL,

    E_LITERAL,
    E_REGISTER,
    E_MEMORY,

    EB_COUNT,
};

struct Bytecode_result {
    BYTECODE_RESULT_TYPE type;
    bool comparison_needed;
    u32 size;

    union {
        u64 _u64;
        u32 r;
        Address _address;
    };
};

internal Bytecode_result new_expression_bytecode_result(u64 _u64) {
    Bytecode_result result;
    result.type = E_LITERAL;
    result.comparison_needed = true;
    result._u64 = _u64;
    return result;
}

internal Bytecode_result new_expression_bytecode_literal(u64 _u64) {
    Bytecode_result result;
    result.type = E_LITERAL;
    result.comparison_needed = true;
    result._u64 = _u64;
    return result;
}

internal Bytecode_result new_expression_bytecode_result(u32 r) {
    Bytecode_result result;
    result.type = E_REGISTER;
    result.comparison_needed = true;
    result.r = r;
    return result;
}

internal Bytecode_result new_expression_bytecode_result(Address _address) {
    Bytecode_result result;
    result.type = E_MEMORY;
    result.comparison_needed = true;
    result._address = _address;
    return result;
}

// TODO: address is probably deprecated
internal void create_bytecode(Bytecode_generator *generator, Ast_declaration *ast);
internal void create_bytecode(Bytecode_generator *generator, Ast_statement *ast);
internal Bytecode_result create_bytecode(Bytecode_generator *generator, Ast_expression *ast);
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

internal void emit_nop(Bytecode_generator *generator) {
    Bytecode_instruction *result = next(generator);
    result->instruction = NOP;
}

internal void emit_return(Bytecode_generator *generator) {
    Bytecode_instruction *result = next(generator);
    result->instruction = RET;
}

internal void emit_op(Bytecode_generator *generator, BYTECODE_INSTRUCTION op, u32 r1, u32 r2) {
    Bytecode_instruction *result = next(generator);
    result->instruction = op;
    result->destination = new_register_operand(r1);
    result->source = new_register_operand(r2);
}

internal void emit_call(Bytecode_generator *generator, str *function_name) {
    Bytecode_instruction *result = next(generator);
    result->instruction = CALL;
    result->destination = new_operand(FUNCTION_NAME, function_name);
}

internal void emit_tag(Bytecode_generator *generator, Tag tag) {
    u32 bytecode_offset = generator->bytecode_head;
    Bytecode_instruction *result = next(generator);
    result->instruction = TAG;
    Tag_offset *to = new_tag_offset(generator->allocator, tag.id, bytecode_offset);
    put(generator->tag_offset_table, to);
    result->destination = new_operand(TAG_ID, tag);
}

internal void emit_function_tag(Bytecode_generator *generator, str *function_name) {
    u32 bytecode_offset = generator->bytecode_head;
    Bytecode_instruction *result = next(generator);
    result->instruction = FTAG;
    Function_offset *function = new_function_offset(generator->allocator, function_name, bytecode_offset);
    put(generator->function_offset_table, function);
    result->destination = new_operand(FUNCTION_NAME, function_name);
}

internal void emit_jump(Bytecode_generator *generator, Tag tag, bool _equals, bool just_jump=false) {
    Bytecode_instruction *result = next(generator);
    result->instruction = just_jump ? JUMP :_equals ? JEQ : JNEQ;
    result->destination = new_operand(TAG_ID, tag);
}

internal void create_bytecode(Bytecode_generator *generator, Function *function) {
    // Here we dont care if the function header variables are in a higher scope than the function body variables, the typecheck is already done
    function->current_scope = generator->current_scope;

    sfor_count (function->params, function->param_count) {
        // TODO: pop the params out of the stack and put them into the symbol table
        u64 address = push_stack(generator, it->type->size);
        put(function->current_scope, it->name, it->type->name, address);
    }

    create_bytecode(generator, function->block);

    emit_nop(generator);
    emit_nop(generator);
    emit_nop(generator);
    emit_nop(generator);
    emit_nop(generator);
    emit_nop(generator);
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
                Bytecode_result value = create_bytecode(generator, ast->variable.expression);
                Address dst = new_address(RBP, (s32)address);
                emit_mov_to_address(generator, dst, value);
                print(current_scope, true);
            }
        } break;
        case AST_DECLARATION_OPERATOR: {
            create_bytecode(generator, ast->_operator.declaration);
        } break;
        case AST_DECLARATION_FUNCTION: {
            u32 parenthesis = 2;
            u32 name_size = length(ast->name) + parenthesis;
            u32 comma = 1;
            Parameter *params = ast->function->params;

            lfor (params) {
                Type_spec *pt = it->type;
                name_size += length(pt->name) + comma;
            }

            // subtract last comma
            name_size -= comma;
            str *function_name = new_string(name_size);
            u32 index = 0;

            str_for (ast->name) {
                function_name->buffer[index++] = it;
            }

            function_name->buffer[index++] = '(';

            {
                lfor (params) {
                    Type_spec *pt = it->type;
                    {
                        str_for (pt->name) {
                            function_name->buffer[index++] = it;
                        }
                    }

                    if (it->next) {
                        function_name->buffer[index++] = ',';
                    }
                }
            }

            function_name->buffer[index++] = ')';

            emit_function_tag(generator, function_name);

            push_stack_call(generator);
            push_new_scope(generator->allocator, &generator->current_scope);
            create_bytecode(generator, ast->function);
            pop_scope(&generator->current_scope);
            pop_stack_call(generator);
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
            create_bytecode(generator, ast->expression_statement);
        } break;
        case AST_STATEMENT_DECLARATION: {
            create_bytecode(generator, ast->declaration_statement);
        } break;
        case AST_STATEMENT_BREAK:{
            emit_jump(generator, *ast->break_continue_loop->loop_statement->end_tag, false, true);
        } break;
        case AST_STATEMENT_CONTINUE: {
            emit_jump(generator, *ast->break_continue_loop->loop_statement->post_tag, false, true);
        } break;
        case AST_STATEMENT_RETURN: {
            Bytecode_result re = create_bytecode(generator, ast->return_statement.return_expression);
            emit_mov_to_register(generator, 0, re);
            emit_return(generator);
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

internal Bytecode_result create_bytecode(Bytecode_generator *generator, Ast_expression *ast) {
    Bytecode_result result = {};
    result.comparison_needed = true;
    Symbol_table *current_scope = generator->current_scope;
    Type_spec_table *type_table = generator->type_table;

    if (is_leaf(ast->type)) {
        switch (ast->type) {
            case AST_EXPRESSION_LITERAL_TYPE: {
                Type_spec *t = get(type_table, STATIC_STR("u32"));
                result.type = E_LITERAL;
                result._u64 = t->id;
                result.size = t->size;
            } break;
            case AST_EXPRESSION_LITERAL_CHAR: {
                result.type = E_LITERAL;
                result._u64 = ast->u64_value;
                result.size = 1;
            } break;
            case AST_EXPRESSION_LITERAL_STR: {
                // TODO: add to the string pool
            } break;
            case AST_EXPRESSION_LITERAL_INTEGER: {
                result.type = E_LITERAL;
                result._u64 = ast->u64_value;
                result.size = 4;
            } break;
            case AST_EXPRESSION_LITERAL_FLOAT: {
                result.type = E_LITERAL;
                // its just bytes
                result._u64 = ast->u64_value;
                result.size = 4;
            } break;
            case AST_EXPRESSION_NAME: {
                if (get(type_table, ast->name)) {
                    // is a type
                    // TODO: when does this happen??
                    Type_spec *t = get(type_table, STATIC_STR("u32"));
                    result.type = E_LITERAL;
                    result._u64 = t->id;
                    result.size = t->size;
                } else {
                    // is a name
                    Symbol *s = get(generator->current_scope, ast->name);
                    Address _address = new_address(RBP, s->stack_offset);

                    result.type = E_MEMORY;
                    result._address = _address;
                    Type_spec *t = get(type_table, s->type_name);
                    result.size = t->size;
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
            case AST_EXPRESSION_FUNCTION_CALL: {
                Call_parameter *params = ast->function_call.parameter;
                u32 parenthesis = 2;
                u32 name_size = length(ast->name) + parenthesis;
                u32 comma = 1;

                lfor (params) {
                    Type_spec *pt = it->parameter->op_type;
                    name_size += length(pt->name) + comma;

                    Bytecode_result param = create_bytecode(generator, it->parameter);
                    u64 address = push_stack(generator, param.size);
                    Address dst = new_address(RBP, (s32)address);
                    emit_mov_to_address(generator, dst, param);
                }

                // Generate the call name in place
                // subtract last comma
                name_size -= comma;
                str *call_name = new_string(name_size);
                u32 index = 0;

                str_for (ast->name) {
                    call_name->buffer[index++] = it;
                }

                call_name->buffer[index++] = '(';

                {
                    lfor (params) {
                        Type_spec *pt = it->parameter->op_type;
                        {
                            str_for (pt->name) {
                                call_name->buffer[index++] = it;
                            }
                        }

                        if (it->next) {
                            call_name->buffer[index++] = ',';
                        }
                    }
                }

                call_name->buffer[index++] = ')';
                // Generate the call name in place


                emit_call(generator, call_name);
            } break;
        }
    } else if (is_binary(ast->type)) {
        Type_spec *l_type = ast->binary.left->op_type;
        Type_spec *r_type = ast->binary.right->op_type;

        if (is_assignment(ast->type)) {
            Bytecode_result r = create_bytecode(generator, ast->binary.right);

            Symbol *s = get(generator->current_scope, ast->binary.left->name);
            Address dst = new_address(RBP, s->stack_offset);
            emit_mov_to_address(generator, dst, r);
        } else {
            Bytecode_result l = create_bytecode(generator, ast->binary.left);
            u32 r1 = new_register(generator);
            emit_mov_to_register(generator, r1, l);

            Bytecode_result r = create_bytecode(generator, ast->binary.right);
            u32 r2 = new_register(generator);
            emit_mov_to_register(generator, r2, r);

            result.type = E_REGISTER;
            Native_types_op *native = get(generator->native_operations_table, to_op_token_type(ast->type), l_type->name, r_type->name);

            if (native) {
                bool is_float = equals(native->return_type, STATIC_STR("f32"));
                BYTECODE_INSTRUCTION instruction = ast_binary_type_to_bytecode_instruction(ast->type, is_float);
                emit_op(generator, instruction, r1, r2);
                result.r = r1;
            } else {
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
                emit_call(generator, operator_name);
                result.r = 1;

                assert(op, "compiler error: operator not found in bytecode generation");
            }
            // TODO: for comparisons maybe have a flag bitfield to check in the jumps
            if (is_arithmetic(ast->type)) {
                result.comparison_needed = true;
            } else if (is_relational(ast->type)) {
                result.comparison_needed = false;
            } else if (is_boolean(ast->type)) {
                result.comparison_needed = false;
            } else if (is_bit_operator(ast->type)) {
                result.comparison_needed = true;
            }
        }

        result.size = ast->op_type->size;
    } else if (is_unary(ast->type)) {
        result.comparison_needed = true;
        result.size = ast->op_type->size;
    } else if (ast->type == AST_EXPRESSION_TYPEOF) {
        Ast_expression *e = ast->statement->type_statement.expression;
        str name = e->name;
        Type_spec *type = 0;
        Symbol *symbol = 0;
        // assert(type, "in bytecode generation the ast type must be set for typeof() directive");

        if (e->type == AST_EXPRESSION_MEMBER) {
            // create_bytecode(generator, e);
        } else if (e->type == AST_EXPRESSION_LITERAL_TYPE) {
            type = get(type_table, name);
        } else {
            assert(e->type == AST_EXPRESSION_NAME, "MAKE ERROR type() argument can only be a member of a struct or a name or a type");
            type = get(type_table, e->name);

            if (!type) {
                symbol = get(current_scope, name);
                type = get(type_table, symbol->type_name);
                assert(type, "missing type in bytecode generation");
            }
        }

        result.type = E_LITERAL;
        result._u64 = type->id;
        result.size = 4;
    } else if (ast->type == AST_EXPRESSION_SIZEOF) {
        Ast_expression *e = ast->statement->sizeof_statement.expression;
        str name = e->name;
        Type_spec *type = 0;
        Symbol *symbol = 0;
        // assert(type, "in bytecode generation the ast type must be set for typeof() directive");

        if (e->type == AST_EXPRESSION_MEMBER) {
            // create_bytecode(generator, e);
        } else if (e->type == AST_EXPRESSION_LITERAL_TYPE) {
            type = get(type_table, name);
        } else {
            assert(e->type == AST_EXPRESSION_NAME, "MAKE ERROR type() argument can only be a member of a struct or a name or a type");
            type = get(type_table, e->name);

            if (!type) {
                symbol = get(current_scope, name);
                type = get(type_table, symbol->type_name);
                assert(type, "missing type in bytecode generation");
            }
        }

        result.type = E_LITERAL;
        result._u64 = type->size;
        result.size = 4;
    } else if (ast->type == AST_EXPRESSION_OFFSETOF) {
        str type_name = ast->statement->offsetof_statement.type_name;
        str member_name = ast->statement->offsetof_statement.member_name;
        Type_spec *type = get(type_table, type_name);
        assert(type, "compiler error: type not found in bytecode generation")

        Type_spec *base = get_base(type);
        Member_info *member_info = get(base->member_info_table, member_name);

        get(type_table, STATIC_STR("u32"));
        result.size = 4;
    }

    return result;
}

internal void create_bytecode(Bytecode_generator *generator, Ast_if *ast) {
    If *ifs = ast->ifs;
    bool needs_end_jump = (
           (bool)ast->else_block
        || (bool)ifs->next
    );

    Tag end_tag = {};

    if (needs_end_jump) {
        end_tag = new_tag(generator);
    }

    lfor (ifs) {
        Tag tag = new_tag(generator);
        Bytecode_result check = create_bytecode(generator, it->condition);

        if (check.comparison_needed) {
            u32 r1 = new_register(generator);
            emit_mov_to_register(generator, r1, check);

            u32 r2 = new_register(generator);
            Bytecode_result zero =  new_expression_bytecode_literal(0);
            emit_mov_to_register(generator, r2, zero);

            emit_op(generator, EQ, r1, r2);
            emit_jump(generator, tag, true);
        } else {
            emit_jump(generator, tag, false);
        }

        create_bytecode(generator, &it->block);

        if (needs_end_jump) {
            emit_jump(generator, end_tag, false, true);
        }

        emit_tag(generator, tag);
    }

    if (ast->else_block) {
        create_bytecode(generator, ast->else_block);
    }

    if (needs_end_jump) {
        emit_tag(generator, end_tag);
    }
}

internal void create_bytecode(Bytecode_generator *generator, Ast_loop *ast) {
    push_new_scope(generator->allocator, &generator->current_scope);

    if (ast->pre) {
        create_bytecode(generator, ast->pre);
    }

    Tag *loop_tag = new_tag_alloc(generator);
    Tag *end_tag = new_tag_alloc(generator);
    Tag *post_tag = new_tag_alloc(generator);
    ast->loop_tag = loop_tag;
    ast->end_tag = end_tag;
    ast->post_tag = post_tag;

    emit_tag(generator, *loop_tag);
    Bytecode_result check = create_bytecode(generator, ast->condition);

    if (check.comparison_needed) {
        u32 r1 = new_register(generator);
        emit_mov_to_register(generator, r1, check);

        u32 r2 = new_register(generator);
        Bytecode_result zero =  new_expression_bytecode_literal(0);
        emit_mov_to_register(generator, r2, zero);

        emit_op(generator, EQ, r1, r2);
        emit_jump(generator, *end_tag, true);
    } else {
        emit_jump(generator, *end_tag, false);
    }

    create_bytecode(generator, ast->block);

    if (ast->post) {
        emit_tag(generator, *post_tag);
        create_bytecode(generator, ast->post);
    } else {
        // TODO: is this correct?
        ast->post_tag = ast->end_tag;
    }

    emit_jump(generator, *loop_tag, false, true);
    emit_tag(generator, *end_tag);

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