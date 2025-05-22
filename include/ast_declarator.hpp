#pragma once

#include "ast_node.hpp"

namespace ast{

class Declarator : public Node
{
public:
    virtual std::string GetId() const = 0;
    virtual std::vector<Parameter> get_param(Context &context) const = 0;
    virtual int get_offset() const =0;
    virtual void store_param(std::ostream &stream, Context &context, std::string dest_reg) const = 0;
    virtual bool isPointer() const = 0;
    virtual int get_deref() const =0;

    virtual void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override = 0;
    virtual void Print(std::ostream &stream) const override = 0;
};
}//namespace ast
