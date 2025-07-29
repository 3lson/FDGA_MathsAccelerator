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

    std::string start_kernel_label = context.create_label("kernel_start");
    std::string warp_switch_label = context.create_label("warp_switch");
    std::string kernel_end_label = context.create_label("kernel_end");

    // Initialize all warps and kernel state
    InitializeKernel(start_kernel_label, stream, context);
    
    // Load the first warp and jump to kernel execution
    InitializeFirstWarp(stream, context);
    stream << "s.j " << start_kernel_label << std::endl;

    // === WARP SWITCHING SECTION ===
    stream << warp_switch_label << ":" << std::endl;
    EmitWarpSwitchLogic(stream, context, start_kernel_label, kernel_end_label);

    // === MAIN KERNEL EXECUTION ===
    stream << start_kernel_label << ":" << std::endl;
    compound_statement_->EmitElsonV(stream, context, dest_reg);
    
    // After kernel execution, jump to warp switching
    stream << "s.j " << warp_switch_label << std::endl;

    // === KERNEL END - CLEANUP ===
    stream << kernel_end_label << ":" << std::endl;
    EmitKernelCleanup(stream, context);
}

void KernelStatement::EmitWarpSwitchLogic(std::ostream& stream, Context& context, 
                                        const std::string& kernel_start_label,
                                        const std::string& kernel_end_label) const {
    (void)kernel_start_label;
    std::vector<Warp>& warp_file = context.get_warp_file();
    
    // Store current warp's state
    std::string current_warp_reg = context.get_register(Type::_INT);
    std::string next_warp_label = context.create_label("load_next_warp");
    std::string check_completion_label = context.create_label("check_completion");
    
    // Set to scalar mode for warp management
    context.set_instruction_state(Kernel::_SCALAR);
    
    // Find and store the currently active warp
    for(size_t i = 0; i < warp_file.size(); i++) {
        std::string warp_check_label = context.create_label("warp_check");
        
        stream << "s.seqi " << current_warp_reg << ", s24, " << i << std::endl;
        stream << "s.beqz " << current_warp_reg << ", " << warp_check_label << std::endl;
        
        // Store current warp (warp i)
        StoreWarpRegisters(stream, context, warp_file[i]);
        warp_file[i].set_completion(true);
        warp_file[i].set_activity(false);
        
        stream << "s.j " << next_warp_label << std::endl;
        stream << warp_check_label << ":" << std::endl;
    }
    
    context.deallocate_register(current_warp_reg);
    
    // === LOAD NEXT WARP ===
    stream << next_warp_label << ":" << std::endl;
    
    // Find next incomplete warp
    std::vector<std::string> warp_load_labels;
    std::string no_more_warps_label = context.create_label("no_more_warps");
    
    for(size_t i = 0; i < warp_file.size(); i++) {
        std::string warp_load_label = context.create_label("load_warp_" + std::to_string(i));
        warp_load_labels.push_back(warp_load_label);
        
        // Check if this warp is complete
        std::string completion_reg = context.get_register(Type::_INT);
        int completion_address = warp_file[i].get_warp_offset() - (32+1)*4; // Store completion flag at offset 32
        
        stream << "s.li " << completion_reg << ", " << completion_address << std::endl;
        stream << "s.lw " << completion_reg << ", 0(" << completion_reg << ")" << std::endl;
        stream << "s.beqz " << completion_reg << ", " << warp_load_label << std::endl;
        
        context.deallocate_register(completion_reg);
    }
    
    // If we get here, all warps are complete
    stream << "s.j " << no_more_warps_label << std::endl;
    
    // === INDIVIDUAL WARP LOADING SECTIONS ===
    for(size_t i = 0; i < warp_file.size(); i++) {
        stream << warp_load_labels[i] << ":" << std::endl;
        
        // Mark this warp as active and load it
        warp_file[i].set_activity(true);
        InitializeWarp(stream, context, warp_file[i]);
        
        // Jump back to kernel execution
        stream << "s.jalr zero, s25, 0" << std::endl;
    }
    
    // === NO MORE WARPS - GO TO CLEANUP ===
    stream << no_more_warps_label << ":" << std::endl;
    stream << "s.j " << kernel_end_label << std::endl;
}

