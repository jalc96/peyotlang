/*
######## TODO ########
    -MODES:
        -compile: creates a binary into a .pvm file
        -run: runs a binary .pvm file with the virtual machine
    -dump the asm generated
    -take the ltl_interpreter path recomendator
    -windows layer, get rid of as much of std_lib/crt as possible
    -linux layer, get rid of as much of std_lib/crt as possible
    -asm segments: https://www.tutorialspoint.com/assembly_programming/assembly_quick_guide.htm
    -performance analysis probably multithread the asm creation just to test performance
    -you can set the stack size of the program and if overflowed an error is shown
    -debugger for the virtual machine, showing the content of the registers and the content of the stack, show a window of the code (like 11 lines of assembly, the current one, 5 above and 5 below)
    -function parsing is easier if a keyword is introduced for function declaration like "fn" or "func" "fn do_stuff(u32 a) -> u32 {return 2 * a;}" maybe "function" since a lot of languages use that
    -add introspection for example have .type in structs to check for the type of structs and also be able to iterate over the members of structs, implement this with an integer, each time a struct is declare the type integer is incremented and that is asign to the struct, the basic types of the language are the first numbers. Add also something like typedef??
    -add the hability to undefine variables with the keyword undef
    -arrays
    -add introspection, be able to check a struct type and iterate over struct members
    -type checking in the ast
    -generics struct v2 <T> {
        T x, y;
    }
    maybe expand the struct later to v2_u32 internally in the ast if v2<u32> is in the code and another like v2_f32 if v2<f32> is found, etc. These will just invoke to the create_type() thing or whatever i make for the structs, allow to check the type that was used in the T.
    -operator overload
    -before allocating the memory for the bytecode, calculate all of the constants sizes and take that into account
    -do string pooling in the .data segment for constant strings
    -interface with the OS to get memory/open_files/etc
    -try Casey's idea for integers: dont have signed/unsigned types, have only integers and when type is important in an operation (multiply/divide/shift/etc) show an error and ask the user to specify someway (figure out this) which type is going to be used
    -show an error like c when a case in a switch statement is already used
        switch(thing){
            case A: do_stuf();break;
            case A: <---this throws an error
        }
*/

#define STB_SPRINTF_IMPLEMENTATION
#include"external/stb_sprintf.h"

#define print_indent(indent) printf("%*s", (indent), "")

#include"types.h"
#include"utils.h"
#include<stdio.h>
#include<stdlib.h>
#include"debug.h"
#include"memory.h"

#include"platform.h"
#include"memory_pool.h"

#include"string.h"
#include"lexer.h"
#include"peyot_types.h"


// #include"symbol_table.h"

struct Ast_declaration;
struct Ast_statement;
struct Ast_expression;
struct Ast_if;
struct Ast_loop;
struct Ast_block;
struct Parser;
struct Pending_type;

#include"lexer.cpp"
#include"type_checker.h"
#include"parser.h"
#include"type_checker.cpp"
#include"parser.cpp"





s16 main(s16 arg_count, char **args) {
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
        };
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

        main :: (a: u32, position: V2u) -> V2ua {
            position = position + a;
        }

        main :: (a: u32, position: V2u) -> V2uaa {
            position = position + a;
        }

        V2u :: struct {
            x :u32;
            y :u32;
        }
    )PROGRAM";


    Memory_pool allocator = {};

    Type_spec_table *type_table = new_type_spec_table(&allocator);
    initialize_native_types(type_table);

    Parser *parser = new_parser(&allocator, type_table);
    Lexer lexer = create_lexer(program_multiple_declarations, parser, &allocator);


    get_next_token(&lexer);
    Lexer_savepoint lexer_savepoint = create_savepoint(&lexer);

    debug(lexer.source);
    debug(lexer.index);
    debug(lexer.current_line);

    // Ast_block *ast = parse_block(&lexer, 0);
    // Ast_statement *ast = parse_statement(&lexer, 0);
    // Ast_declaration *ast = parse_declaration(&lexer, 0);
    Ast_program *ast = parse_program(&lexer, 0);
    out_of_order_declaration(lexer.parser);
    type_check(&lexer, ast);

    if (lexer.parser->parsing_errors) {
        report_parsing_errors(&lexer);
    } else if (type_errors(lexer.parser)) {
        report_type_errors(&lexer);
    } else {
        print(ast);
        rollback_lexer(lexer_savepoint);
        test_parser(&lexer);
        print(type_table);

        BOLD(ITALIC(UNDERLINE(GREEN("\n\n\nfinished correctly\n"))));

        debug(lexer.current_line);
    }

    restore_console();

    return 0;
}
