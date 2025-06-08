#include "../../include/context/ast_context.hpp"

namespace ast{

//extern Context context; //Global context object for the lexer hack in typedef

const std::unordered_map<Type, int> types_size = {
    {Type::_VOID, 0},
    {Type::_CHAR, 1},
    {Type::_SHORT, 2},
    {Type::_UNSIGNED_INT, 4},
    {Type::_INT, 4},
    {Type::_LONG, 8},
    {Type::_FLOAT, 4},
    {Type::_DOUBLE, 8},
    {Type::_CUSTOM_STRUCT, 8},
};

const std::unordered_map<Type, std::string> assembler_directives = {
    {Type::_VOID, ".space"},
    {Type::_CHAR, ".byte"},
    {Type::_SHORT, ".half"},
    {Type::_UNSIGNED_INT, ".word"},
    {Type::_INT, ".word"},
    {Type::_LONG, ".word"},
    {Type::_FLOAT, ".word"},
    {Type::_DOUBLE, ".word"},
};

Context::Context()
{
    label_counter = 0;
    current_stack_offset.push(0);
    allocated_registers.push(std::set<int>());

    enumMap.push_back(enum_Map());
    typedefMap.push_back(std::unordered_map<std::string, TypedefSpec>());
}

Context::~Context()
{
}

}//namespace ast

ast::Context ast::context;
