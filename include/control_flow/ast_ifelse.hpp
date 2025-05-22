#pragma once

#include "ast_node.hpp"

namespace ast {

class IfStatement : public Node {
private:
    NodePtr condition_;
    NodePtr then_branch_;
    NodePtr else_branch_;
    bool is_ternary_; //tenary op flag

public:
    IfStatement(NodePtr condition, NodePtr then_branch, NodePtr else_branch = nullptr, bool is_ternary = false)
        : condition_(std::move(condition)), then_branch_(std::move(then_branch)), else_branch_(std::move(else_branch)), is_ternary_(is_ternary) {}

    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
};

}