void KernelStatement::EmitKernelCleanup(std::ostream& stream, Context& context) const {
    (void) stream;
    std::vector<Warp>& warp_file = context.get_warp_file();
    std::vector<std::string>& thread_regs = context.get_thread_regs();
    
    // Set to scalar mode for cleanup
    context.set_instruction_state(Kernel::_SCALAR);
    
    // Deallocate all thread registers across all warps
    for(auto& warp : warp_file) {
        for(int i = 0; i < warp.get_size(); i++) {
            Thread& current_thread = warp.return_thread(i);
            context.assign_reg_manager(current_thread.get_thread_file());
            for(const std::string& reg : thread_regs) {
                context.deallocate_register(reg);
            }
        }
    }
    
    // Restore original CPU context
    if(!warp_file.empty()) {
        Warp& first_warp = warp_file[0];
        context.assign_reg_manager(first_warp.get_warp_file());
    }
}

// Enhanced StoreWarpRegisters that also stores completion status
void KernelStatement::StoreWarpRegisters(std::ostream& stream, Context& context, Warp& warp) const {
    // Set the context to scalar mode and assign the warp's register file
    context.set_instruction_state(Kernel::_SCALAR);
    ScalarRegisterFile& reg_file = warp.get_warp_file();
    context.assign_reg_manager(reg_file);
    
    std::string addr = context.get_register(Type::_INT);

    // Store completion flag first (at offset 32)
    int completion_address = warp.get_warp_offset() - (32+1)*4;
    stream << asm_prefix.at(context.get_instruction_state()) << "li " << addr << ", " << completion_address << std::endl;
    stream << asm_prefix.at(context.get_instruction_state()) << "li t0, 1" << std::endl; // 1 = completed
    stream << asm_prefix.at(context.get_instruction_state()) << "sw t0, 0(" << addr << ")" << std::endl;

    // Store scalar registers (0-31) to warp stack
    for(int i = 0; i < 32; i++){
        if(!reg_file.get_register_by_id(i).isAvailable()){
            int address = warp.get_warp_offset() - (i+1)*4;
            std::string tmp_name = reg_file.get_register_name(i);
            
            stream << asm_prefix.at(context.get_instruction_state()) << "li " << addr << ", " << address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "sw " << tmp_name << ", 0(" << addr << ")" << std::endl;
        }
    }

    // Store floating-point registers (33-63) to warp stack
    for(int i = 33; i < 64; i++){
        if(!reg_file.get_register_by_id(i).isAvailable()){
            int address = warp.get_warp_offset() - (i+1)*4;
            std::string tmp_name = reg_file.get_register_name(i);
            
            stream << asm_prefix.at(context.get_instruction_state()) << "li " << addr << ", " << address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "fsw " << tmp_name << ", 0(" << addr << ")" << std::endl;
        } 
    }

    context.deallocate_register(addr);

    // Switch to vector mode for thread operations
    context.set_instruction_state(Kernel::_VECTOR);

    // Store thread registers for each thread in the warp
    for(int thread_idx = 0; thread_idx < warp.get_size(); thread_idx++){
        Thread& thread = warp.return_thread(thread_idx);
        VectorRegisterFile& thread_file = thread.get_thread_file();
        context.assign_reg_manager(thread_file);
        
        std::string thread_addr = context.get_register(Type::_INT);

        // Store integer vector registers (0-31)
        for(int reg_idx = 0; reg_idx < 32; reg_idx++){
            int address = thread.get_offset() - (reg_idx+1)*4;
            std::string tmp_name = thread_file.get_register_name(reg_idx);
            
            stream << asm_prefix.at(context.get_instruction_state()) << "li " << thread_addr << ", " << address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "sw " << tmp_name << ", 0(" << thread_addr << ")" << std::endl;
        }

        // Store floating-point vector registers (32-63)
        for(int reg_idx = 32; reg_idx < 64; reg_idx++){
            int address = thread.get_offset() - (reg_idx+1)*4;
            std::string tmp_name = thread_file.get_register_name(reg_idx);

            stream << asm_prefix.at(context.get_instruction_state()) << "li " << thread_addr << ", " << address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "fsw " << tmp_name << ", 0(" << thread_addr << ")" << std::endl;
        }

        context.deallocate_register(thread_addr);
    }
}

