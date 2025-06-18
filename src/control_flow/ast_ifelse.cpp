#include "../../include/control_flow/ast_ifelse.hpp"
#include <iostream>

namespace ast {

void IfStatement::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {
    std::string condition_reg = context.get_register(Type::_INT);
    condition_->EmitElsonV(stream, context, condition_reg);


    if(context.get_instruction_state() == Kernel::_SCALAR){
        std::string else_label = context.create_label("else");
        std::string end_label = context.create_label("end_if");

        if (is_ternary_) {
            stream << "s.beqz " << condition_reg << ", " << else_label << std::endl;
            then_branch_->EmitElsonV(stream, context, dest_reg);
            stream << "s.j " << end_label << std::endl;

            stream << else_label << ":" << std::endl;
            else_branch_->EmitElsonV(stream, context, dest_reg);

            stream << end_label << ":" << std::endl;
        } else {
            stream << "s.beqz " << condition_reg << ", " << else_label << std::endl;
            then_branch_->EmitElsonV(stream, context, dest_reg);

            stream << "s.j " << end_label << std::endl;
            stream << else_label << ":" << std::endl;

            if (else_branch_) {
                else_branch_->EmitElsonV(stream, context, dest_reg);
            }

            stream << end_label << ":" << std::endl;
        }
    }
    else{
        then_branch_->EmitElsonV(stream, context, dest_reg);
        stream << "s.neg s26, s26" << std::endl; //invert the exeuction mask
        //this will also invert non-existant lanes too most probably need to fix this shit
        else_branch_->EmitElsonV(stream, context, dest_reg);
        std::vector<Warp>& warps = context.get_warp_file();
        
        Warp& target_warp = warps[0];
        for(auto& warp : warps){
            if(warp.get_activity() == true){
                target_warp = warp;
                break;
            }
        }

        //reset execution mask
        stream << "s.li s26, " << target_warp.get_execution_mask() << std::endl;

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
