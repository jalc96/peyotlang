/*
######## TODO ########
    -get a memory allocator in the platform layer
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
    -error reporter that shows a sample of the code that created the error (a couple of lines above the current line should be enough)
    -add introspection for example have .type in structs to check for the type of structs and also be able to iterate over the members of structs, implement this with an integer, each time a struct is declare the type integer is incremented and that is asign to the struct, the basic types of the language are the first numbers. Add also something like typedef??
    -add the hability to undefine variables with the keyword undef
    -get the stb_printf for compositing strings (for the ast debug print)
    -structs
    -arrays
    -variable names with _ and the other special characters allowed in variable names in c-like languages
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
    -grammar check with a function require_token(lexer, TOKEN_OPEN_BRACE); and maybe have an error bool in the lexer or something to check stuff??
    -try Casey's idea for integers: dont have signed/unsigned types, have only integers and when type is important in an operation (multiply/divide/shift/etc) show an error and ask the user to specify someway (figure out this) which type is going to be used
    -show an error like c when a case in a switch statement is already used
        switch(thing){
            case A: do_stuf();break;
            case A: <---this throws an error
        }
*/
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
#include"lexer.cpp"
#include"parser.h"
#include"parser.cpp"





s16 main(s16 arg_count, char **args) {
    char *program_complex = R"PROGRAM(
    main :: (u32 x, u32 y) -> u32 {
        u32 a = 0;

        for (u32 i=0; 1; i = i + 1) {
            a = a + 1;
            a = a + 1;
            u32 c = 2;

            if (1) {
                u32 a = 0;
                if (22) {
                    a = 22;
                }
            } else if (2) {
                u32 a = 321;
            } else if (3) {
                u32 a = 567;
            } else {
                u32 a = 34562;
            }
        }
    }
    )PROGRAM";

    char *program_function = R"PROGRAM(
    main :: (u32 x, u32 y) -> u32 {
        u32 a = 0;

        for (u32 i=0; 1; i = i + 1) {
            a = a + 1;
            a = a + 1;
            u32 c = 2;
        }
    }
    )PROGRAM";

    char *program_function_simple = R"PROGRAM(
    main :: (u32 x, u32 y) -> u32 {
        u32 a = 3+4*2 == 3%2 && 3 != 3-3-------++++3;
    }
    )PROGRAM";

    char *program_for = R"PROGRAM(
    {
        u32 a = 0;

        for (u32 i=0; 1; i = i + 1) {
            a = a + 1;
            a = a + 1;
            u32 c = 2;
            continue;
            break;
            return;
        }
    }
    )PROGRAM";
    char *program_while = R"PROGRAM(
    {
        u32 a = 0;

        while (1) {
            a = a + 1;
            a = a + 1;
            u32 c = 2;
        }
    }
    )PROGRAM";

    char *program_block = R"PROGRAM(
    {
        u32 a = 1;
        u32 b = 2;
        {
            u32 c = a + b;
        }
        a=3;
    }
    )PROGRAM";

    char *program_1 = R"PROGRAM(
    {
        u32 a = 1;
        a = a + 1;
    }
    )PROGRAM";

    char *program_0 = R"PROGRAM(
        1+2+3*2+4*4;
    )PROGRAM";

    char *program_if = R"PROGRAM(
        if (1) {
            u32 a = 0;
        }
    )PROGRAM";

    char *program_if_else = R"PROGRAM(
        if (1) {
            u32 a = 0;
        } else {
            u32 a = 12;
        }
    )PROGRAM";

    char *program_if_else_if = R"PROGRAM(
        if (1) {
            u32 a = 0;
        } else if (2) {
            u32 a = 321;
        } else if (3) {
            u32 a = 567;
        } else {
            u32 a = 34562;
        }
    )PROGRAM";

    char *program_struct = R"PROGRAM(
        struct V2u :: {
            u32 a;
            u32 b;
        };
    )PROGRAM";

    char *program_union = R"PROGRAM(
        union stuff :: {
            u32 a;
            u32 b;
        };
    )PROGRAM";

    char *program_union_struct = R"PROGRAM(
        union V2u :: {
            struct :: {
                u32 x;
                u32 y;
            };
            struct :: {
                u32 u;
                u32 v;
            };
        };
    )PROGRAM";

    char *program_enum = R"PROGRAM(
        enum THING_TYPE :: {
            THING_NONE,

            THING_SMALL,
            THING_BIG=2,
        };
    )PROGRAM";


    Memory_pool allocator = {};

    Type_spec_table *type_table = new_type_spec_table(&allocator);
    initialize_native_types(type_table);

    Parser parser = new_parser(type_table);
    Lexer lexer = create_lexer(program_for, &parser, &allocator);


    get_next_token(&lexer);
    Lexer_savepoint lexer_savepoint = create_savepoint(&lexer);

    debug(lexer.source);
    debug(lexer.index);
    debug(lexer.current_line);

    // Ast_block *ast = parse_block(&lexer, 0);
    Ast_statement *ast = parse_statement(&lexer, 0);
    // Ast_declaration *ast = parse_declaration(&lexer, 0);
    print(ast);
    rollback_lexer(lexer_savepoint);
    test_parser(&lexer);
    print(type_table);

    BOLD(ITALIC(UNDERLINE(GREEN("\n\n\nfinished correctly\n"))));

    return 0;
}
