#include "../../include/kernel/ast_kernel.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <stack>

namespace ast {

void KernelStatement::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {
    (void)stream;
    (void)context;
    (void)dest_reg;

    std::string start_kernel_label = context.create_label("start");

    InitializeKernel(start_kernel_label, stream, context);
    InitializeFirstWarp(stream, context);

    stream << start_kernel_label << ": " << std::endl;

    compound_statement_->EmitElsonV(stream,context,dest_reg); 
}

void KernelStatement::Print(std::ostream& stream) const {

    const IntConstant *threads = dynamic_cast <const IntConstant *>(threads_.get());
    int thread_total = threads->get_val();

    stream << "kernel (";
    stream << thread_total;
    stream << "){" << std::endl;
    compound_statement_->Print(stream);
    stream << "}" << std::endl;
}

void KernelStatement::InitializeFirstWarp(std::ostream& stream, Context& context) const{
    std::vector<Warp>& warp_file = context.get_warp_file();

    Warp& first_warp = warp_file[0];
    int thread_num = first_warp.get_size();
    int warp_offset = first_warp.get_warp_offset();

    std::string offset_reg = context.get_register(Type::_INT);

    int address = warp_offset - ((3+1)*4);

    //load gp
    int address = warp_offset - ((3+1)*4);
    stream << asm_prefix.at(context.get_instruction_state()) << "li " << offset_reg << ", " << address << std::endl;
    stream << asm_prefix.at(context.get_instruction_state()) << "lw gp, " << 0 << "(" << offset_reg << ")" << std::endl;


    //load s24 (warp-id)
    address = warp_offset - ((24+1)*4);
    stream << asm_prefix.at(context.get_instruction_state()) << "li " << offset_reg << ", " << address << std::endl;
    stream << asm_prefix.at(context.get_instruction_state()) << "lw s24, " << 0 << "(" << offset_reg << ")" << std::endl;

    //load s25 (PC value)
    address = warp_offset - ((25+1)*4);
    stream << asm_prefix.at(context.get_instruction_state()) << "li " << offset_reg << ", " << address << std::endl;
    stream << asm_prefix.at(context.get_instruction_state()) << "lw s25, " << 0 << "(" << offset_reg << ")" << std::endl;

    //load s26 execution mask
    address = warp_offset - ((26+1)*4);
    stream << asm_prefix.at(context.get_instruction_state()) << "li " << offset_reg << ", " << address << std::endl;
    stream << asm_prefix.at(context.get_instruction_state()) << "lw s26, " << 0 << "(" << offset_reg << ")" << std::endl;

    context.deallocate_register(offset_reg);

    context.set_instruction_state(Kernel::_VECTOR);
    context.assign_reg_manager(first_warp.return_thread(0).get_thread_file());
    
    //load threads:

    for(int i = 0; i < first_warp.get_size(); i++){
        Thread& load_thread = first_warp.return_thread(i);
        int thread_offset = load_thread.get_offset();
        int address;

        std::string thread_addr = context.get_register(Type::_INT);

        //load sp
        address = thread_offset - ((2+1)*4);
        stream << asm_prefix.at(context.get_instruction_state()) << "li " << thread_addr << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "lw sp, " << 0 << "(" << thread_addr << ")" << std::endl;

        //load gp
        address = thread_offset - ((3+1)*4);
        stream << asm_prefix.at(context.get_instruction_state()) << "li " << thread_addr << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "lw gp, " << 0 << "(" << thread_addr << ")" << std::endl;

        //load v0 (stack header)
        address = thread_offset - ((6+1)*4);
        stream << asm_prefix.at(context.get_instruction_state()) << "li " << thread_addr << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "lw v0, " << 0 << "(" << thread_addr << ")" << std::endl;

        //load v26 (global thread id)
        address = thread_offset - ((31+1)*4);
        stream << asm_prefix.at(context.get_instruction_state()) << "li " << thread_addr << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "lw v26, " << 0 << "(" << thread_addr << ")" << std::endl;

    }
}


void KernelStatement::InitializeKernel(std::string start_kernel_label, std::ostream& stream, Context& context) const{
    const IntConstant *threads = dynamic_cast <const IntConstant *>(threads_.get());
    int thread_total = threads->get_val();
    int warp_size = context.get_warp_size();

    int num_warps = (thread_total + warp_size - 1) / warp_size;

    std::vector<Warp>& warp_file = context.get_warp_file(); //direclty modifies warp_file inside context
    warp_file.reserve(num_warps);
    for(int i = 0; i < num_warps; i++){
        thread_total -= warp_size;
        int thread;
        if(thread_total >= 0){
            thread = warp_size;
        }
        else{
            thread = std::abs(thread_total);
        }

        bool active = false;
        if(i == 0){
            active = true;
        }

        warp_file.emplace_back(i,thread,active);
    }

    //warp register file set the same main_cpu_reg file
    warp_file[0].initialise_from_cpu(context.get_main_cpu_regs());

    //sets warp & thread offsets    
    int total_size = KernelStackSize(context) + context.get_warp_offset();;

    int file_offset = 64 * 4;

    for(auto& warp : warp_file){
        warp.set_warp_offset(total_size);

        int tmp_total = total_size;
        for(int i = 0; i < warp.get_size(); i++){
            tmp_total -= (i+1) * file_offset;
            Thread& threadref = warp.return_thread(i);
            threadref.set_offset(tmp_total);
        }

        total_size -= file_offset;
        total_size -= file_offset * warp.get_size();
    }

    std::string tmp_reg = context.get_register(Type::_INT);
    std::string address_reg = context.get_register(Type::_INT);


    //set offset addresses (gp)
    for(auto& warp: warp_file){
        int warp_offset = warp.get_warp_offset();
        int address = warp_offset - (3+1)*4;

        stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "li " << tmp_reg << ", " << warp_offset << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "sw " << tmp_reg << ", " << 0 << "(" << address_reg << ")" << std::endl; 

        //stores thread offset gp
        for(int i = 0; i < warp.get_size(); i++){
            Thread& thread = warp.return_thread(i);

            int thread_offset = thread.get_offset();
            int reg_address = thread_offset - (3+1)*4;

            stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << reg_address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "li " << tmp_reg << ", " << thread_offset << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "sw "<< tmp_reg << ", " << 0 << "(" << address_reg << ")" << std::endl; 
        }
        
    }


    //set execution mask registers (s26)
    for(auto& warp: warp_file){
        int warp_offset = warp.get_warp_offset();
        int address = warp_offset - (31+1)*4;

        int32_t execution_mask = warp.get_execution_mask();

        stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "li " << tmp_reg << ", " << execution_mask << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "sw " << tmp_reg << ", " << 0 << "(" << address_reg << ")" << std::endl; 
        
    }


    //set warpid (s24) + global_thread_id
    for(auto& warp: warp_file){
        int warp_offset = warp.get_warp_offset();
        int address = warp_offset - (29+1)*4;

        int warp_id = warp.get_id();

        stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "li " << tmp_reg << ", " << warp_id << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "sw " << tmp_reg << ", " << 0 << "(" << address_reg << ")" << std::endl; 
        
        for(int i = 0; i < warp.get_size(); i++){
            Thread& thread = warp.return_thread(i);

            int thread_offset = thread.get_offset();
            int reg_address = thread_offset - (31+1)*4;
            int global_thread_id = thread.get_global_thread_id();

            stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << reg_address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "li " << tmp_reg << ", " << global_thread_id << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "sw "<< tmp_reg << ", " << 0 << "(" << address_reg << ")" << std::endl; 
        }
    }

    //preserving stack pointer
    for(auto& warp: warp_file){
        int warp_offset = warp.get_warp_offset();
        int address = warp_offset - (2+1)*4;

        stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "sw sp, " << 0 << "(" << address_reg << ")" << std::endl; 

        for(int i = 0; i < warp.get_size(); i++){
            Thread& thread = warp.return_thread(i);

            int thread_offset = thread.get_offset();
            int reg_address = thread_offset - (2+1)*4;

            stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << reg_address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "sw sp, " << 0 << "(" << address_reg << ")" << std::endl; 
        }
        
    }

    //preserving return pointer
    for(auto& warp: warp_file){
        int warp_offset = warp.get_warp_offset();
        int address = warp_offset - (1+1)*4;


        stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "sw ra, " << 0 << "(" << address_reg << ")" << std::endl; 
        
    }

    //preserving stack header
    for(auto& warp: warp_file){
        int warp_offset = warp.get_warp_offset();
        int address = warp_offset - (6+1)*4;

        stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "sw s0, " << 0 << "(" << address_reg << ")" << std::endl;

        for(int i = 0; i < warp.get_size(); i++){
            Thread& thread = warp.return_thread(i);

            int thread_offset = thread.get_offset();
            int reg_address = thread_offset - (6+1)*4;

            stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << reg_address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "sw s0, " << 0 << "(" << address_reg << ")" << std::endl; 
        }
        
    }

    //preserving PC value of kernel_start
    for(auto& warp: warp_file){
        int warp_offset = warp.get_warp_offset();
        int address = warp_offset - (30+1)*4;

        stream << "sync " << start_kernel_label << std::endl; //stores the PC value in s25
        stream << asm_prefix.at(context.get_instruction_state()) << "li " << address_reg << ", " << address << std::endl;
        stream << asm_prefix.at(context.get_instruction_state()) << "sw s25, " << 0 << "(" << address_reg << ")" << std::endl; 
        
    }

    context.deallocate_register(tmp_reg);
    context.deallocate_register(address_reg);

}

int KernelStatement::KernelStackSize(Context& context) const{

    int type_offset = 4; // 4 bytes for 32 bit registers
    int total = 0;
    int register_file_total = 64 * type_offset;

    for(auto& warp : context.get_warp_file()){
        total += register_file_total;
        total +=  warp.get_size() * register_file_total;
    }

    return total;

}

}
