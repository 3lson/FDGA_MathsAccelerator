#include "../../include/context/ast_context.hpp"

namespace ast{

bool Context::is_typedef(const std::string& name) const
{
    for (auto it = typedefMap.rbegin(); it != typedefMap.rend(); ++it)
    {
        if (it->find(name) != it->end())
        {
            return true;
        }
    }

    return false;
}

TypedefSpec Context::get_typedef(const std::string& name) const
{
    for (auto it = typedefMap.rbegin(); it != typedefMap.rend(); ++it)
    {
        if (it->find(name) != it->end())
        {
            return it->at(name);
        }
    }

    throw std::runtime_error("Context::get_typedef_specs - no corresponding spec found");
}

void Context::define_typedef(const std::string& name, const TypedefSpec& spec)
{
    if (name.empty())
    {
        throw std::runtime_error("Context::define_typedef: Attempted to define a typedef with an empty label");
    }

    if (typedefMap.empty())
    {
        typedefMap.push_back(std::unordered_map<std::string, TypedefSpec>());
    }
    typedefMap.back()[name] = spec;
}
}//namespace ast