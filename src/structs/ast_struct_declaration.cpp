#include "../../include/structs/ast_struct_declaration.hpp"

namespace ast{

void StructDeclaration::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const {
    (void)stream;
    (void)dest_reg;
    std::cout << "Declaring struct variable: " << *variable_name_ << std::endl;
    std::cout << "Passing in struct type: " << *struct_type_ << std::endl;

    Type structType = context.get_or_create_struct_type(*struct_type_);
    int size = context.get_struct_size(structType);
    int offset = context.get_stack_offset();
    context.increase_stack_offset(size);

    Variable variable(false, false, size, Type::_STRUCT, offset, 0, *struct_type_);
    context.define_variable(*variable_name_, variable);

    std::cout << "Allocated " << size << " bytes on the stack for struct variable " << *variable_name_ << std::endl;
}

void StructDeclaration::Print(std::ostream& stream) const {
    stream << "struct " << *struct_type_ << " ";
}

std::string StructDeclaration::GetId() const{
    return *variable_name_;
}

Type StructDeclaration::GetType() const{
    return Type::_STRUCT;
}


}//namespace ast
