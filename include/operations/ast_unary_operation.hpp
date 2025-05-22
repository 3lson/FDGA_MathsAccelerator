#pragma once

#include "ast_node.hpp"
#include "ast_operand.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../structs/ast_struct_access.hpp"

namespace ast {

enum class UnaryOp {
    INC, DEC, PLUS, MINUS, BITWISE_NOT, LOGICAL_NOT,
};

class UnaryExpression : public Operand {
private:
    UnaryOp op_;
    NodePtr operand_;

public:
    UnaryExpression(UnaryOp op, NodePtr operand)
        : op_(op), operand_(std::move(operand)) {}

    Type GetType(Context& context) const;
    std::string GetOperation(Type type) const;
    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
    bool isPointerOp(Context &context) const override;
};

} // namespace ast
