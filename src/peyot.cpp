/*
######## TODO ########
    -MODES:
        -compile: creates a binary into a .pvm file
        -run: runs a binary .pvm file with the virtual machine
    -take the ltl_interpreter path recomendator
    -windows layer, get rid of as much of std_lib/crt as possible
    -linux layer, get rid of as much of std_lib/crt as possible
    -arrays
    -pointers
    -generics struct v2 <T> {
        T x, y;
    }
    maybe expand the struct later to v2_u32 internally in the ast if v2<u32> is in the code and another like v2_f32 if v2<f32> is found, etc. These will just invoke to the create_type() thing or whatever i make for the structs, allow to check the type that was used in the T.
    -interface with the OS to get memory/open_files/etc
    -avoid infinitly big structs like:
        struct A {
            u32 x;
            u32 y;
            A other;
        }
    -maybe have 2 types of errors hard error when we have to stop the process and soft errors where we can keep doing stuff for example in the type checker its useful to have all of the errors at once but when parsing into the ast once you find an error you dont know what is happening in the input so its better to stop
*/
#if DEVELOPMENT
#include "external/superluminal/include/PerformanceAPI.h"
#endif


#define STB_SPRINTF_IMPLEMENTATION
#include"external/stb_sprintf.h"

#define print_indent(indent) printf("%*s", (indent), "")


#include"types.h"
#include"utils.h"
#include<stdio.h>
#include<stdlib.h>

global_variable bool ASSERT_FOR_DEBUGGING = false;

#include"debug.h"
#include"memory.h"

#include"platform.h"
#include"memory_pool.h"

#include"string.h"

#include"lexer.h"
#include"peyot_types.h"


#include"symbol_table.h"

global_variable Type_spec_table *global_type_table;

struct Ast_declaration;
struct Ast_statement;
struct Ast_expression;
struct Ast_if;
struct Ast_loop;
struct Ast_block;
struct Parser;
struct Pending_type;
struct Operator_table;

#include"type_checker.h"
#include"parser.h"
#include"peyot_operators.h"
#include"bytecode_generator.h"
#include"virtual_machine.h"

#include"lexer.cpp"
#include"parser.cpp"
#include"type_checker.cpp"
#include"bytecode_generator.cpp"
#include"virtual_machine.cpp"


