#include "../../include/pointers/ast_addressof.hpp"

namespace ast {

Type AddressOf::GetType(Context& context) const
{
    (void)context;
    return Type::_INT;
}

void AddressOf::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const
{
    std::string identifier = GetId();

    Variable variable = context.get_variable(identifier);
    Type type = isPointerOp(context) ? Type::_INT : GetType(context);

    const ArrayIndexAccess *array_index_access = dynamic_cast<const ArrayIndexAccess *>(operand_.get());

    if (array_index_access){
        std::string index_register = context.get_register(Type::_INT);
        array_index_access->get_position(stream, context, index_register, type);

        if (variable.get_scope() == ScopeLevel::LOCAL)
        {
            if (variable.is_pointer())
            {
                std::string pointer_register = context.get_register(Type::_INT);
                stream << context.load_instr(Type::_INT) << " " << pointer_register << ", " << variable.get_offset() << "(s0)" << std::endl;
                stream << "add " << dest_reg << ", " << index_register << ", " << pointer_register << std::endl;
                context.deallocate_register(pointer_register);
            }
            else if (variable.is_array())
            {
                stream << "add " << dest_reg << ", " << index_register << ", s0" << std::endl;
                stream << "addi " << dest_reg << ", " << dest_reg << ", " << variable.get_offset() << std::endl;
            }
            else
            {
                throw std::runtime_error("ArrayIndexAccess EmitRISC: Variable is not a pointer or array");
            }
        }

        else if (variable.get_scope() == ScopeLevel::GLOBAL)
        {
            stream << "la " << dest_reg << ", global_" << identifier << std::endl;
            stream << "add " << dest_reg << ", " << dest_reg << ", " << index_register << std::endl;
        }

        else
        {
            throw std::runtime_error("AddressOf EmitRISC: variable_specs.scope is not local or global");
        }

        context.deallocate_register(index_register);
    }
    else{
        if (variable.get_scope() == ScopeLevel::LOCAL)
        {
            stream << "addi " << dest_reg << ", s0, " << variable.get_offset() << std::endl;
        }
        else if (variable.get_scope() == ScopeLevel::GLOBAL)
        {
            stream << "la " << dest_reg << ", global_" << identifier << std::endl;
        }
        else
        {
            throw std::runtime_error("AddressOf EmitRISC: Unsupported variable scope");
        }
    }
}

void AddressOf::Print(std::ostream& stream) const
{
    stream << "&";
    operand_->Print(stream);
}

std::string AddressOf::GetId() const
{
    const Identifier* identifier = dynamic_cast<const Identifier*>(operand_.get());
    const ArrayIndexAccess* array_index_access = dynamic_cast<const ArrayIndexAccess*>(operand_.get());
    const AddressOf *address_of = dynamic_cast<const AddressOf*>(operand_.get());

    if (identifier != nullptr)
    {
        return identifier->GetId();
    }
    else if (array_index_access != nullptr)
    {
        return array_index_access->GetId();
    }
    else if (address_of != nullptr)
    {
        return address_of->GetId();
    }

    throw std::runtime_error("Invalid address-of expression: expected an identifier or array access");
}


bool AddressOf::isPointerOp(Context &context) const
{
    (void)context;
    return true;
}

int AddressOf::AddressCount() const
{
    const AddressOf *adddress_of = dynamic_cast<const AddressOf *>(operand_.get());

    if (adddress_of != nullptr)
    {
        return 1 + adddress_of->AddressCount();
    }
    return 1;
}

} // namespace ast