#pragma once

namespace ast {
enum class Type
{
    _VOID,
    _CHAR,
    _SHORT,
    _UNSIGNED_INT,
    _INT,
    _LONG,
    _FLOAT,
    _DOUBLE,
    _STRUCT,
    _CUSTOM_STRUCT,
};

//Defined in context.cpp now for universal use
extern const std::unordered_map<Type, int> types_size;
extern const std::unordered_map<Type, std::string> assembler_directives;

const std::unordered_map<Type, int> types_mem_shift = {
    {Type::_VOID, 0},
    {Type::_CHAR, 0},
    {Type::_SHORT, 1},
    {Type::_UNSIGNED_INT, 2},
    {Type::_INT, 2},
    {Type::_LONG, 3},
    {Type::_FLOAT, 2},
    {Type::_DOUBLE, 3},
};

}//namespace ast
