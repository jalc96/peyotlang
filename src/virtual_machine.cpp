internal u32 get_register_index(Virtual_machine *vm, u32 r) {
    u32 result;

    if (r == 0) {
        result = R0;
    } else if (r == RBP) {
        result = RBP;
    } else if (r == RSP) {
        result = RSP;
    } else if (r == SMP) {
        result = SMP;
    } else {
        u32 register_count = REGISTER_COUNT - R1;
        u32 first_addresable_register = R1;
        result = first_addresable_register + (r % register_count);
    }

    return result;
}

internal u64 *get_register(Virtual_machine *vm, u32 r) {
    u32 index = get_register_index(vm, r);
    u64 *result = vm->registers + index;
    return result;
}

internal void *get_memory(Virtual_machine *vm, Address _address) {
    void *result = 0;

    u64 *r = get_register(vm, _address.r);
    u8 *base = (u8 *)*r;
    result = (void *)(base + _address.offset);

    return result;
}

#define DEBUG_VM 10
#define DEBUG_VM_MEM 0

internal void execute(Virtual_machine *vm) {
    u32 i = 0;
    vm->registers[RSP] = (u64)vm->stack;
    vm->registers[SMP] = (u64)vm->string_pool->buffer;
    u64 *rsp = vm->registers + RSP;
    u64 *rbp = vm->registers + RBP;

    while (vm->program_counter < vm->bytecode->head) {
        Bytecode_instruction *it = vm->bytecode->buffer + vm->program_counter;
        Bytecode_instruction to_print = *it;

        Operand d = it->destination;
        Operand s = it->source;

        if (i == 184) {
            auto breakhere = 12;
        }

        if (i >= 220) {
            break;
        }

        u32 pc = vm->program_counter;

        switch (it->instruction) {
            case NOP: {} break;
            case MOVI: {
                if (d.type == REGISTER_ID) {
                    to_print.destination.r = get_register_index(vm, d.r);

                    u64 *r = get_register(vm, d.r);
                    *r = s.qword;
                } else if (d.type == ADDRESS) {
                    u32 *mem = (u32 *)get_memory(vm, d._address);
#if DEBUG_VM_MEM
                    debug(*mem);
#endif
                    *mem = s.dword;
#if DEBUG_VM_MEM
                    debug(*mem);
#endif

                    if (d._address.r == RBP) {
                        *rsp = al_max(*rsp, (u64)mem);
                    }
                }
            } break;
            case MOVR: {
                to_print.source.r = get_register_index(vm, s.r);

                if (d.type == REGISTER_ID) {
                    to_print.destination.r = get_register_index(vm, d.r);

                    u64 *r1 = get_register(vm, d.r);
                    u64 *r2 = get_register(vm, s.r);
                    *r1 = *r2;
                } else if (d.type == ADDRESS) {
                    u32 *mem = (u32 *)get_memory(vm, d._address);
                    u64 *r = get_register(vm, s.r);
#if DEBUG_VM_MEM
                    debug(*mem);
#endif
                    *mem = *r;
#if DEBUG_VM_MEM
                    debug(*mem);
#endif

                    if (d._address.r == RBP) {
                        *rsp = al_max(*rsp, (u64)mem);
                    }
                }
            } break;
            case MOVM: {
                if (d.type == REGISTER_ID) {
                    to_print.destination.r = get_register_index(vm, d.r);

                    u64 *r = get_register(vm, d.r);
                    u32 *mem = (u32 *)get_memory(vm, s._address);
                    *r = *mem;

                    if (d._address.r == RBP) {
                        *rsp = al_max(*rsp, (u64)mem);
                    }
                }
            } break;
            case ADDI: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 += *r2;
            } break;
            case SUBI: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 -= *r2;
            } break;
            case MULI: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 *= *r2;
            } break;
            case DIVI: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 /= *r2;
            } break;
            case MODI: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 %= *r2;
            } break;
            case GTI: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 > *r2;
            } break;
            case GTOEI: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 >= *r2;
            } break;
            case LTI: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 < *r2;
            } break;
            case LTOEI: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 <= *r2;
            } break;
            case ADDF: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                *r1 += *r2;
            } break;
            case SUBF: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                *r1 -= *r2;
            } break;
            case MULF: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                *r1 *= *r2;
            } break;
            case DIVF: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                *r1 /= *r2;
            } break;
            case GTF: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                vm->check_is_true = *r1 > *r2;
            } break;
            case GTOEF: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                vm->check_is_true = *r1 >= *r2;
            } break;
            case LTF: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                vm->check_is_true = *r1 < *r2;
            } break;
            case LTOEF: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                vm->check_is_true = *r1 <= *r2;
            } break;
            case EQ: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 == *r2;
            } break;
            case NEQ: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 != *r2;
            } break;
            case LNOT: {} break;
            case LOR: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 || *r2;
            } break;
            case LAND: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 && *r2;
            } break;
            case BNOT: {} break;
            case BOR: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 | *r2;
            } break;
            case BAND: {
                to_print.destination.r = get_register_index(vm, d.r);
                to_print.source.r = get_register_index(vm, s.r);

                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 & *r2;
            } break;
            case JUMP: {
                Tag_offset *offset = get(vm->tag_offset_table, d.tag.id);
                vm->program_counter = offset->bytecode_offset;
                goto ignore_program_counter_increment;
            } break;
            case JEQ: {
                if (vm->check_is_true) {
                    Tag_offset *offset = get(vm->tag_offset_table, d.tag.id);
                    vm->program_counter = offset->bytecode_offset;
                    goto ignore_program_counter_increment;
                }
            } break;
            case JNEQ: {
                if (!vm->check_is_true) {
                    Tag_offset *offset = get(vm->tag_offset_table, d.tag.id);
                    vm->program_counter = offset->bytecode_offset;
                    goto ignore_program_counter_increment;
                }
            } break;
            case TAG: {} break;
            case FTAG: {} break;
            case CALL: {
                push(vm, *rbp, *rsp, vm->program_counter);
                Function_offset *fo = get(vm->function_offset_table, d.function_name);
                vm->program_counter = fo->bytecode_offset;
                goto ignore_program_counter_increment;
            } break;
            case RET: {
                Call_stack *last = pop(vm);

                if (last) {
                    *rbp = last->rbp;
                    *rsp = last->rsp;
                    vm->program_counter = last->program_counter;
                }
            } break;
            case LEAVE: {} break;
            case PUSH: {} break;
            case POP: {} break;

            case BYTECODE_COUNT: {assert(false, "BYTECODE_COUNT must not be in the bytecode")} break;
            invalid_default_case_msg("unknown bytecode instruction");
        }

        vm->program_counter++;

ignore_program_counter_increment:
        auto __=2;
#if DEBUG_VM
        printf("(%u)", i++);
        print_instruction(&to_print, pc);
        putchar('\n');
        // debug(*rsp)
        // debug(*rbp)
        // debug(vm->check_is_true)
        // print_registers(vm);
        // print_stack(vm, *rbp, *rsp);
#endif
    }

    print_registers(vm);
}
