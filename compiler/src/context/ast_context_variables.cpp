#include "../../include/context/ast_context.hpp"

namespace ast{

//Define variable in the current scope
void Context::define_variable(const std::string& name, const Variable& var){
    if (variableMap.empty()){
        throw std::runtime_error("Error: Attempted to define variable in an empty scope.");
    }
    variableMap.back()[name] = var;

}

//Retreive a variable from the current or parent scopes
Variable& Context::get_variable(const std::string& name){
    auto global_it = globalMap.find(name);
    if (global_it != globalMap.end()) {
        return global_it->second;
    }
    
    for (auto it = variableMap.rbegin(); it != variableMap.rend(); ++it) {
        auto var_it = it->find(name);
        if (var_it != it->end()) {
            return var_it->second;
        }
    }
    
    throw std::runtime_error("Error: Variable '" + name + "' not found.");
}

void Context::define_global(std::string name, Global &global){
    globalMap[name] = global;
}

}//namespace ast
