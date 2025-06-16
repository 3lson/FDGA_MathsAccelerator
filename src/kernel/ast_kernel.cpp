#include "../../include/kernel/ast_kernel.hpp"
#include <iostream>
#include <vector>
#include <cmath>

namespace ast {

void KernelStatement::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {
    (void)stream;
    (void)context;
    (void)dest_reg;

    InitializeKernel(context);
}

void KernelStatement::Print(std::ostream& stream) const {
    (void)stream;
}


void KernelStatement::InitializeKernel(Context& context) const{
    const IntConstant *threads = dynamic_cast <const IntConstant *>(threads_.get());
    int thread_total = threads->get_val();
    int warp_size = context.get_warp_size();

    int num_warps = (thread_total + warp_size - 1) / warp_size;

    std::vector<Warp>& warp_file = context.get_warp_file(); //direclty modifies warp_file inside context
    warp_file.reserve(num_warps);
    for(int i = 0; i < num_warps; i++){
        thread_total -= warp_size;
        if(thread_total >= 0){
            warp_file.emplace_back(i,warp_size,false, false);
        }
        else{
            warp_file.emplace_back(i,std::abs(thread_total),false, false);
        }
    }

    for(int i = 0; i < warp_file.size(); i++){
        warp_file[i].initialise_from_cpu(context.get_main_cpu_regs());
    }

    
}

}
