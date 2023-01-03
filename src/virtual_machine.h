struct Call_stack {
    u64 rbp;
    u64 rsp;
    u32 program_counter;
    Call_stack *next;
};

struct Virtual_machine {
    Memory_pool *allocator;
    Tag_offset_hash_table *tag_offset_table;
    Function_offset_hash_table *function_offset_table;

    Regrowable_linear_buffer<u8> *string_pool;
    Regrowable_linear_buffer<Bytecode_instruction> *bytecode;

    u32 program_counter;
    bool check_is_true;

    Call_stack *call_stack;
    Call_stack *call_stack_first_free;

    u8 *stack;
    u64 registers[REGISTER_COUNT];
};

internal void push(Virtual_machine *vm, u64 rbp, u64 rsp, u32 program_counter) {
    Call_stack *item = vm->call_stack_first_free;

    if (!item) {
        item = push_struct(vm->allocator, Call_stack);
    }

    item->rbp = rbp;
    item->rsp = rsp;
    item->program_counter = program_counter;
    item->next = vm->call_stack;
    vm->call_stack = item;
}

internal Call_stack *pop(Virtual_machine *vm) {
    // This is going to be used inmediatly after poping it an then its not needed so it doesnt matter if its in the free list
    Call_stack *result = vm->call_stack;

    if (result) {
        vm->call_stack = result->next;
        result->next = vm->call_stack_first_free;
        vm->call_stack_first_free = result;
    }

    return result;
}

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

internal void print_stack(Virtual_machine *vm, u64 from, u64 to) {
    if (!from) return;

    u32 *to_print = (u32 *)(from);

    while ((u64)to_print <= to) {
        printf("%u|", *to_print);
        to_print++;
    }

    putchar('\n');
}
