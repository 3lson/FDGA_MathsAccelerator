#pragma once

#include <string>
#include <unordered_map>
#include "ast_context_types.hpp"

namespace ast {

class TypedefSpec
{
private:
    int number_pointers_;
    std::string base_class_;
    Type type_;
    int array_size_;

public:
    TypedefSpec()
        : number_pointers_(0), base_class_(""), type_(Type::_VOID), array_size_(0) {}

    TypedefSpec(Type type, int number_pointers, int array_size)
        : number_pointers_(number_pointers), base_class_(""), type_(type), array_size_(array_size) {}

    TypedefSpec(const std::string& base_class, int number_pointers, int array_size)
        : number_pointers_(number_pointers), base_class_(base_class), type_(Type::_STRUCT), array_size_(array_size) {}

    // Getters
    int get_number_pointers() const { return number_pointers_; }
    const std::string& get_base_class() const { return base_class_; }
    Type get_type() const { return type_; }
    int get_array_size() const { return array_size_; }

    // Setters
    void set_number_pointers(int number_pointers) { number_pointers_ = number_pointers; }
    void set_base_class(const std::string& base_class) { base_class_ = base_class; }
    void set_type(Type type) { type_ = type; }
    void set_array_size(int array_size) { array_size_ = array_size; }
};

} // namespace ast