s16 main(s16 arg_count, char **args) {
    PERFORMANCEAPI_INSTRUMENT_FUNCTION();
    setup_console();

    char *program_complex = R"PROGRAM(
    main :: (x :u32, y :u32) -> u32 {
        a :u32;
        a = 0;

        for (i :u32 = 0; 1; i = i + 1) {
            a = a + 1;
            a = a + 1;
            c :u32 = 2;

            if (1) {
                a :u32 = 0;
                if (22) {
                    a = 22;
                }
            } else if (2) {
                a :u32 = 321;
            } else if (3) {
                a :u32 = 567;
            } else {
                a :u32 = 34562;
            }
        }
    }
    )PROGRAM";

    char *program_function = R"PROGRAM(
    main :: (x :u32, y :u32) -> u32 {
        f(a, a+1, a.member);
        a :u32;
        a :u32 = 0;
        a.member;
        f(a, a+1, a.member) + a.member;
        f() + a.member;

        for (i :u32 = 0; 1; i = i + 1) {
            a = a + 1;
            a = a + 1;
            c :u32 = 2;
            if (1) {
                a = 0;
            }
        }
    }
    )PROGRAM";

    char *program_function_simple = R"PROGRAM(
    main :: (x :u32 , y :u32) -> u32 {
        a :u32 = 3+4*2 == 3%2 && 3 != 3-3-------++++3;
    }
    )PROGRAM";

    char *program_for = R"PROGRAM(
    {
        a :u32 = 0;

        for (i :u32 = 0; 1; i = i + 1) {
            a = a + 1;
            a = a + 1;
            c :u32 = 2;
            continue;
            break;
            return;
        }
    }
    )PROGRAM";
    char *program_while = R"PROGRAM(
    {
        a :u32 = 0;

        while (1) {
            a = a + 1;
            a = a + 1;
            c :u32 = 2;
        }
    }
    )PROGRAM";

    char *program_block = R"PROGRAM(
    {
        a :u32 = 1;
        b :u32 = 2;
        {
            c :u32 = a + b;
        }
        a=3;
    }
    )PROGRAM";

    char *program_1 = R"PROGRAM(
    {
        a :u32 = 1;
        a = a + 1;
    }
    )PROGRAM";

    char *program_0 = R"PROGRAM(
        1+2+3*2+4*4;
    )PROGRAM";

    char *program_if = R"PROGRAM(
        if (1) {
            a :u32 = 0;
        }
    )PROGRAM";

    char *program_if_else = R"PROGRAM(
        if (1) {
            a :u32 = 0;
        } else {
            a :u32 = 12;
        }
    )PROGRAM";

    char *program_if_else_if = R"PROGRAM(
        if (1) {
            a :u32 = 0;
        } else if (2) {
            a :u32 = 321;
        } else if (3) {
            a :u32 = 567;
        } else {
            a :u32 = 34562;
        }
    )PROGRAM";

    char *program_struct = R"PROGRAM(
        V2u :: struct {
            a :u32;
            b :u32;
        }
    )PROGRAM";

    char *program_union = R"PROGRAM(
        stuff :: union {
            a :u32;
            b :u32;
        }
    )PROGRAM";

    char *program_union_struct = R"PROGRAM(
        V2u :: union {
            struct {
                x: u32;
                y: u32;
            };
            struct {
                u: u32;
                v: u32;
            };
        }
    )PROGRAM";

    char *program_enum = R"PROGRAM(
        THING_TYPE :: enum {
            THING_NONE,

            THING_SMALL,
            THING_BIG=2,
        }
    )PROGRAM";

    char *program_error_1 = R"PROGRAM(
        main :: (x :u32, y :u32) -> u32 {
            really_long_name_to_see_if_the_pointer_points_to_the_end_of_the_variable: u32;
            really_long_name_to_see_if_the_pointer_points_to_the_end_of_the_variable + 12222;
            break;
            continue;
        }
    )PROGRAM";

    char *program_error_2 = R"PROGRAM(
        things :: enum {
            THING_NONE,

            THING_SMALL,
            THING_BIG=2,
        }
    )PROGRAM";

    char *program_error_3 = R"PROGRAM(
        a(1, 2, 3
    )PROGRAM";

    char *program_error_4 = R"PROGRAM(
        main :: (x :u32, y :u32) -> u32 {
            a :u32 = 3;
            12 + 2;
            break;
            continue
        }
    )PROGRAM";

    char *program_error_5 = R"PROGRAM(
        main :: (x :u32, y :u32) -> u32 {
            a :u32 = 3;
            12 + 2;
            break;
            continue
        }
    )PROGRAM";

    char *program_error_6 = R"PROGRAM(
        main :: (x :u32, y :u32) -> u32 {
            f(1, 2 2 2, 3);
            f(1, 2,, 3);
        }
    )PROGRAM";

    char *program_error_7 = R"PROGRAM(
        V2u :: struct {
            a :u32;
            b :u32;

    )PROGRAM";

    char *program_multiple_declarations = R"PROGRAM(

        V3u :: struct {
            x :u32;
            y :u32;
            z :u32;
        }

        main :: (a: u32, position: V2u) -> V2u {
            position = position + a;
            vector :V2u;
        }

        main :: (a: u32, position: V2u) -> V2u {
            position = position + a;
        }

        V2u :: struct {
            x :u32;
            y :u32;
        }
    )PROGRAM";

    char *program_type_error_1 = R"PROGRAM(

        V3u :: struct {
            x :u32a;
            y :u32;
            z :u32;
        }

        main :: (a: u32, position: V2u) -> V2ua {
            position = position + a;
            vector :V2ua;
        }

        main :: (a: u32, position: V2ua) -> V2uaa {
            position = position + a;
        }

        V2u :: struct {
            x :u32;
            y :u32;
        }
    )PROGRAM";

    char *program_type_checking = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        main :: (a: u32) -> u32 {
            very_long_name_for_a_vector :V2u;
            a :u32 = 1 + very_long_name_for_a_vector;
        }
    )PROGRAM";

    char *program_type_checking_long_expression = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        main :: (a: u32) -> u32 {
            very_long_name_for_a_vector :V2u;
            a :u32 = 1 + 2 + 1 + 1 + 1 + 1 + 1 + very_long_name_for_a_vector;
        }
    )PROGRAM";

    // TODO: handle multiline more gracefully
    char *program_type_checking_multiline = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        main :: (a: u32) -> u32 {
            very_long_name_for_a_vector :V2u;
            a :u32 = (
                  (1 + 2)
                + very_long_name_for_a_vector
            );
        }
    )PROGRAM";

    char *program_type_checking_multiple_types = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        V3u :: struct {
            x :u32;
            y :u32;
            z :u32;
        }
        main :: (a: u32) -> u32 {
            vector :V2u;
            vector2 :V3u;
            a :u32 = 1 * vector + vector2;
        }
    )PROGRAM";

    char *program_type_checking_assignment = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        main :: (a: u32) -> u32 {
            vector :V2u;
            a :u32 = vector;
        }
    )PROGRAM";

    char *program_type_checking_scope = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        main :: (a: u32) -> u32 {
            vector :V2u;
            a = vector;
        }
    )PROGRAM";

    // TODO: this one is not implemented
    char *program_type_checking_function = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }

        f ::(a :u32) ->u32 {
            return a;
        }

        main :: (a: u32) -> u32 {
            vector :V2u;
            a = vector;
        }
    )PROGRAM";

    char *program_error_undeclared_identifier = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        main :: (a: u32) -> u32 {
            a = vector + 11;
        }
    )PROGRAM";

    char *program_error_variable_redefinition = R"PROGRAM(
        main :: (a: u32) -> u32 {
            a :u32 = 11;
        }
    )PROGRAM";

    char *program_new_native_types = R"PROGRAM(
        main :: (in: u32) -> u32 {
            e :str = "a long string of characters to see if this crap holds any water";
            d :char = 'a';
            a :u32 = 1;
            b :s32 = -1;
            c :f32 = 1.3425;
        }
    )PROGRAM";

    char *program_type_checking_member = R"PROGRAM(
        V2f :: struct {
            x :f32;
            y :f32;
        }
        main :: (in: u32) -> u32 {
            p :V2f;
            p.x = p.x + in;
        }
    )PROGRAM";

    char *program_real_1 = R"PROGRAM(
        Entry_Loc :: struct {
            directory_block: u32;
            directory_index: u32;
        }

        str :: struct {
            count :u32;
            data :u32;
        }


        parse_path :: (path_str: str) -> str {
            path: str;

            if path_str.count {
                length: u32 = 1;
                for (i :u32= 0; 10; i = i + 1) {
                    it :char;

                    if it == '/' {
                        new: str;
                        new.data = path_str.data + i - length + 1;
                        new.count = length - 1;
                        length = 0;
                    }

                    length = length + 1;
                }

                new: str;
                new.data = path_str.data + path_str.count - length + 1;
                new.count = length - 1;
            }

            return path;
        }
    )PROGRAM";

    char *program_pre_post = R"PROGRAM(
        main :: (in :u32) -> u32 {
            a :u32 = 1;
            a++;
            ++a;
            a--;
            --a;
        }
    )PROGRAM";

    char *program_function_no_parameters = R"PROGRAM(
        main :: () -> u32 {
            a :u32 = 1;
        }
    )PROGRAM";

    char *program_compound_native_type = R"PROGRAM(
        main :: () -> u32 {
            a :str;
            a.count;
        }
    )PROGRAM";

    char *program_type_check_simple_type_member = R"PROGRAM(
        main :: () -> u32 {
            a :u32;
            a.count;
        }
    )PROGRAM";

    char *program_more_sentences = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        main :: () -> u32 {
            a :u32;
            sizeof(a);
            sizeof(u32);
            sizeof(V2u);
            offsetof(V2u, x);
            type(a);
            type(u32);
        }
    )PROGRAM";

    char *program_type_check_more_sentences = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        main :: () -> u32 {
            a :u32;
            sizeof(aa);
            sizeof(u3s2);
            sizeof(V2du);
            offsetof(Vfg2u, x);
            offsetof(V2u, asdf);
            type(aa);
            type(u3s2);
        }
    )PROGRAM";

    char *program_type_check_typedef = R"PROGRAM(
        int :: u32
        integer :: int

        main :: () -> u32 {
            a :u32;
            b :int;
            c :int = a + b;
            d :integer = c + a * c + b;
        }
    )PROGRAM";

    char *program_type_check_typedef_compound = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }
        Vector2 :: V2u

        main :: () -> u32 {
            p :V2u;
            p2 :Vector2;
            p2.x = p.x * 2;
            offsetof(Vector2, x);
        }
    )PROGRAM";

    char *program_type_inference = R"PROGRAM(
        V2u :: struct {
            x :f32;
            y :f32;
        }
        main :: () -> u32 {
            v :V2u;
            p := v;
            a := 1;
        }
    )PROGRAM";

    char *program_constant_declarations = R"PROGRAM(
        PI32 :: 3.141516
        COUNT :: 23
        CHARACTER :: 'a'
        NAME :: "William Shakespeare"

        main :: () -> u32 {
            pi :f32 = PI32;
            count :u32 = 23;
            character :char = CHARACTER;
            name :str = NAME;
            ERRORS HERE
            COUNT := 1;
            COUNT = 1;
        }
    )PROGRAM";
    char *program_type_comparison = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }

        main :: () -> u32 {
            u32 == u32;
            V2u == V2u;
            a :u32;
            type(a) == u32;
            u32 == type(a);
            p :V2u;
            type(p) == V2u;
            type(p.x) == u32;
            t := u32;
            type(t) == u32;
        }
    )PROGRAM";

    char *program_sizeof = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }

        main :: () -> u32 {
            a :u32 = sizeof(V2u) == sizeof(u32);
            p :V2u;
            type(p.x) == u32;
            sizeof(p);
            sizeof(p.ax);
        }
    )PROGRAM";

    char *program_sizes = R"PROGRAM(
        Quad :: struct {
            min :V2u;
            max :V2u;
            is_thing : bool;
        }
        V2u :: struct {
            x :f32;
            y :f32;
        }

        main :: () -> u32 {
            a := 3;
        }
    )PROGRAM";

    char *program_sizes_union = R"PROGRAM(
        Thing :: union {
            a :f32;
            b :bool;
            c :bool;
        }

        main :: () -> u32 {
            a := 3;
        }
    )PROGRAM";

    // TODO: annonymous structs and nesting with unions doesnt work
    char *program_sizes_union_this_doesnt_work = R"PROGRAM(
        A_thing :: union {
            struct {
                a :f32;
                b :f32;
            };
            struct {
                c :f32;
                d :f32;
            };
        }

        main :: () -> u32 {
            a := 3;
        }
    )PROGRAM";

    char *program_return_link_and_type_check = R"PROGRAM(
        main :: () -> u32 {
        }
    )PROGRAM";

    char *program_continue_break_link = R"PROGRAM(
        main :: () -> u32 {
            for (i :u32 = 1; i < 3; i ++) {
                break;
            }
            return 1;
        }
    )PROGRAM";

    char *program_undefined_operators = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }

        f::(x :u32) -> u32 {
            return x + 1;
        }

        main :: (in :u32, on: V2u, blah: f32) -> u32 {
            a :V2u;
            b :V2u;
            a + b;
            return 1;
        }
    )PROGRAM";

    char *program_undefined_operators2 = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }

        operator + :: (a: V2u, b: V2u)->V2u {
            c: V2u;

            c.x = a.x + b.x;
            c.y = a.y + b.y;

            return c;
        }

        main :: (in :u32, on: V2u, blah: f32) -> u32 {
            a :V2u;
            b :V2u;
            a + b;
            return 1;
        }
    )PROGRAM";


    char *program_bytecode_1 = R"PROGRAM(
        main :: (in :u32) -> u32 {
            a := 3;
            d := 2;
            b := a * 2 + 3 * type(d);
            return 1;
        }
    )PROGRAM";

    char *program_bytecode_2 = R"PROGRAM(
        main :: (in :u32) -> u32 {
            a := 3;
            d := 2;
            bol := d == a;
            t := type(a);
            s := sizeof(a);
            return 1;
        }
    )PROGRAM";

    char *program_bytecode_if = R"PROGRAM(
        main :: (in :u32) -> u32 {
            a := 3;
            d := 2;
            b :u32;

            if (a == d) {
                b = 1;
            } else if (a > d) {
                b = 3;
            } else {
                b = 2;
            }

            return 1;
        }
    )PROGRAM";

    char *program_bytecode_loop = R"PROGRAM(
        main :: (in :u32) -> u32 {
            sum := 0;

            for (i := 0; i; i = i + 1) {
                if (sum < 2) {
                    continue;
                }

                sum = sum + i;

                if (sum > 3) {
                    break;
                }
            }

            return 1;
        }
    )PROGRAM";

    char *program_bytecode_function = R"PROGRAM(
        add :: (a :u32, b :u32) -> u32 {
            result := a + b;
            return result;
        }

        main :: (in :u32) -> u32 {
            sum := add(1, 2);
            sum = sum + in;

            return 1;
        }
    )PROGRAM";

    char *program_bytecode_fibonacci = R"PROGRAM(
        function :: (x :u32) -> u32 {
            result :u32;
            if (x <= 1) {
                result = 1;
            } else {
                result = function(x - 1) + function(x - 2);
            }

            return result;
        }

        main :: (in :u32) -> u32 {
            sum := function(10);

            return 1;
        }
    )PROGRAM";

    char *program_bytecode_char_and_float = R"PROGRAM(
        main :: (in :u32) -> u32 {
            c := 'A';
            f := 2.0;

            return 1;
        }
    )PROGRAM";

    char *program_bytecode_struct = R"PROGRAM(
        V2u :: struct {
            x :u32;
            y :u32;
        }

        main :: (in :u32) -> u32 {
            a := 69;
            c :V2u;
            c.x = 1;
            c.y = 2;
            c.x = c.y;

            return 1;
        }
    )PROGRAM";

    char *program_bytecode_string = R"PROGRAM(
        main :: (in :u32) -> u32 {
            a := "a string";
            b := "another string";

            return 1;
        }
    )PROGRAM";

    char *program_bytecode_vm_basic = R"PROGRAM(
        main :: (in :u32) -> u32 {
            a := 1 + 2;

            return 69;
        }
    )PROGRAM";

    char *program_bytecode_vm_movs = R"PROGRAM(
        main :: (in :u32) -> u32 {
            a := 1;
            a2 := 2;
            a34 := 34;
            b := a;
            c := a+ b;

            return 69;
        }
    )PROGRAM";

    char *program_bytecode_vm_if = R"PROGRAM(
        main :: (in :u32) -> u32 {
            if (1) {
                a := 1 + 2;
            } else {
                a := 1 + 4;
            }

            return 69;
        }
    )PROGRAM";

