#include "../../include/arrays/ast_array_declaration.hpp"

namespace ast{
std::string ArrayDeclaration::GetId() const
{
    const Identifier *identifier = dynamic_cast<const Identifier *>(identifier_.get());
    const Declarator *declarator = dynamic_cast<const Declarator *>(identifier_.get());
    if (identifier != nullptr)
    {
        return identifier->GetId();
    }
    else if (declarator != nullptr)
    {
        return declarator->GetId();
    }
    throw std::runtime_error("ArrayDeclaration::GetId - not an identifier");
}

int ArrayDeclaration::GetArraySize() const
{
    if (constant_expression_ == nullptr)
    {
        return -1;
    }
    const IntConstant *int_const = dynamic_cast<const IntConstant *>(constant_expression_.get());
    const Identifier *enumerator = dynamic_cast<const Identifier *>(constant_expression_.get());
    const ArrayDeclaration *array_declaration=  dynamic_cast<const ArrayDeclaration *>(identifier_.get());
    if (int_const != nullptr && array_declaration == nullptr)
    {
        return int_const->get_val();
    }
    else if(int_const != nullptr && array_declaration != nullptr){
        return int_const->get_val() * array_declaration->GetArraySize();
    }
    if (enumerator != nullptr)
    {
        return enumerator->get_val(context);
    }
    throw std::runtime_error("ArrayDeclaration::GetArraySize - array is null and neither is a enumerator");
}

std::vector<int> ArrayDeclaration::GetArrayDim(Context& context) const
{
    std::vector<int> dim;
    collect_dims(context, dim);
    return dim;
}

void ArrayDeclaration::collect_dims(Context& context, std::vector<int>& dim) const{
    const ArrayDeclaration* nested_array = dynamic_cast<const ArrayDeclaration*>(identifier_.get());
    if(nested_array != nullptr){
        nested_array->collect_dims(context, dim);
    }

    if(constant_expression_ != nullptr){
        const IntConstant* int_const = dynamic_cast<const IntConstant*>(constant_expression_.get());
        const Identifier* ident = dynamic_cast<const Identifier*>(constant_expression_.get());
        if(int_const != nullptr){
            dim.push_back(int_const->get_val());
        }
        else if(ident != nullptr){
            dim.push_back(ident->get_index(context));
        }
    }
}

void ArrayDeclaration::EmitElsonV(std::ostream &stream, Context &context, std::string dest_reg) const
{
    (void)stream;
    (void)context;
    (void)dest_reg;
}

void ArrayDeclaration::Print(std::ostream &stream) const
{
    identifier_->Print(stream);
    stream << "[";
    if (constant_expression_ != nullptr)
    {
        constant_expression_->Print(stream);
    }
    stream << "]";
}

bool ArrayDeclaration::isPointer() const
{
    return dynamic_cast<const PointerDeclaration *>(identifier_.get()) != nullptr;
}

std::vector<Parameter> ArrayDeclaration::get_param(Context &context) const
{
    return dynamic_cast<const Declarator *>(identifier_.get())->get_param(context);
}

int ArrayDeclaration::get_offset() const
{
    return dynamic_cast<const Declarator *>(identifier_.get())->get_offset();
}

void ArrayDeclaration::store_param(std::ostream &stream, Context &context, std::string dest_reg) const
{
    return dynamic_cast<const Declarator *>(identifier_.get())->store_param(stream, context, dest_reg);
}

int ArrayDeclaration::get_deref() const
{
    const Declarator *declarator = dynamic_cast<const Declarator *>(identifier_.get());
    if (declarator)
    {
        return declarator->get_deref();
    }

    return 0;
}


}
