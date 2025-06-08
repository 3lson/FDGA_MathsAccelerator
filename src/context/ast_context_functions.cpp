#include "../../include/context/ast_context.hpp"
namespace ast{
void Context::define_function(std::string identifier, Function function)
{
    functionMap[identifier] = function;
    last_function_end_statement = identifier + "_end";
    set_return_register(function.get_return_value().is_pointer() ? Type::_INT : function.get_return_value().get_type());
    current_stack_offset.push(0);
}

Function Context::get_function(std::string identifier) const
{
    if (functionMap.find(identifier) == functionMap.end())
    {
        throw std::runtime_error("Context::get_function - function " + identifier + " not found");
    }
    return functionMap.at(identifier);
}

std::string Context::get_function_end() const
{
    if (last_function_end_statement.empty())
    {
        throw std::runtime_error("Context::get_function_end - no function defined");
    }
    return last_function_end_statement;
}

void Context::set_return_register(Type type)
{
    switch (type)
    {
    case Type::_INT:
    case Type::_CHAR:
    case Type::_SHORT:
    case Type::_UNSIGNED_INT:
        return_register = "a0";
        break;
    case Type::_FLOAT:
    case Type::_DOUBLE:
    case Type::_LONG:
        return_register = "fa0";
        break;
    case Type::_VOID:
        return_register = "zero";
        std::cerr << "Context::set_return_register - zero register set as return for type VOID" << std::endl;
        break;
    default:
        std::cerr << "type: " << std::endl;
        std::cerr << static_cast<int>(type) << std::endl;
        throw std::runtime_error("Context::set_return_register: Invalid return type");
    }
}

int Parameter::GetTypeSize() const{
    if (is_pointer()){
        return types_size.at(Type::_INT);
    }
    return types_size.at(type);
}

void Context::push_function_call(std::string function)
{
    function_stack.push(function);
    allocated_registers.push(std::set<int>());
}

void Context::pop_function_call()
{
    function_stack.pop();
    allocated_registers.pop();
}

Function Context::get_function_call() const{
    if (function_stack.empty())
    {
        throw std::runtime_error("Context::get_function_call - no function call");
    }
    return get_function(function_stack.top());
}

void Context::exit_function()
{
    current_stack_offset.pop();
}


}//namespace ast
