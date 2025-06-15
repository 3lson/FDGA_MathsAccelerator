#include "../../include/kernel/ast_kernel.hpp"
#include <iostream>

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

    
}

}
