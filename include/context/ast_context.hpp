#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <set>

#include "ast_context_types.hpp"
#include "ast_context_registers.hpp"
#include "ast_context_functions.hpp"
#include "ast_context_variables.hpp"
#include "ast_context_constant.hpp"
#include "ast_context_enums.hpp"
#include "ast_context_typedef.hpp"
#include "ast_context_strings.hpp"

namespace ast {

class Context
{
private:
    // ----- Register Management ------
    ContextRegister reg_manager;
    std::stack<std::set<int>> allocated_registers;
    std::unordered_map<int, int> allocated_register_offsets;

    // ----- Stack Management -----
    std::stack<int> current_stack_offset;

    int total_offset =0;

    // ----- Variable Management ------
    std::vector<std::unordered_map<std::string, Variable>> variableMap;

    // ------ Function Management -------
    std::unordered_map<std::string, Function> functionMap;
    std::string last_function_end_statement;
    std::string return_register;

    // ------ Function Calling Management ------
    std::stack<std::string> function_stack;

    // ------ Control Flow Management ------
    int label_counter;
    std::stack<std::string> start_labels;
    std::stack<std::string> end_labels;

    // ------ Type tracking -------
    std::stack<Type> operation_stack;

    // --------- Constant management -----------
    int constantIndex = 0;
    std::vector<ContextConstant> constants;

    // ------- Global Management -------
    std::unordered_map<std::string, Global> globalMap;

    // ------- Enums Management ---------
    std::vector<enum_Map> enumMap;
    std::unordered_map<std::string, std::vector<std::string>> enumsDefinitions;

    // ------ Typedef Management ------
    std::vector<std::unordered_map<std::string, TypedefSpec>> typedefMap;

    // ------ Char and String Management --------
    std::vector<ContextStrings> strings;
    int string_counter = 0;

    // ------ Struct Management -------
    std::unordered_map<std::string, std::unordered_map<std::string, Type>>structMap;
    std::unordered_map<std::string, std::unordered_map<std::string, int>>structOffsetMap;
    std::unordered_map<std::string, Type> struct_type_map_;
    std::unordered_map<Type, int> struct_size_map_;
    std::unordered_map<std::string, int> structSizes;

public:
    Context();
    ~Context();

    // ---------- Register Management --------------
    std::string get_register(Type type) { return reg_manager.get_register(type); }
    std::string get_return_register() const { return return_register; }
    std::string get_register_name(int reg_number) const {return reg_manager.get_register_name(reg_number); }
    void set_register_type(const std::string &reg_name, Type type) { reg_manager.set_register_type(reg_name, type); }

    void allocate_register(std::string reg_name, Type type) { reg_manager.allocate_register(reg_name, type); }
    void deallocate_register(const std::string &reg_name) { reg_manager.deallocate_register(reg_name); }

    void add_reg_to_set(std::string reg_name);
    void remove_reg_from_set(std::string reg_name);

    void push_registers(std::ostream &stream);
    void pop_registers(std::ostream &stream);

    // --------- Variable Management --------

    void define_variable(const std::string& name, const Variable& var);
    Variable get_variable(const std::string& name) const;

    // --------- Scope Management ---------
    void create_scope();
    void pop_scope();
    int get_stack_offset() const;
    void increase_stack_offset(int offset);
    void set_stack_offset(int offset);

    // --------- Function Management ---------
    void define_function(std::string identifier, Function function);
    Function get_function(std::string identifier) const;
    std::string get_function_end() const;
    void exit_function();
    void set_return_register(Type type);

    // ------ Function Calling Management -------
    void push_function_call(std::string function);
    void pop_function_call();
    Function get_function_call() const;

    // ------ Control Flow Management --------
    std::string create_label(std::string id);

    void push_start_label(std::string label);
    void push_end_label(std::string label);

    void pop_start_label();
    void pop_end_label();

    std::string get_start_label() const;
    std::string get_end_label() const;

    // --------- Move, store and load instructions ------
    std::string move_instr(Type type) const;
    std::string store_instr(Type type) const;
    std::string load_instr(Type type) const;

    // ----- Type Management --------
    void push_operation_type(Type type);
    void pop_operation_type();
    Type get_operation_type() const;
    bool is_type_stack_empty() const;

    // --------- Constant Management ---------
    int registerConstant(float value);
    int registerConstant(double value);
    void constDecl(std::ostream &stream) const;

    // -------- Global Management -------
    void define_global(std::string name, Global &global);
    void print_global(std::ostream &stream) const;

    // -------- Enum Management ---------
    bool is_enum(std::string identifier);
    int get_enum_label(std::string label);
    void define_enum(std::string name);
    void define_enum_label(std::string label, int value);

    // ----- Typedef Management -------
    void define_typedef(const std::string& name, const TypedefSpec& spec);
    TypedefSpec get_typedef(const std::string& name) const;
    bool is_typedef(const std::string& name) const;

    // ------ Pointer Management ----------
    char *cancel_complement(const char *sequence);

    // ------ Chars and Strings Management ---------
    char *process_characters(const char *sequence);
    std::string escape_sequences(const char *processed_characters);
    int define_string(std::string string);
    void print_string(std::ostream &stream) const;

    // ------ Struct Management ------------
    void struct_init(const std::string& identifier, const std::unordered_map<std::string, Type>& structMembers);
    std::unordered_map<std::string, Type> get_struct_members(const std::string& identifier) const;
    bool is_struct(const std::string& identifier);
    std::unordered_map<std::string, int> get_struct_offsets(const std::string& identifier) const;
    Type get_or_create_struct_type(const std::string& struct_name);
    void set_struct_size(Type struct_type, int size);
    int get_struct_size(Type struct_type) const;
};

extern Context context;

}//namespace ast
