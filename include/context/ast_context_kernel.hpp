#pragma once

#include "ast_context_registers.hpp" //for the RegisterFile initiailzations
#include "ast_context.hpp"
#include <unordered_map>
#include <string>
#include <vector>

namespace ast {
enum class Kernel {
    _SCALAR,
    _VECTOR,
};

extern const std::unordered_map<Kernel, std::string> asm_prefix;

class Thread{
private:
    int local_thread_id;
    int global_thread_id;
    int warp_id;
    int offset;
    VectorRegisterFile thread_registers;

public:

    Thread(int thread_id_, int warp_id_)
    : local_thread_id(thread_id_), warp_id(warp_id_){
        global_thread_id = local_thread_id + (warp_id*16);
    }

    //getters
    int get_local_thread_id() const{return local_thread_id;}
    int get_global_thread_id() const{return local_thread_id;}
    int get_warp_id() const {return warp_id;}
    int get_offset() const {return offset;}
    VectorRegisterFile& get_thread_file() {return thread_registers;}

    //setters
    void set_offset(int thread_offset) {offset = thread_offset;}

};


class Warp{
private:
    int warp_id;
    int warp_size;
    int warp_offset; // points to the top of data_mem warp address of warp stack 
    bool is_active = false;
    bool is_complete = false;
    uint32_t execution_mask;
    uint32_t PC;
    ScalarRegisterFile warp_registers;
    std::vector<Thread> threads;

public:

    Warp(int warp_id_,int warp_size_, bool is_active_);
    
    //getters
    int get_id() const {return warp_id;}
    int get_size() const {return warp_size;}
    bool get_activity() const {return is_active;}
    bool get_completion() const {return is_complete;}
    Thread& return_thread(int thread_id);
    ScalarRegisterFile& get_warp_file() {return warp_registers;}
    int get_warp_offset() const {return warp_offset;}
    int32_t get_execution_mask() const {return execution_mask;}
    
    
    //setters
    void initialise_from_cpu(const ScalarRegisterFile& cpu_registers);
    void set_activity(bool activity) {is_active = activity;}
    void set_completion(bool status) {is_complete = status;}
    void set_warp_offset(int offset) {warp_offset = offset;}
    void deactivate_lane(int index);
    void activate_lane(int index);


    Thread& return_thread(int thread_id);
};

}//namespace ast
