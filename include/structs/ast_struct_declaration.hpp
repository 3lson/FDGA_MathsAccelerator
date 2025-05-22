#pragma once

#include "ast_node.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../context/ast_context.hpp"

namespace ast{

class StructDeclaration : public Node{
private:
    std::string *struct_type_;
    std::string *variable_name_;

public:
    StructDeclaration(std::string *struct_type, std::string *variable_name) : struct_type_(struct_type), variable_name_(variable_name) {}

    ~StructDeclaration(){
        if (struct_type_){
            delete struct_type_;
        }
        if (variable_name_){
            delete variable_name_;
        }
    }

    void Print(std::ostream& stream) const override;
    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    std::string GetId() const;
    Type GetType() const;
};

}//namespace ast
