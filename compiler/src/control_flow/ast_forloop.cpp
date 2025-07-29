#include "../../include/control_flow/ast_forloop.hpp"

namespace ast{
void ForStatement::EmitElsonV(std::ostream &stream, Context &context, std::string dest_reg) const{
    std::string start_label = context.create_label("for_start");
    std::string end_label = context.create_label("for_end");
    std::string update_label = context.create_label("for_update");

    context.push_start_label(update_label);
    context.push_end_label(end_label);

    if(context.get_instruction_state() == Kernel::_SCALAR){
        if (init_){
            init_->EmitElsonV(stream, context, dest_reg);
        }

        stream << start_label << ":" << std::endl;

        if (condition_){
            std::string condition_reg = context.get_register(Type::_INT);
            condition_->EmitElsonV(stream, context, condition_reg);
            stream << "s.beqz " << condition_reg << ", " << end_label << std::endl;
            context.deallocate_register(condition_reg);
        }

        body_->EmitElsonV(stream, context, dest_reg);

        stream << update_label << ":" << std::endl;
        if (update_){
            update_->EmitElsonV(stream, context, dest_reg);
        }

        stream << "s.j " << start_label << std::endl;
        stream << end_label << ":" << std::endl;
    }
    else{

        context.set_instruction_state(Kernel::_SCALAR);
        std::vector<Warp>& warp_file = context.get_warp_file();
        
        Warp& active_warp = warp_file[0];
        for(auto& warp : warp_file){
            if(warp.get_activity() == true){
                active_warp = warp;
                break;
            }
        }

        context.assign_reg_manager(active_warp.get_warp_file());
        if (init_){
            init_->EmitElsonV(stream, context, dest_reg);
        }

        stream << start_label << ":" << std::endl;

        if (condition_){
            std::string condition_reg = context.get_register(Type::_INT);
            condition_->EmitElsonV(stream, context, condition_reg);
            stream << "s.beqz " << condition_reg << ", " << end_label << std::endl;
            context.deallocate_register(condition_reg);
        }

        context.set_instruction_state(Kernel::_VECTOR);
        context.assign_reg_manager(active_warp.return_thread(0).get_thread_file());

        body_->EmitElsonV(stream, context, dest_reg);

        context.set_instruction_state(Kernel::_SCALAR);
        context.assign_reg_manager(active_warp.get_warp_file());


        stream << update_label << ":" << std::endl;
        if (update_){
            update_->EmitElsonV(stream, context, dest_reg);
        }

        stream << "s.j " << start_label << std::endl;
        stream << end_label << ":" << std::endl;

        context.set_instruction_state(Kernel::_VECTOR);
        context.assign_reg_manager(active_warp.return_thread(0).get_thread_file());
    }

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
