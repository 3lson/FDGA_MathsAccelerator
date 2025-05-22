#include "../../include/control_flow/ast_while.hpp"
#include <iostream>

namespace ast {

void WhileStatement::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const {
    std::string start_label = context.create_label("loop_start");
    std::string end_label = context.create_label("loop_end");
    context.push_start_label(start_label);
    context.push_end_label(end_label);

    stream << start_label << ":" << std::endl;

    std::string condition_reg = context.get_register(Type::_INT);

    condition_->EmitRISC(stream, context, condition_reg);

    stream << "beqz " << condition_reg << ", " << end_label << std::endl;

    body_->EmitRISC(stream, context, dest_reg);

    stream << "j " << start_label << std::endl;

    stream << end_label << ":" << std::endl;

    context.deallocate_register(condition_reg);

    context.pop_start_label();
    context.pop_end_label();
}

void WhileStatement::Print(std::ostream& stream) const {
    stream << "while (";
    condition_->Print(stream);
    stream << ") ";
    body_->Print(stream);
}

} // namespace ast