// TODO: fibonacci of 10 freaks out
    char *program_bytecode_vm_function = R"PROGRAM(
        fib :: (x :u32) -> u32 {
            result :u32;
            if (x == 0) {
                result = 0;
            } else if (x == 1) {
                result = 1;
            } else {
                result = fib(x - 1) + fib(x - 2);
            }

            return result;
        }

        main :: (in :u32) -> u32 {
            //  0: 0
            //  1: 1
            //  2: 1
            //  3: 2
            //  4: 3
            //  5: 5
            //  6: 8
            //  7: 13
            //  8: 21
            //  9: 34
            // 10: 55
            f := fib(4);

            return f;
        }
    )PROGRAM";
    char *program_bytecode_vm_function2 = R"PROGRAM(
        fib :: (x :u32) -> u32 {
            result :u32;
            if (x == 0) {
                result = 0;
            } else if (x == 1) {
                result = 1;
            } else {
                result = fib(x - 1) + fib(x - 2);
            }

            return result;
        }

        add_1 ::(a :u32) -> u32 {
            result := a + 1;
            return result;
        }

        function :: (x1 :u32, x2 :u32, x3 :u32, x4 :u32, x5 :u32) -> u32 {
            result := add_1(x1) + add_1(x2) + add_1(x3) + add_1(x4) + add_1(x5);
            result = result *result;
            return result;
        }

        main :: (in :u32) -> u32 {
            //  0: 0
            //  1: 1
            //  2: 1
            //  3: 2
            //  4: 3
            //  5: 5
            //  6: 8
            //  7: 13
            //  8: 21
            //  9: 34
            // 10: 55
            f1 := 0;
            f2 := 1;
            f := f1;
            n := 50;

            for (i := 0; i < n; i = i+1) {
                f = f1 + f2;
                f2 = f1;
                f1 = f;

                if (f2 > 2) {
                    d := "a string so this is more juicy";
                    l := function(3, 4, 5, 6, 7);
                }
            }


            return f;
        }
    )PROGRAM";


