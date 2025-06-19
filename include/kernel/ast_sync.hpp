#pragma once

#include "ast_node.hpp"
#include "../context/ast_context.hpp"

namespace ast {

class SyncStatement : public Node {
private:

public:
    SyncStatement() = default;
    ~SyncStatement() override = default;

    void EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
};

}