void KernelStatement::InitializeWarp(std::ostream& stream, Context& context, Warp& warp) const {
    // Set the context to scalar mode and assign the warp's register file
    context.set_instruction_state(Kernel::_SCALAR);
    ScalarRegisterFile& reg_file = warp.get_warp_file();
    context.assign_reg_manager(reg_file);
    
    std::string addr = context.get_register(Type::_INT);

    // Load scalar registers (0-31) from warp stack
    for(int i = 0; i < 32; i++){
        if(!reg_file.get_register_by_id(i).isAvailable()){
            int address = warp.get_warp_offset() - (i+1)*4;
            std::string tmp_name = reg_file.get_register_name(i);
            
            stream << asm_prefix.at(context.get_instruction_state()) << "li " << addr << ", " << address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "lw " << tmp_name << ", " << 0 << "(" << addr << ")" << std::endl;
        }
    }

    // Load floating-point registers (33-63) from warp stack
    for(int i = 33; i < 64; i++){
        if(!reg_file.get_register_by_id(i).isAvailable()){
            int address = warp.get_warp_offset() - (i+1)*4;
            std::string tmp_name = reg_file.get_register_name(i);
            
            stream << asm_prefix.at(context.get_instruction_state()) << "li " << addr << ", " << address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "flw " << tmp_name << ", " << 0 << "(" << addr << ")" << std::endl;
        } 
    }

    context.deallocate_register(addr);

    // Switch to vector mode for thread operations
    context.set_instruction_state(Kernel::_VECTOR);

    // Load thread registers for each thread in the warp
    for(int thread_idx = 0; thread_idx < warp.get_size(); thread_idx++){
        Thread& thread = warp.return_thread(thread_idx);
        VectorRegisterFile& thread_file = thread.get_thread_file();
        context.assign_reg_manager(thread_file);
        
        std::string thread_addr = context.get_register(Type::_INT);

        // Load integer vector registers (0-31)
        for(int reg_idx = 0; reg_idx < 32; reg_idx++){
            int address = thread.get_offset() - (reg_idx+1)*4;
            std::string tmp_name = thread_file.get_register_name(reg_idx);
            
            stream << asm_prefix.at(context.get_instruction_state()) << "li " << thread_addr << ", " << address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "lw " << tmp_name << ", " << 0 << "(" << thread_addr << ")" << std::endl;
        }

        // Load floating-point vector registers (32-63)
        for(int reg_idx = 32; reg_idx < 64; reg_idx++){
            int address = thread.get_offset() - (reg_idx+1)*4;
            std::string tmp_name = thread_file.get_register_name(reg_idx);

            stream << asm_prefix.at(context.get_instruction_state()) << "li " << thread_addr << ", " << address << std::endl;
            stream << asm_prefix.at(context.get_instruction_state()) << "flw " << tmp_name << ", " << 0 << "(" << thread_addr << ")" << std::endl;
        }

        context.deallocate_register(thread_addr);
    }

    // Set the context back to the first thread's register file for subsequent operations
    context.assign_reg_manager(warp.return_thread(0).get_thread_file());
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
    first_warp.set_activity(true); //set to true
    int warp_offset = first_warp.get_warp_offset();

    std::string offset_reg = context.get_register(Type::_INT);

    int address = warp_offset - ((3+1)*4);

    //load gp
    address = warp_offset - ((3+1)*4);
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
            thread = thread_total + warp_size;
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
            tmp_total -= file_offset;
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
