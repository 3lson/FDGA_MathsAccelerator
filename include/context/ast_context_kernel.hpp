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
    int thread_id;
    int warp_id;
    VectorRegisterFile thread_registers;

public:

    Thread(int thread_id_, int warp_id_)
    : thread_id(thread_id_), warp_id(warp_id_){}

    //getters
    int get_thread_id() const{return thread_id;}
    int get_warp_id() const {return warp_id;}
    VectorRegisterFile& get_thread_file() {return thread_registers;}

    //setters
};


class Warp{
private:
    int warp_id;
    int warp_size;
    int warp_offset; // points to the top of data_mem warp address of warp stack 
    bool is_active = false;
    bool is_complete = false;
    ScalarRegisterFile warp_registers;
    std::vector<Thread> threads;

    void initialise_from_cpu(Context& context);


public:

    Warp(int warp_id_,int warp_size_, bool is_active_, int warp_offset_)
    : warp_id(warp_id_), warp_size(warp_size_), is_active(is_active_), warp_offset(warp_offset_){
        //initialize warp_size number of threads here
        initialise_from_cpu(context); 
    }


    //getters
    int get_id() const {return warp_id;}
    int get_size() const {return warp_size;}
    bool get_activity() const {return is_active;}
    bool get_completion() const {return is_complete;}
    Thread& return_thread(int thread_id);
    ScalarRegisterFile& get_warp_file() {return warp_registers;}
    int warp_offset() const {return warp_offset;}
    
    
    //setters
    void set_activity(bool activity) {is_active = activity;}
    void set_completion(bool status) {is_complete = status;}
    void set_warp_offset(int offset) {warp_offset = offset;}

    Thread& return_thread(int thread_id);
};

}//namespace ast
