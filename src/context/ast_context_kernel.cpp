#include "../../include/context/ast_context.hpp"


namespace ast{

const std::unordered_map<Kernel,std::string> asm_prefix = {
    {Kernel::_SCALAR,"s."},
    {Kernel::_VECTOR,"v."},
};

void Context::set_instruction_state(Kernel state){
    instruction_state = state;


}

Warp::Warp(int warp_id_,int warp_size_, bool is_active_)
:warp_id(warp_id_), warp_size(warp_size_), is_active(is_active_){

    //threads created
    threads.reserve(warp_size);
    for(int i = 0; i < warp_size; i++){
        threads.emplace_back(i, warp_id);
    }
    
}

void Warp::initialise_from_cpu(const ScalarRegisterFile& cpu_registers){
    warp_registers = cpu_registers;
}

Thread& Warp::return_thread(int thread_id){
    return threads.at(thread_id);
}

}