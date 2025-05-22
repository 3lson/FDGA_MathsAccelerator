#include "../../include/structs/ast_struct_access.hpp"

namespace ast{

void StructAccess::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const {
    const Identifier* struct_identifier = dynamic_cast<const Identifier *>(struct_name_.get());
    if (!struct_identifier) {
        throw std::runtime_error("StructAccess::EmitRISC - struct_name_ is not an Identifier");
    }

    std::string struct_var_name = struct_identifier->GetId();

    // Look up the struct variable
    Variable struct_var;
    try {
        struct_var = context.get_variable(struct_var_name);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("StructAccess::EmitRISC - Struct variable '" + struct_var_name + "' not found.");
    }

    int total_offset = get_offset(context);
    std::cout << "Total offset: " << total_offset << std::endl;
    Type type = GetType(context);

    // Load the member value
    stream << context.load_instr(type) << " " << dest_reg << ", " << total_offset << "(s0)" << std::endl;
}

void StructAccess::Print(std::ostream &stream) const{
    struct_name_->Print(stream);
    stream << "." << *member_name_;
}

Type StructAccess::GetType(Context &context) const{
    const Identifier* struct_identifier = dynamic_cast<const Identifier *>(struct_name_.get());
    if (struct_identifier){
        std::string struct_var_name = struct_identifier->GetId();
        Variable struct_var;
        try {
            struct_var = context.get_variable(struct_var_name);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("StructAccess::get_offset - Struct variable '" + struct_var_name + "' not found.");
        }

        std::unordered_map<std::string, Type> members = context.get_struct_members(struct_var.get_type_name());
        if (members.find(*member_name_) == members.end()) {
            throw std::runtime_error("StructAccess::get_offset - Member '" + *member_name_ + "' not found in struct.");
        }

        Type member_type = members[*member_name_];
        return member_type;
    }else {
        return Type::_INT; //Default
    }
}

std::string StructAccess::GetId() const{
    std::cout << "Getting id of the struct access: " << *member_name_ << std::endl;
    return *member_name_;
}

std::string StructAccess::GetStructName() const {
    const Identifier *id = dynamic_cast<const Identifier *>(struct_name_.get());
    if (!id) {
        throw std::runtime_error("StructAccess::GetStructName - struct_name_ is not an Identifier");
    }
    return id->GetId();
}

int StructAccess::get_offset(Context &context) const{
    const Identifier* struct_identifier = dynamic_cast<const Identifier *>(struct_name_.get());
    if (!struct_identifier) {
        throw std::runtime_error("StructAccess::get_offset - struct_name_ is not an Identifier");
    }

    std::string struct_var_name = struct_identifier->GetId();

    Variable struct_var;
    try {
        struct_var = context.get_variable(struct_var_name);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("StructAccess::get_offset - Struct variable '" + struct_var_name + "' not found.");
    }

    std::unordered_map<std::string, int> offsets = context.get_struct_offsets(struct_var.get_type_name());
    if (offsets.find(*member_name_) == offsets.end()) {
        throw std::runtime_error("StructAccess::get_offset - Member '" + *member_name_ + "' not found in struct.");
    }

    int member_offset = offsets[*member_name_];

    int base_offset = struct_var.get_offset();

    int total_offset = base_offset + member_offset;

    return total_offset;
}


}//namespace ast
