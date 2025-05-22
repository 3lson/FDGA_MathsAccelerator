#pragma once

#include "../ast_node.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../statements/ast_expression_list.hpp"

namespace ast{

class FunctionInvocation : public Operand
{
private:
    NodePtr identifier_;
    NodePtr argument_list_;

public:
    FunctionInvocation(NodePtr identifier) : identifier_(std::move(identifier)), argument_list_(nullptr) {};
    FunctionInvocation(NodePtr identifier, NodePtr argument_list) : identifier_(std::move(identifier)), argument_list_(std::move(argument_list)) {};

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
    Type GetType(Context &context) const override;
    bool isPointerOp(Context &context) const override;
};
}//namespace ast
