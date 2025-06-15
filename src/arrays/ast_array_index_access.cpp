#include "../include/arrays/ast_array_index_access.hpp"
#include <vector>
#include <algorithm> //for std::reverse
namespace ast{

std::string ArrayIndexAccess::GetId() const
{
    const Identifier *identifier = dynamic_cast<const Identifier *>(identifier_.get());
    const ArrayIndexAccess *array_index_access = dynamic_cast<const ArrayIndexAccess *>(identifier_.get());
    if (identifier != nullptr)
    {
        return identifier->GetId();
    }
    else if (array_index_access != nullptr){
        return array_index_access->GetId();
    }
    throw std::runtime_error("ArrayIndexAccess::GetId - not an identifier");
}

void ArrayIndexAccess::EmitElsonV(std::ostream &stream, Context &context, std::string dest_reg) const
{
    Variable variable = context.get_variable(GetId());

    Type type = isPointerOp(context) ? Type::_INT : GetType(context);

    std::string index_register = context.get_register(Type::_INT);
    get_position(stream, context, index_register, type, variable);

    if (variable.get_scope() ==ScopeLevel::LOCAL){
        if (variable.is_pointer())
        {
            // Pointers points to first item in list
            std::string pointer_register = context.get_register(Type::_INT);
            stream << context.load_instr(Type::_INT) << " " << pointer_register << ", " << variable.get_offset() << "(s0)" << std::endl;
            stream << "add " << index_register << ", " << index_register << ", " << pointer_register << std::endl;
            context.deallocate_register(pointer_register);
        }
        else if (variable.is_array())
        {
            std::string temp_register = context.get_register(Type::_INT);

            stream << asm_prefix.at(context.get_instruction_state()) << "li " << temp_register << ", " << variable.get_offset() << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "sub " << index_register << ", " << temp_register << ", " << index_register << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "add " << index_register << ", " << index_register << ", sp" << std::endl; //sp contains the bottom of the stack offset value
            
            context.deallocate_register(temp_register);
        }
        else
        {
            throw std::runtime_error("ArrayAccess EmitElsonV: Variable is not a pointer or array");
        }

        stream << asm_prefix.at(context.get_instruction_state()) << context.load_instr(type) << " " << dest_reg << ", 0(" << index_register << ")" << std::endl;
    }
    else if(variable.get_scope() == ScopeLevel::GLOBAL){
        std::string global_memory_location = "global_" + GetId();
        std::string global_memory_register = context.get_register(Type::_INT);

        stream << "lui " << global_memory_register << ", " << "%hi(" << global_memory_location << ")" << std::endl;
        stream << "add " << global_memory_register << ", " << global_memory_register << ", " << index_register << std::endl;
        stream << context.load_instr(type) << " " << dest_reg << ", %lo(" << global_memory_location << ")(" << global_memory_register << ")" << std::endl;

        context.deallocate_register(global_memory_register);
    }

    context.deallocate_register(index_register);
}

void ArrayIndexAccess::get_position(std::ostream &stream, Context &context, std::string dest_reg, Type type, Variable& variable) const
{
    // Set operation type as dealing with pointers
    context.push_operation_type(Type::_INT);

    std::vector<int> indexes = get_linear_index(context);
    std::vector<int> dimension = variable.get_dim();

    int linear_index;

    //only can handle up to 2d arrays
    if(indexes.size() == 2){
        linear_index = indexes[0] * dimension[1] + (indexes[1]);
    }
    else{
        linear_index = indexes[0];
    }

    // Emit index to specified register
    //index_->EmitElsonV(stream, context, dest_reg);
    stream << asm_prefix.at(context.get_instruction_state()) <<"li " << dest_reg << ", " << linear_index + 1 << std::endl;
    stream << asm_prefix.at(context.get_instruction_state()) << "slli " << dest_reg << ", " << dest_reg << ", " << types_mem_shift.at(type) << std::endl;

    context.pop_operation_type();
}

std::vector<int> ArrayIndexAccess::get_linear_index(Context &context) const{
    const ArrayIndexAccess *array = this;

    std::vector<int> indexes;

    const Identifier *variable = dynamic_cast<const Identifier *>(index_.get());
    const IntConstant *constant = dynamic_cast<const IntConstant*>(index_.get());

    while(array != nullptr){

        variable = dynamic_cast<const Identifier *>(array->index_.get());
        constant = dynamic_cast<const IntConstant*>(array->index_.get());

        if(variable != nullptr){
            indexes.push_back(variable->get_index(context));
        }
        else if(constant != nullptr){
            indexes.push_back(constant->get_val());
        }

        array = dynamic_cast<const ArrayIndexAccess *>(array->identifier_.get());
    }

    std::reverse(indexes.begin(),indexes.end());

    return indexes;

}

void ArrayIndexAccess::Print(std::ostream &stream) const
{
    identifier_->Print(stream);
    stream << "[";
    index_->Print(stream);
    stream << "]";
}

Type ArrayIndexAccess::GetType(Context &context) const
{
    Variable variable = context.get_variable(GetId());
    return variable.get_type();
}

bool ArrayIndexAccess::isPointerOp(Context &context) const
{
    Variable variable = context.get_variable(GetId());

    if (!variable.is_pointer())
    {
        return false;
    }

    return variable.get_dereference_num() > 1;
}
}//namespace ast
