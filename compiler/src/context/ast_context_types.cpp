#include "../../include/context/ast_context.hpp"
namespace ast{

void Context::push_operation_type(Type type) { operation_stack.push(type); }
void Context::pop_operation_type() { operation_stack.pop(); }
Type Context::get_operation_type() const { return operation_stack.top(); }
bool Context::is_type_stack_empty() const { return operation_stack.size() > 0; }


}//namespace ast
