enum PENDING_TYPE_TYPE {
    PENDING_TYPE_NONE,

// TODO: trim down to only the useful ones
    PENDING_TYPE_DECLARATION,
    PENDING_TYPE_STATEMENT,
    PENDING_TYPE_EXPRESSION,
    PENDING_TYPE_IF,
    PENDING_TYPE_LOOP,
    PENDING_TYPE_FUNCTION_PARAMETER,
    PENDING_TYPE_MEMBER,

    PENDING_TYPE_COUNT,
};

struct Parameter;
struct Member;

struct Pending_type {
    PENDING_TYPE_TYPE type;

    str type_name;
    Src_position type_name_location;
    u32 times_checked;

    union {
        Ast_declaration *node_declaration;
        Ast_statement *node_statement;
        Ast_expression *node_expression;
        Ast_if *node_if;
        Ast_loop *node_loop;
        Ast_block *node_block;
        Parameter *node_parameter;
        Member *node_member;
    };

    Pending_type *previous;
    Pending_type *next;
};

internal Pending_type *new_pending_type(Memory_pool *allocator, str type_name) {
    Pending_type *result = push_struct(allocator, Pending_type);

    result->type_name = type_name;

    return result;
}
