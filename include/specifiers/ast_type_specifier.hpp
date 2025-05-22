#pragma once
#include "ast_node.hpp"
#include "ast_specifier.hpp"
#include "./structs/ast_struct_specifier.hpp"

namespace ast{

class TypeSpecifier : public Specifier
{
private:
    Type type_;

public:
    TypeSpecifier(Type type) : type_(type){};

    void define_spec(Context &context) const override;
    Type GetType() const;
    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
};

}//namespace ast
