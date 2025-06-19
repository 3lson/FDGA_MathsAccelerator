#include "../../include/control_flow/ast_ifelse.hpp"
#include <iostream>

namespace ast {

// void IfStatement::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {
//     std::string condition_reg = context.get_register(Type::_INT);
//     condition_->EmitElsonV(stream, context, condition_reg);

//     if(context.get_instruction_state() == Kernel::_SCALAR){
//         std::string else_label = context.create_label("else");
//         std::string end_label = context.create_label("end_if");

//         if (is_ternary_) {
//             stream << "s.beqz " << condition_reg << ", " << else_label << std::endl;
//             then_branch_->EmitElsonV(stream, context, dest_reg);
//             stream << "s.j " << end_label << std::endl;

//             stream << else_label << ":" << std::endl;
//             else_branch_->EmitElsonV(stream, context, dest_reg);

//             stream << end_label << ":" << std::endl;
//         } else {
//             stream << "s.beqz " << condition_reg << ", " << else_label << std::endl;
//             then_branch_->EmitElsonV(stream, context, dest_reg);

//             stream << "s.j " << end_label << std::endl;
//             stream << else_label << ":" << std::endl;

//             if (else_branch_) {
//                 else_branch_->EmitElsonV(stream, context, dest_reg);
//             }

//             stream << end_label << ":" << std::endl;
//         }
//     }
//     else{
//         then_branch_->EmitElsonV(stream, context, dest_reg);
//         stream << "s.neg s26, s26" << std::endl; //invert the exeuction mask
//         //this will also invert non-existant lanes too most probably need to fix this shit

//         if (else_branch_){
//             else_branch_->EmitElsonV(stream, context, dest_reg);
//         }
        
//         std::vector<Warp>& warps = context.get_warp_file();
        
//         Warp& target_warp = warps[0];
//         for(auto& warp : warps){
//             if(warp.get_activity() == true){
//                 target_warp = warp;
//                 break;
//             }
//         }

//         //reset execution mask
//         stream << "s.li s26, " << target_warp.get_execution_mask() << std::endl;

//     }


//     context.deallocate_register(condition_reg);
// }

void IfStatement::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {

    if(context.get_instruction_state() == Kernel::_SCALAR){
        std::string condition_reg = context.get_register(Type::_INT);
        condition_->EmitElsonV(stream, context, condition_reg);
        // Scalar path remains unchanged
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
        context.deallocate_register(condition_reg);
    }
    else {
        // SIMPLIFIED VECTOR DIVERGENCE HANDLING
        std::vector<Warp>& warps = context.get_warp_file();
        Warp& active_warp = GetActiveWarp(warps);
        uint32_t original_mask = active_warp.get_execution_mask();

        std::string condition_reg = context.get_register(Type::_INT);
        condition_->EmitElsonV(stream, context, condition_reg);

        
        // Create condition mask
        std::string mask_reg = context.get_divergence_safe_register(Type::_INT);

        // Execute then branch
        then_branch_->EmitElsonV(stream, context, dest_reg);
        
        // === ELSE BRANCH ===
        if (else_branch_) {
            // Invert mask for else branch (condition == false)
            stream << "s.neg s26, s26" << std::endl;
            
            // Execute else branch
            else_branch_->EmitElsonV(stream, context, dest_reg);
        }
        
        // === CONVERGENCE ===
        // Restore original execution mask
        stream << "s.li s26, " << original_mask << std::endl;
        
        // Clean up mask register from all threads
        context.deallocate_from_all_threads(mask_reg);
        context.deallocate_register(condition_reg);
    }

    
}

Warp& IfStatement::GetActiveWarp(std::vector<Warp>& warps) const {
    for(auto& warp : warps) {
        if(warp.get_activity()) {
            return warp;
        }
    }
    return warps[0]; // fallback
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
