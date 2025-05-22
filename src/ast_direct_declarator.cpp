#include "../include/ast_direct_declarator.hpp"

namespace ast{

void DirectDeclarator::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    identifier_->EmitRISC(stream, context, dest_reg);
    stream << ":" << std::endl;
}

void DirectDeclarator::Print(std::ostream &stream) const
{
    identifier_->Print(stream);
    if (parameter_list_ != nullptr)
    {
        stream << "(";
        parameter_list_->Print(stream);
        stream << ")";
    }
}

std::string DirectDeclarator::GetId() const
{
    std::cout << "Passing inside DirectDeclarator::GetId()" << std::endl;
    const Identifier *id = dynamic_cast<const Identifier *>(identifier_.get());
    const Declarator *declarator = dynamic_cast<const Declarator *>(identifier_.get());
    if (id != nullptr) {
        return id->GetId();
    } else if (declarator != nullptr){
        return declarator->GetId();
    }
    throw std::runtime_error("DirectDeclarator::GetId() - identifier_ is not an Identifier");

}

std::vector<Parameter> DirectDeclarator::get_param(Context &context) const
{
    if (parameter_list_ == nullptr)
    {
        return std::vector<Parameter>();
    }
    else
    {
        const ParameterList* parameter_list = dynamic_cast<const ParameterList*>(parameter_list_.get());
        return parameter_list->get_param(context);
    }
}

int DirectDeclarator::get_offset() const
{
    if (parameter_list_ == nullptr)
    {
        return 0;
    }
    else
    {
        const ParameterList* parameter_list = dynamic_cast<const ParameterList*>(parameter_list_.get());
        return parameter_list->get_offset();
    }
}


void DirectDeclarator::store_param(std::ostream &stream, Context &context, std::string dest_reg) const
{
    if (parameter_list_ != nullptr)
    {
        const ParameterList *parameter_list = dynamic_cast<const ParameterList*>(parameter_list_.get());
        parameter_list->EmitRISC(stream, context, dest_reg);
    }
}

bool DirectDeclarator::isPointer() const
{
    return false;
}

int DirectDeclarator::get_deref() const
{
    const Declarator *declarator = dynamic_cast<const Declarator *>(identifier_.get());
    if (declarator != nullptr)
    {
        return declarator->get_deref();
    }

    return 0;
}

}//namespace ast
