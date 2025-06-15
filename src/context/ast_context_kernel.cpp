#include "../../include/context/ast_context.hpp"


namespace ast{

const std::unordered_map<Kernel,std::string> asm_prefix = {
    {Kernel::_SCALAR,"s."},
    {Kernel::_VECTOR,"v."},
};

void Context::set_instruction_state(Kernel state){
    instruction_state = state;
}

void Warp::initialise_from_cpu(Context& context){
    warp_registers = context.get_main_cpu_regs();
}

Thread& Warp::return_thread(int thread_id){
    return threads.at(thread_id);
}

}