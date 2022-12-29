internal u32 get_register_index(Virtual_machine *vm, u32 r) {
    u32 result;

    if (r == 0) {
        result = R0;
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

internal void execute(Virtual_machine *vm) {
    while (vm->program_counter < vm->bytecode->head) {
        Bytecode_instruction *it = vm->bytecode->buffer + vm->program_counter;
        Operand d = it->destination;
        Operand s = it->source;

        switch (it->instruction) {
            case NOP: {} break;
            case MOVI: {
                if (d.type == REGISTER_ID) {
                    u64 *r = get_register(vm, d.r);
                    *r = s.qword;
                }
            } break;
            case MOVR: {} break;
            case MOVM: {} break;
            case ADDI: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 += *r2;
            } break;
            case SUBI: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 -= *r2;
            } break;
            case MULI: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 *= *r2;
            } break;
            case DIVI: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 /= *r2;
            } break;
            case MODI: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                *r1 %= *r2;
            } break;
            case GTI: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 > *r2;
            } break;
            case GTOEI: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 >= *r2;
            } break;
            case LTI: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 < *r2;
            } break;
            case LTOEI: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 <= *r2;
            } break;
            case ADDF: {
                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                *r1 += *r2;
            } break;
            case SUBF: {
                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                *r1 -= *r2;
            } break;
            case MULF: {
                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                *r1 *= *r2;
            } break;
            case DIVF: {
                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                *r1 /= *r2;
            } break;
            case GTF: {
                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                vm->check_is_true = *r1 > *r2;
            } break;
            case GTOEF: {
                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                vm->check_is_true = *r1 >= *r2;
            } break;
            case LTF: {
                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                vm->check_is_true = *r1 < *r2;
            } break;
            case LTOEF: {
                f64 *r1 = (f64 *)get_register(vm, d.r);
                f64 *r2 = (f64 *)get_register(vm, s.r);
                vm->check_is_true = *r1 <= *r2;
            } break;
            case EQ: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 == *r2;
            } break;
            case NEQ: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 != *r2;
            } break;
            case LNOT: {} break;
            case LOR: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 || *r2;
            } break;
            case LAND: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 && *r2;
            } break;
            case BNOT: {} break;
            case BOR: {
                u64 *r1 = get_register(vm, d.r);
                u64 *r2 = get_register(vm, s.r);
                vm->check_is_true = *r1 | *r2;
            } break;
            case BAND: {
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
            case CALL: {} break;
            case RET: {} break;
            case LEAVE: {} break;
            case PUSH: {} break;
            case POP: {} break;

            case BYTECODE_COUNT: {assert(false, "BYTECODE_COUNT must not be in the bytecode")} break;
            invalid_default_case_msg("unknown bytecode instruction");
        }

        vm->program_counter++;

ignore_program_counter_increment:

        print(it->instruction);
        putchar(':');
        putchar('\n');
        debug(vm->check_is_true)
        print_registers(vm);
    }
}
