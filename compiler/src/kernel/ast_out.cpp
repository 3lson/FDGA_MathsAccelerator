#include "../../include/kernel/ast_out.hpp"
#include "../../include/symbols/ast_identifier.hpp"
#include "../../include/arrays/ast_array_declaration.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <stack>

namespace ast {

void OutStatement::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {

    (void) dest_reg;
    
    const ArrayDeclaration * array = dynamic_cast<const ArrayDeclaration*>(declarator_.get());
    const Identifier * identifier = dynamic_cast<const Identifier*>(declarator_.get());

    std::string name;
    if(array != nullptr){
        name = array->GetId();
    }
    else{
        name = identifier->GetId();
    }
    
    Variable& variable = context.get_variable(name);

    int offset = variable.get_offset();
    Type type = variable.get_type();
    int bytes = types_size.at(type);
    int size = variable.get_array_size();

    int current_out_offset = context.get_current_out_offset();
    int variable_out_offset = variable.get_out_offset();

    std::string addr = context.get_register(Type::_INT);
    std::string tmp = context.get_register(Type::_INT);

    if(variable_out_offset == 0){

        variable.set_out_offset(current_out_offset);

        for(int i = 0; i < size; i++){
            int start_location = offset - (i+1)*bytes;
            context.set_current_out_offset((i+1)*bytes);
            int end_location = context.get_current_out_offset();
            stream << "s.li " << addr << ", " << start_location << std::endl;
            stream << "s.lw " << tmp << ", 0" << "(" << addr << ")" << std::endl;
            stream << "s.li " << addr << ", " << end_location << std::endl;
            stream << "s.sw " << tmp << ", 0" << "(" << addr << ")" << std::endl;

        }
    }
    else{

        for(int i = 0; i < size; i++){
            int start_location = offset - (i+1)*bytes;
            int end_location = variable_out_offset - (i+1)*bytes;

            stream << "s.li " << addr << ", " << start_location << std::endl;
            stream << "s.lw " << tmp << ", 0" << "(" << addr << ")" << std::endl;
            stream << "s.li " << addr << ", " << end_location << std::endl;
            stream << "s.sw " << tmp << ", 0" << "(" << addr << ")" << std::endl;
        }

    }

    context.deallocate_register(addr);
    context.deallocate_register(tmp);

}

void OutStatement::Print(std::ostream& stream) const {
    stream << "OUT ";
    declarator_->Print(stream);
    stream << ";" << std::endl;

}

}