#include "../../include/functions/ast_function_definition.hpp"
namespace ast {

void FunctionDefinition::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    auto direct_declarator_ = dynamic_cast<const Declarator *>(declarator_.get());
    if (!direct_declarator_) {
        throw std::runtime_error("Failed to cast declarator_ to DirectDeclarator");
    }

    std::string function_name = direct_declarator_->GetId();

    const Specifier *return_type_specifier = dynamic_cast<const Specifier *>(declaration_specifiers_.get());
    if (!return_type_specifier) {
        throw std::runtime_error("Failed to cast declaration_specifiers_ to Specifier");
    }
    bool return_is_pointer = direct_declarator_->isPointer();
    Type return_type = return_type_specifier->GetType();

    //Hard coding according to assembler directives
    stream << ".text" << std::endl;
    stream << ".globl " << function_name << std::endl;
    stream << ".align " << types_mem_shift.at(return_type) << std::endl;
    stream << ".type " << function_name << ", @function" << std::endl;
    stream << function_name << ":" << std::endl;

    ReturnValue return_value(return_is_pointer, false, return_type, direct_declarator_->get_deref());
    //vector for my arguments to pull the arguments out then processing then in "pameter_definition"
    std::vector<Parameter> parameters = direct_declarator_->get_param(context);
    Function function(return_value, parameters);
    context.define_function(function_name, function);

    if (compound_statement_ != nullptr)
    {
        context.create_scope();
        if (return_type != Type::_VOID){
            context.push_operation_type(return_type);
        }

        if (!compound_statement_) {
            throw std::runtime_error("Error: compound_statement_ is null in FunctionDefinition::EmitRISC");
        }

        const CompoundStatement *compound_statement = dynamic_cast<const CompoundStatement*>(compound_statement_.get());
        context.increase_stack_offset(8);
        int initial_offset = 8 + direct_declarator_->get_offset();

        int stack_allocated_space = compound_statement->get_offset(context) + initial_offset + 128;
        stack_allocated_space = stack_allocated_space + (4 - stack_allocated_space % 4);

        //Added for more than 8 params

        if (parameters.size() > 8){
            int additional_args_space = (parameters.size() -8) * 4;
            stack_allocated_space += additional_args_space;
        }
        context.set_stack_offset(stack_allocated_space);

        stream << "addi sp, sp, -" << stack_allocated_space << std::endl;
        stream << "sw ra, " << stack_allocated_space - 4 << "(sp)" <<std::endl;
        stream << "sw s0, " << stack_allocated_space - 8 << "(sp)" <<std::endl;
        stream << "addi s0, sp, " << stack_allocated_space <<std::endl;
        direct_declarator_->store_param(stream, context, dest_reg);

        compound_statement_->EmitRISC(stream, context, dest_reg);

        stream << context.get_function_end() << ":" << std::endl;
        stream << "lw s0, "<< stack_allocated_space - 8 << "(sp)" << std::endl;
        stream << "lw ra, "<< stack_allocated_space - 4 << "(sp)" <<std::endl;
        stream << "addi sp, sp, " << stack_allocated_space <<std::endl;
        stream << "ret" << std::endl;

        context.pop_scope();
        if (return_type != Type::_VOID){
            context.pop_operation_type();
        }
    context.exit_function();
    stream << ".size " << function_name << ", .-" << function_name <<std::endl;
    }
}

void FunctionDefinition::Print(std::ostream &stream) const
{
    declaration_specifiers_->Print(stream);
    stream << " ";

    declarator_->Print(stream);
    stream << "() {" << std::endl;

    if (compound_statement_ != nullptr)
    {
        compound_statement_->Print(stream);
    }
    stream << "}" << std::endl;
}

}//namespace ast
