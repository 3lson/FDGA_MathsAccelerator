#include "../../include/control_flow/ast_ifelse.hpp"
#include <iostream>

namespace ast {

void IfStatement::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const {
    std::string condition_reg = context.get_register(Type::_INT);
    condition_->EmitRISC(stream, context, condition_reg);

    std::string else_label = context.create_label("else");
    std::string end_label = context.create_label("end_if");

    if (is_ternary_) {
        stream << "beqz " << condition_reg << ", " << else_label << std::endl;
        then_branch_->EmitRISC(stream, context, dest_reg);
        stream << "j " << end_label << std::endl;

        stream << else_label << ":" << std::endl;
        else_branch_->EmitRISC(stream, context, dest_reg);

        stream << end_label << ":" << std::endl;
    } else {
        stream << "beqz " << condition_reg << ", " << else_label << std::endl;
        then_branch_->EmitRISC(stream, context, dest_reg);

        stream << "j " << end_label << std::endl;
        stream << else_label << ":" << std::endl;

        if (else_branch_) {
            else_branch_->EmitRISC(stream, context, dest_reg);
        }

        stream << end_label << ":" << std::endl;
    }

    context.deallocate_register(condition_reg);
}

void IfStatement::Print(std::ostream& stream) const {
    if (is_ternary_) {
        stream << "(";
        condition_->Print(stream);
        stream << " ? ";
        then_branch_->Print(stream);
        stream << " : ";
        else_branch_->Print(stream);
        stream << ")";
    } else {
        stream << "if (";
        condition_->Print(stream);
        stream << ") ";
        then_branch_->Print(stream);
        if (else_branch_) {
            stream << " else ";
            else_branch_->Print(stream);
        }
    }
}

}
