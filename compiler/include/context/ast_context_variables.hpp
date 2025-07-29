#pragma once
#include "ast_context_types.hpp"
#include <vector>

namespace ast{
enum class ScopeLevel {
    GLOBAL,
    LOCAL,
};

class Variable {
private:
    bool isPointer;
    bool isArray;
    Type dataType;
    ScopeLevel scope;
    int memoryOffset;
    int arraySize;
    int dereference_num;
    std::vector<int> arr_dim = {};
    int identifier_constant = 0;
    std::string type_name_;
    std::string reg = "";
    int out_offset = 0;

public:
    Variable() : isPointer(false), isArray(false), dataType(Type::_INT), scope(ScopeLevel::LOCAL), memoryOffset(0), arraySize(0), dereference_num(0), type_name_("")  {}

    Variable(bool ptr, bool arr, Type type, int offset, int dereference_num)
    : isPointer(ptr), isArray(arr), dataType(type), scope(ScopeLevel::LOCAL), memoryOffset(offset), arraySize(1), dereference_num(dereference_num), type_name_("")  {}

    Variable(bool ptr, bool arr, int size, Type type, int offset, int dereference_num)
    : isPointer(ptr), isArray(arr), dataType(type), scope(ScopeLevel::LOCAL), memoryOffset(offset), arraySize(size), dereference_num(dereference_num), type_name_("")  {}

    Variable(bool ptr, bool arr, int size, Type type, int offset, int dereference_num, std::vector<int> dim)
    : isPointer(ptr), isArray(arr), dataType(type), scope(ScopeLevel::LOCAL), memoryOffset(offset), arraySize(size), dereference_num(dereference_num), arr_dim(dim) , type_name_("")  {}

    Variable(bool ptr, bool arr, int size, Type type, int offset, int dereference_num, int constant)
    : isPointer(ptr), isArray(arr), dataType(type), scope(ScopeLevel::LOCAL), memoryOffset(offset), arraySize(size), dereference_num(dereference_num),identifier_constant(constant), type_name_("")  {}

    Variable(bool ptr, bool arr, int size, Type type, int offset, int dereference_num, int constant, std::vector<int> dim)
    : isPointer(ptr), isArray(arr), dataType(type), scope(ScopeLevel::LOCAL), memoryOffset(offset), arraySize(size), dereference_num(dereference_num), arr_dim(dim) ,identifier_constant(constant), type_name_("") {}

    Variable(bool ptr, bool arr, Type type, ScopeLevel scp, int dereference_num)
    : isPointer(ptr), isArray(arr), dataType(type), scope(scp), memoryOffset(0), arraySize(1), dereference_num(dereference_num), type_name_("")  {}

    Variable(bool ptr, bool arr, int size, Type type, ScopeLevel scp, int dereference_num)
    : isPointer(ptr), isArray(arr), dataType(type), scope(scp), memoryOffset(0), arraySize(size), dereference_num(dereference_num), type_name_("")  {}

    Variable(bool is_pointer, bool arr, int size, Type type, int offset, int dereference_num, std::string type_name)
        : isPointer(is_pointer), isArray(arr), dataType(type), //hard coded local as global isn't set this way
          memoryOffset(offset), arraySize(size), dereference_num(dereference_num), type_name_(type_name) {}

    //Getters
    bool is_pointer() const { return isPointer; }
    bool is_array() const { return isArray; }
    Type get_type() const { return dataType; }
    ScopeLevel get_scope() const { return scope; }
    int get_offset() const { return memoryOffset; }
    int get_array_size() const { return arraySize; }
    int get_dereference_num() const {return dereference_num;}
    int get_value() const {return identifier_constant; }
    std::vector<int> get_dim() const {return arr_dim; }
    std::string get_type_name() const{ return type_name_; }
    std::string get_reg() const {return reg;}
    int get_out_offset() const {return out_offset;}

    //Setters
    void set_pointer(bool ptr) {isPointer = ptr; }
    void set_array(bool arr) { isArray = arr; }
    void set_type(Type type) { dataType = type; }
    void set_scope(ScopeLevel scp) { scope = scp; }
    void set_offset(int offset) { memoryOffset = offset; }
    void set_value(int value) {identifier_constant = value;}
    void set_dereference_num(int num) { dereference_num = num; }
    void set_reg(std::string fixed_register) {reg = fixed_register;}
    void set_out_offset(int offset) {out_offset = offset;}
};

class Global : public Variable {
    private:
        std::vector<uint32_t> lowerValues;
        std::vector<uint32_t> upperValues;
        std::vector<std::string> labels;
        bool hasLabel = false;

    public:
        Global() : Variable() {}
        Global(bool ptr, bool arr, Type type, int dereference_num) : Variable(ptr, arr, type, ScopeLevel::GLOBAL, dereference_num) {}
        Global(bool ptr, bool arr, int size, Type type, int dereference_num) : Variable(ptr, arr, size, type, ScopeLevel::GLOBAL, dereference_num) {}

        void print_global(std::ostream &stream) const;

        void print_labels(std::ostream &stream) const;
        void print_num(std::ostream &stream) const;

        void push_lower(uint32_t value);
        void push_upper(uint32_t value);

        void push_label(std::string label);
    };
}//namespace ast
