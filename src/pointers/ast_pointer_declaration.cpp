#include "../../include/pointers/ast_pointer_declaration.hpp"

namespace ast{

std::string PointerDeclaration::GetId() const
{
    const Identifier *identifier = dynamic_cast<const Identifier *>(direct_declarator_.get());
    const Declarator *declarator = dynamic_cast<const Declarator *>(direct_declarator_.get());

    if (identifier)
    {
        return identifier->GetId();
    }
    else if (declarator)
    {
        return declarator->GetId();
    }

    throw std::runtime_error("PointerDeclaration::GetId() - direct_declarator_ is not an Identifier or PointerDeclarator");
}

void PointerDeclaration::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    // unsused
    (void)stream;
    (void)context;
    (void)dest_reg;
}

void PointerDeclaration::Print(std::ostream &stream) const
{
    stream << "*";
    direct_declarator_->Print(stream);
}

Type PointerDeclaration::GetType() const
{
    return Type::_INT;
}

int PointerDeclaration::get_deref() const
{
    const PointerDeclaration *ptr = dynamic_cast<const PointerDeclaration *>(direct_declarator_.get());

    if (ptr != nullptr)
    {
        return 1 + ptr->get_deref();
    }
    return 1;
}

std::vector<Parameter> PointerDeclaration::get_param(Context &context) const
{
    return dynamic_cast<const Declarator *>(direct_declarator_.get())->get_param(context);
}

void PointerDeclaration::store_param(std::ostream &stream, Context &context, std::string dest_reg) const
{
    return dynamic_cast<const Declarator *>(direct_declarator_.get())->store_param(stream, context, dest_reg);
}

int PointerDeclaration::get_offset() const
{
    int offset = dynamic_cast<const Declarator *>(direct_declarator_.get())->get_offset();
    if(offset %4 !=0){
        offset += 4 - (offset % 4);
    }

    return offset;
}

bool PointerDeclaration::isPointer() const
{
    return true;
}

}//namespace ast
