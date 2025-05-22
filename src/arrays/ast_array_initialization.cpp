#include "../../include/arrays/ast_array_initialization.hpp"

namespace ast{

int ArrayInitialization::GetArraySize() const
{
    return dynamic_cast<const NodeList *>(initializer_list_.get())->get_nodes().size();
}

void ArrayInitialization::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    (void)stream;
    (void)context;
    (void)dest_reg;
}

void ArrayInitialization::Print(std::ostream &stream) const
{
    stream << "{ ";
    initializer_list_->Print(stream);
    stream << " }";
}

void ArrayInitialization::SaveValue(std::ostream &stream, Context &context, Variable variable, std::string identifier) const
{
    int offset;
    Type type = variable.get_type();
    std::string dest_reg = context.get_register(type);

    if (variable.get_scope() == ScopeLevel::LOCAL){
        offset = variable.get_offset();
        for (const auto& initializer : dynamic_cast<const NodeList *>(initializer_list_.get())->get_nodes())
        {
            dynamic_cast<const Operand *>(initializer.get())->EmitRISC(stream, context, dest_reg);
            stream << context.store_instr(type) << " " << dest_reg << ", " << offset << "(s0)" << std::endl;

            offset += types_size.at(type);
        }

    }
    else if (variable.get_scope() == ScopeLevel::GLOBAL)
    {
        std::string global_memory_location = "global_" + identifier;
        std::string global_memory_register = context.get_register(Type::_INT);

        stream << "lui " << global_memory_register << ", " << "%hi(" << global_memory_location << ")" << std::endl;
        stream << "addi " << global_memory_register << ", " << "%lo(" << global_memory_location << ")" << std::endl;

        offset = 0;

        for (const auto& initializer : dynamic_cast<const NodeList *>(initializer_list_.get())->get_nodes())
        {
            dynamic_cast<const Operand *>(initializer.get())->EmitRISC(stream, context, dest_reg);

            stream << context.store_instr(type) << " " << dest_reg << ", " << offset << "(" << global_memory_register << ")" << std::endl;

            offset += types_size.at(type);
        }

        context.deallocate_register(global_memory_register);
    }

    else
    {
        throw std::runtime_error("ArrayInitialization Save: Invalid scope.");
    }


    context.deallocate_register(dest_reg);
}

void ArrayInitialization::global_init(std::ostream &stream, Context &context, Global &global) const
{
    (void)context;
    (void)stream;
    for (const auto& initializer : dynamic_cast<const NodeList *>(initializer_list_.get())->get_nodes())
    {
        if (initializer == nullptr)
        {
            continue;
        }
        dynamic_cast<const Constant *>(initializer.get())->SaveValue(global);
    }
}

}
