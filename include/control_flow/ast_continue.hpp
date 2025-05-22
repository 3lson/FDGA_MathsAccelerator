#pragma once

#include "ast_node.hpp"

namespace ast{

class ContinueStatement : public Node{
public:
    ContinueStatement() {}

    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
};

}//namespace ast
