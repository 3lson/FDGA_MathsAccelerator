#include "../../include/context/ast_context.hpp"

namespace ast {

void Context::create_scope(){
    variableMap.push_back(std::unordered_map<std::string, Variable>());
    enumMap.push_back(enum_Map());
    current_stack_offset.push(current_stack_offset.top());
}

void Context::pop_scope(){
    if (!variableMap.empty()){
        variableMap.pop_back();
    }
    if (!enumMap.empty()){
        enumMap.pop_back();
    }
    if (!current_stack_offset.empty()){
        current_stack_offset.pop();
    }
}

int Context::get_stack_offset() const {
    return current_stack_offset.top() - total_offset;
}

void Context::increase_stack_offset(int offset){
    current_stack_offset.top() += offset;
    //4 byte alignment
    if (current_stack_offset.top() % 4 !=0){
        current_stack_offset.top() += 4 - (current_stack_offset.top() % 4);
    }
}

void Context::set_stack_offset(int offset){
    total_offset = offset;
    if (offset % 4 != 0)
    {
        std::runtime_error("Context::set_stack_offset: stack offset not aligned to 4 bytes");
    }
}

}//namespace ast

