#include "../../include/control_flow/ast_forloop.hpp"

namespace ast{
void ForStatement::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const{
    std::string start_label = context.create_label("for_start");
    std::string end_label = context.create_label("for_end");
    std::string update_label = context.create_label("for_update");

    context.push_start_label(update_label);
    context.push_end_label(end_label);

    if (init_){
        init_->EmitRISC(stream, context, dest_reg);
    }

    stream << start_label << ":" << std::endl;

    if (condition_){
        std::string condition_reg = context.get_register(Type::_INT);
        condition_->EmitRISC(stream, context, condition_reg);
        stream << "beqz " << condition_reg << ", " << end_label << std::endl;
        context.deallocate_register(condition_reg);
     }

     body_->EmitRISC(stream, context, dest_reg);

     stream << update_label << ":" << std::endl;
     if (update_){
        update_->EmitRISC(stream, context, dest_reg);
     }

     stream << "j " << start_label << std::endl;
     stream << end_label << ":" << std::endl;

     context.pop_start_label();
     context.pop_end_label();

}

void ForStatement::Print(std::ostream& stream) const {
    stream << "for (";
    if (init_) {
        init_->Print(stream);
    }
    stream << "; ";
    if (condition_) {
        condition_->Print(stream);
    }
    stream << "; ";
    if (update_) {
        update_->Print(stream);
    }
    stream << ") ";
    body_->Print(stream);
}

}//namespace ast
