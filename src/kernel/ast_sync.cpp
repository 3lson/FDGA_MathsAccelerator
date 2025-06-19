#include "../../include/kernel/ast_sync.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <stack>

namespace ast {

void SyncStatement::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {

    (void) dest_reg;

    std::string endsync_label = context.create_label("endsync");

    stream << "sync " << endsync_label << std::endl;
    
    std::vector<Warp>& warp_file = context.get_warp_file();
    Warp& active_warp = warp_file[0];

    for(auto& warp : warp_file){
        if(warp.get_activity() == true){
            active_warp = warp;
            break;
        }
    }

    //context.set_instruction_state(Kernel::_SCALAR);
    context.assign_reg_manager(active_warp.get_warp_file());

    std::string warp_condition = context.get_register(Type::_INT);
    std::vector<std::string> labels;


    //warp_number checks
    for(size_t i = 1; i < warp_file.size(); i++){
        std::string warp_label = context.create_label("warp");
        labels.push_back(warp_label);
        stream << "s.seqi " << warp_condition << ", s24" << ", " << i << std::endl;
        stream << "s.beqo " << warp_condition << ", " <<  warp_label << std::endl;
    }

    context.deallocate_register(warp_condition);

    

    for(size_t i = 0; i < warp_file.size(); i++){
        Warp& warp = warp_file[i];
        ScalarRegisterFile& reg_file = warp.get_warp_file();
        context.assign_reg_manager(reg_file);
        std::string addr = context.get_register(Type::_INT);

        for(int z = 0; z < 32; z++){
            if(!reg_file.get_register_by_id(z).isAvailable()){
                int address = warp.get_warp_offset() - (z+1)*4;
                std::string tmp_name = reg_file.get_register_name(z);
                
                stream << "s.li " << addr << ", " << address << std::endl;
                stream << "s.sw " << tmp_name << ", " << 0 << "(" << addr << ")" << std::endl;
            }
        }

        for(int r = 33; r < 64; r++){
            if(!reg_file.get_register_by_id(r).isAvailable()){
                int address = warp.get_warp_offset() - (r+1)*4;
                std::string tmp_name = reg_file.get_register_name(r);
                
                stream << "s.li " << addr << ", " << address << std::endl;
                stream << "s.fsw " << tmp_name << ", " << 0 << "(" << addr << ")" << std::endl;
            } 
        }

        context.deallocate_register(addr);
        
        //iterates for each thread
        for(int k = 0; k < warp.get_size(); k++){
            Thread& thread = warp.return_thread(k);
            VectorRegisterFile& thread_file = thread.get_thread_file();
            context.assign_reg_manager(thread_file);
            std::string addr = context.get_register(Type::_INT);

            for(int l = 0; l < 32; l++){
                int address = thread.get_offset() - (l+1)*4;
                std::string tmp_name = thread_file.get_register_name(l);
                
                stream << "v.li " << addr << ", " << address << std::endl;
                stream << "v.sw " << tmp_name << ", " << 0 << "(" << addr << ")" << std::endl;

            }

            for(int m = 32; m < 64; m++ ){
                int address = thread.get_offset() - (m+1)*4;
                std::string tmp_name = thread_file.get_register_name(m);

                stream << "v.li " << addr << ", " << address << std::endl;
                stream << "v.fsw " << tmp_name << ", " << 0 << "(" << addr << ")" << std::endl;

            }

            context.deallocate_register(addr);
        }

        warp.set_activity(false);

        //load next_warp regs

        Warp& next_warp = warp_file[(i+1) % warp_file.size()];
        next_warp.set_activity(true);
        ScalarRegisterFile& new_reg_file = next_warp.get_warp_file();
        context.assign_reg_manager(new_reg_file);
        std::string next_addr = context.get_register(Type::_INT);

        for(int p = 0; p < 32; p++){
            if(!new_reg_file.get_register_by_id(p).isAvailable()){
                int address = next_warp.get_warp_offset() - (p+1)*4;
                std::string tmp_name = new_reg_file.get_register_name(p);
                
                stream << "s.li " << next_addr << ", " << address << std::endl;
                stream << "s.lw " << tmp_name << ", " << 0 << "(" << next_addr << ")" << std::endl;
            }
        }

        for(int o = 33; o < 64; o++){
            if(!new_reg_file.get_register_by_id(o).isAvailable()){
                int address = next_warp.get_warp_offset() - (o+1)*4;
                std::string tmp_name = new_reg_file.get_register_name(o);
                
                stream << "s.li " << next_addr << ", " << address << std::endl;
                stream << "s.flw " << tmp_name << ", " << 0 << "(" << next_addr << ")" << std::endl;
            } 
        }

        context.deallocate_register(next_addr);

        //iterates for each thread
        for(int w = 0; w < next_warp.get_size(); w++){
            Thread& thread = next_warp.return_thread(w);
            VectorRegisterFile& thread_file = thread.get_thread_file();
            context.assign_reg_manager(thread_file);
            std::string addr = context.get_register(Type::_INT);

            for(int n = 0; n < 32; n++){
                int address = thread.get_offset() - (n+1)*4;
                std::string tmp_name = thread_file.get_register_name(n);
                
                stream << "v.li " << addr << ", " << address << std::endl;
                stream << "v.lw " << tmp_name << ", " << 0 << "(" << addr << ")" << std::endl;

            }

            for(int m = 32; m < 64; m++ ){
                int address = thread.get_offset() - (m+1)*4;
                std::string tmp_name = thread_file.get_register_name(m);

                stream << "v.li " << addr << ", " << address << std::endl;
                stream << "v.flw " << tmp_name << ", " << 0 << "(" << addr << ")" << std::endl;

            }

            context.deallocate_register(addr);
        
        }

        if(static_cast<int>(i+1) < static_cast<int>(warp_file.size())){
            context.assign_reg_manager(next_warp.return_thread(0).get_thread_file());
            stream << "s.jalr zero, s25, 0" << std::endl;  
            stream << labels[i] << ": " << std::endl;
        }
    }

    stream << endsync_label << ": "  << std::endl;
}

void SyncStatement::Print(std::ostream& stream) const {
    stream << "sync;" <<  std::endl;
}

}
