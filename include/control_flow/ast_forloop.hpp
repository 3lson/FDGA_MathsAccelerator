#pragma once

#include "../ast_node.hpp"

namespace ast{

class ForStatement : public Node {
private:
    NodePtr init_;
    NodePtr condition_;
    NodePtr update_;
    NodePtr body_;

public:
    ForStatement(NodePtr init, NodePtr condition, NodePtr update, NodePtr body)
    : init_(std::move(init)), condition_(std::move(condition)), update_(std::move(update)), body_(std::move(body)) {}

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;

};


} //namespace ast
