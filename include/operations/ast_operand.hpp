#pragma once

#include "../ast_node.hpp"

namespace ast {


class Operand : public Node
{
public:
    Operand() {}

    virtual Type GetType(Context &context) const = 0;

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override = 0;
    void Print(std::ostream &stream) const override = 0;
    virtual bool isPointerOp(Context &context) const = 0;
    void ShiftPointerOp(std::ostream &stream, Context &context, std::string dest_reg, NodePtr node) const;
    Type NewPointerType(Context &context) const;

    virtual std::string GetId() const {return "";}
};

}
