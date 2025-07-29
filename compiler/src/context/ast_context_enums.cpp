#include "../../include/context/ast_context.hpp"

namespace ast{

void Context::define_enum(std::string name){
    if (typedefMap.empty())
    {
        throw std::runtime_error("Context::define_enum: custom_typedef is empty.");
    }

    if (enumMap.empty())
    {
        throw std::runtime_error("Context::define_enum: enum_name is empty.");
    }

    TypedefSpec typedef_specs = TypedefSpec(Type::_INT, 0, 0);
    typedefMap.back()[name] = typedef_specs;
}

void Context::define_enum_label(std::string label, int value){
    if (enumMap.empty()){
        throw std::runtime_error("Context::define_enum_label - No active scope");
    }
    enumMap.back()[label] = value;
}

bool Context::is_enum(std::string identifier){
    for (auto it = enumMap.rbegin(); it != enumMap.rend(); ++it){
        if (it->find(identifier) != it->end()){
            return true;
        }
    }
    return false;
}

int Context::get_enum_label(std::string label){
    for (auto it = enumMap.rbegin(); it != enumMap.rend(); ++it){
        if (it->find(label) != it->end()){
            return it->at(label);
        }
    }
    throw std::runtime_error("Context::GetEnumValue - enum value not found");
}


}//namespace ast
