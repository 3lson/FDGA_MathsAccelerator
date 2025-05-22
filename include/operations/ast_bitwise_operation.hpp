#pragma once

#include "ast_node.hpp"
#include "ast_operand.hpp"
#include "../structs/ast_struct_access.hpp"

namespace ast{

enum class BitwiseOp {
    BITWISE_AND, BITWISE_OR, BITWISE_XOR,
    LEFT_SHIFT, RIGHT_SHIFT,
};

class BitwiseExpression : public Operand{
private:
    BitwiseOp op_;
    NodePtr left_;
    NodePtr right_;

public:
    BitwiseExpression(BitwiseOp op, NodePtr left, NodePtr right) : op_(op), left_(std::move(left)), right_(std::move(right)) {}

    Type GetType(Context& context) const;
    std::string GetOperation(Type type) const;
    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
    bool isPointerOp(Context &context) const override;
    void ShiftPointerOp(std::ostream &stream, Context &context, std::string dest_reg,const NodePtr& node) const;
    Type NewPointerType(Context &context) const;

};

}//namespace ast