// TODO: void functions dont have the jump back, move that code from the return to the closing scope from the body of the function

    char *program_bytecode_void_function = R"PROGRAM(
        function ::(a :u32) ->void {
            a = 23;
        }
        main :: (in :u32) -> u32 {
            a := 1 + 2;

            return 1;
        }
    )PROGRAM";


    Memory_pool allocator = {};

    Type_spec_table *type_table = new_type_spec_table(&allocator);
    global_type_table = type_table;
    Symbol_table *global_scope = new_symbol_table(&allocator);
    Operator_table *operator_table = new_operator_table(&allocator);
    Native_operations_table *native_operations_table = new_native_types_table(&allocator);

    string_context->allocator = push_struct(&allocator, Memory_pool);

    initialize_native_types(type_table, &allocator);
    // initialize_operators(type_table, operator_table, &allocator);
    initialize_native_operators(native_operations_table);

    Parser *parser = new_parser(&allocator, type_table, global_scope, operator_table, native_operations_table);
    Lexer lexer = create_lexer(program_bytecode_vm_function, parser, &allocator);


    get_next_token(&lexer);
    Lexer_savepoint lexer_savepoint = create_savepoint(&lexer);

    debug(lexer.source);
    debug(lexer.index);
    debug(lexer.current_line);

    // Ast_block *ast = parse_block(&lexer, 0);
    // Ast_statement *ast = parse_statement(&lexer, 0);
    // Ast_declaration *ast = parse_declaration(&lexer, 0);
    NAME_AND_EXECUTE(Ast_program *ast = parse_program(&lexer, 0));

    if (lexer.parser->parsing_errors) {
        report_parsing_errors(&lexer);
    } else {
        NAME_AND_EXECUTE(out_of_order_declaration(lexer.parser));

        if (out_of_order_declaration_errors(lexer.parser)) {
            report_type_declaration_errors(&lexer);
        } else {
            NAME_AND_EXECUTE(type_check(&lexer, ast))

            if (type_errors(lexer.parser)) {
                report_type_errors(&lexer);
            } else {
                Memory_pool bytecode_allocator = {};
                // TODO: the ast needs to keep the symbol tables so the bytecode generator knows the types of the variables, dont create stack Memory pools and dont clear them, store them in the correct ast node
                Bytecode_generator *generator = new_bytecode_generator(&bytecode_allocator, type_table, operator_table, native_operations_table);
                NAME_AND_EXECUTE(create_bytecode(generator, ast));
                print_string_pool(generator);
                print_bytecode(generator);
                print(generator->tag_offset_table);
                print(generator->function_offset_table);

                Memory_pool vm_allocator = {};
                Virtual_machine *vm = new_virtual_machine(&vm_allocator, generator->tag_offset_table, generator->function_offset_table, generator->string_pool, generator->bytecode, KILOBYTES(1));
                execute(vm);

                // rollback_lexer(lexer_savepoint);
                // test_parser(&lexer);
                BOLD(ITALIC(UNDERLINE(GREEN("\n\n\nfinished correctly\n"))));
                // debug(lexer.current_line);
            }
        }
    }


    // print(type_table);
    // print(global_scope);
    // print(operator_table);

    restore_console();

    return 0;
}
