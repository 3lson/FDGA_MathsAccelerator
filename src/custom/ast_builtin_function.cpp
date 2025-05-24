#include "../../include/custom/ast_builtin_function.hpp"
#include <iostream>

namespace ast {

Type BuiltInFunction::GetType(Context& context) const {
    // Assume type stays the same after applying fabsf
    const Operand *args = dynamic_cast<const Operand*>(argument_.get());
    return args->GetType(context);
}

void BuiltInFunction::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const {
    Type type = GetType(context);

    if (func_name_ != "fabsf") {
        throw std::runtime_error("Unsupported builtin function: " + func_name_);
    }

    context.push_operation_type(type);

    std::string arg_reg = context.get_register(type);

    argument_->EmitRISC(stream, context, arg_reg);

    if (dest_reg == "zero") {
        dest_reg = context.get_register(type);
    }

    stream << "abs " << dest_reg << ", " << arg_reg << std::endl;

    context.deallocate_register(arg_reg);
    context.pop_operation_type();
}

void BuiltInFunction::Print(std::ostream& stream) const {
    stream << func_name_ << "(";
    argument_->Print(stream);
    stream << ")";
}

bool BuiltInFunction::isPointerOp(Context &context) const {
    //empty
    (void)context;
    return false;
}

} // namespace ast
