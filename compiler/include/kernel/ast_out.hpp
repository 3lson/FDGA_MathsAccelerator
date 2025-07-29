#pragma once

#include "ast_node.hpp"
#include "../context/ast_context.hpp"

namespace ast {

class OutStatement : public Node {
private:

    NodePtr declarator_;

public:
    OutStatement(NodePtr declarator) : declarator_(std::move(declarator)) {}
    ~OutStatement() override = default;

    void EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
};

}
