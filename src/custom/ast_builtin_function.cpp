#include "../../include/custom/ast_builtin_function.hpp"
#include <iostream>

namespace ast {

Type BuiltInFunction::GetType(Context& context) const {
    if (func_name_ == "sync") {
        return Type::_VOID;
    }

    const Operand *args = dynamic_cast<const Operand*>(argument_.get());
    return args->GetType(context);
}

void BuiltInFunction::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {
    if (func_name_ == "fabsf") {
        Type type = GetType(context);
        context.push_operation_type(type);
        std::string arg_reg = context.get_register(type);

        argument_->EmitElsonV(stream, context, arg_reg);

        if (dest_reg == "zero") {
            dest_reg = context.get_register(type);
        }

        stream << asm_prefix.at(context.get_instruction_state()) <<"fabs.s " << dest_reg << ", " << arg_reg << std::endl;

        context.deallocate_register(arg_reg);
        context.pop_operation_type();
    } else if (func_name_ == "sync") {
        stream << "sync" << std::endl;
    } else {
        throw std::runtime_error("Unsupported builtin function: " + func_name_);
    }
}


void BuiltInFunction::Print(std::ostream& stream) const {
    stream << func_name_;
    if (argument_) {
        stream << "(";
        argument_->Print(stream);
        stream << ")";
    }
}


bool BuiltInFunction::isPointerOp(Context &context) const {
    //empty
    (void)context;
    return false;
}

} // namespace ast
