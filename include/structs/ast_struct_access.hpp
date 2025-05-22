#pragma once

#include "../ast_node.hpp"
#include "../symbols/ast_identifier.hpp"

namespace ast{

class StructAccess : public Node{
private:
    NodePtr struct_name_;
    std::string *member_name_;

public:
    StructAccess(NodePtr struct_name, std::string *member_name) : struct_name_(std::move(struct_name)), member_name_(member_name) {}
    ~StructAccess(){
        if (member_name_){
            delete member_name_;
        }
    }

    void EmitRISC(std::ostream &stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
    std::string GetId() const;
    std::string GetStructName() const;
    Type GetType(Context &context) const;
    int get_offset(Context &context) const;


};



}//namespace ast
