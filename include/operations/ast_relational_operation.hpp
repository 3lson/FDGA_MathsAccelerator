#pragma once

#include "ast_node.hpp"
#include "ast_operand.hpp"
#include "../structs/ast_struct_access.hpp"

namespace ast{

enum class RelationOp {
    LESS_THAN, GREATER_THAN, LESS_THAN_OR_EQUAL, GREATER_THAN_OR_EQUAL,
};

class RelationExpression : public Operand{
private:
    RelationOp op_;
    NodePtr left_;
    NodePtr right_;

public:
    RelationExpression(RelationOp op, NodePtr left, NodePtr right) : op_(op), left_(std::move(left)), right_(std::move(right)) {}

    Type GetType(Context& context) const;
    std::string GetOperation(Type type) const;
    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
    bool isPointerOp(Context &context) const override;
    void ShiftPointerOp(std::ostream &stream, Context &context, std::string dest_reg, const NodePtr& node) const;
    Type NewPointerType(Context &context) const;

};

}//namespace ast
