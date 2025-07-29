#pragma once
#include <string>
#include <unordered_map>
#include <stack>
#include <set>
#include <iostream>
#include "ast_context_types.hpp"

namespace ast {

class Register {
private:
    const std::string reg_name;
    Type type;
    bool is_available;

public:
    Register() : reg_name(""), type(Type::_VOID), is_available(false) {}

    Register(std::string name, Type type = Type::_VOID, bool available = false)
        : reg_name(std::move(name)), type(type), is_available(available) {}

    // Getters
    bool isAvailable() const { return is_available; }
    Type getType() const { return type; }
    const std::string& getName() const { return reg_name; }

    // Setters
    void setAvailable(bool available) { is_available = available; }
    void setType(Type newType) { type = newType; }
};

// The RegisterFile abstract class manages registerFile allocation/deallocation for both scalar and vector
class RegisterFile {
protected:
    std::unordered_map<int, Register> register_file;
    std::unordered_map<std::string, int> register_name_to_int;

    virtual void initialiseRegisters() = 0;
    void populateNameMap(){
        for (const auto& [key, reg] : register_file){
            register_name_to_int[reg.getName()] = key;
        }
    }

public:
    RegisterFile() = default;


    virtual ~RegisterFile() = default;

    virtual std::string get_register(Type type);
    virtual void deallocate_register(const std::string &reg_name);
    virtual std::string get_register_name(int reg_number) const;
    virtual void set_register_type(const std::string &reg_name, Type type);
    virtual void allocate_register(std::string reg_name, Type type);
    //Getters to be used outside in context class since register_file and register_to_int are private
    Register& get_register_by_id(int reg_num) { return register_file[reg_num]; }
    int get_register_id(const std::string& reg_name) const;

};

class ScalarRegisterFile : public RegisterFile {
protected:
    void initialiseRegisters() override;
public:
    ScalarRegisterFile();

};

class VectorRegisterFile : public RegisterFile {
protected:
    void initialiseRegisters() override;

public:
    VectorRegisterFile();

};

}//namespace ast
