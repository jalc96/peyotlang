struct Virtual_machine {
    Memory_pool *allocator;
    Tag_offset_hash_table *tag_offset_table;
    Function_offset_hash_table *function_offset_table;

    Regrowable_linear_buffer<u8> *string_pool;
    Regrowable_linear_buffer<Bytecode_instruction> *bytecode;

    u32 program_counter;
    bool check_is_true;

    u8 *stack;
    u64 registers[REGISTER_COUNT];
};

internal u32 get_main_offset(Function_offset_hash_table *table) {
    u32 result = 0;

    sfor(table->table) {
        Function_offset *function = *it;

        lfor(function) {
            Split_iterator _ = split(*it->name, STATIC_STR("("));
            str name_part = _.sub_str;

            if (equals(name_part, STATIC_STR("main"))) {
                result = it->bytecode_offset;
            }
        }
    }

    return result;
}

internal Virtual_machine *new_virtual_machine(Memory_pool *allocator, Tag_offset_hash_table *tag_offset_table, Function_offset_hash_table *function_offset_table, Regrowable_linear_buffer<u8> *string_pool, Regrowable_linear_buffer<Bytecode_instruction> *bytecode, u32 stack_size) {
    Virtual_machine *result = push_struct(allocator, Virtual_machine);

    result->allocator = allocator;
    result->tag_offset_table = tag_offset_table;
    result->function_offset_table = function_offset_table;
    result->string_pool = string_pool;
    result->program_counter = get_main_offset(function_offset_table);
    result->bytecode = bytecode;
    result->stack = push_array(allocator, u8, stack_size);

    return result;
}

internal u32 log10(u32 x) {
    u32 result = 0;

    while (x > 10) {
        x = x / 10;
        result++;
    }

    return result;
}

internal void print_registers(Virtual_machine *vm) {
    for (u32 i = R0; i < REGISTER_COUNT; i++) {
        printf("r%u ", i - R0);
    }
    putchar('\n');
    for (u32 i = R0; i < REGISTER_COUNT; i++) {
        print_indent(1 + log10(i-R0) - log10(vm->registers[i]));
        printf("%llu ", vm->registers[i]);
    }
    putchar('\n');
}
