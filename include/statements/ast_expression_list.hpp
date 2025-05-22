#pragma once

#include "../ast_node.hpp"
#include "../operations/ast_operand.hpp"

namespace ast{

class Expression : public Operand
{
protected:
    std::vector<NodePtr> nodes_;
public:
    Expression(NodePtr first_node) {
        nodes_.push_back(std::move(first_node));
    }

    Type GetType(Context &context) const;
    void PushBack(NodePtr item);
    bool isPointerOp(Context &context) const override;

    virtual void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    virtual void Print(std::ostream &stream) const override;

};

class ExpressionList : public NodeList
{
public:
    using NodeList::NodeList;
    ~ExpressionList() = default;

    void GetArguments(std::ostream &stream, Context &context, std::string dest_reg) const;
    void Print(std::ostream &stream) const override;
};

}
