#pragma once

#include "ast_node.hpp"

namespace ast{

class Specifier : public Node
{
public:
    Specifier() {};

    virtual Type GetType() const = 0;

    virtual void define_spec(Context &context) const = 0;

    virtual void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    virtual void Print(std::ostream &stream) const override = 0;
};

}//namespace ast
