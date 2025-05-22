#pragma once

#include "ast_node.hpp"
#include "../operations/ast_operand.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../arrays/ast_array_index_access.hpp"

namespace ast {

class AddressOf : public Operand {
private:
    NodePtr operand_;

public:
    AddressOf(NodePtr operand) : operand_(std::move(operand)) {}

    Type GetType(Context& context) const override;

    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;

    void Print(std::ostream& stream) const override;

    int AddressCount() const;

    std::string GetId() const;
    bool isPointerOp(Context &context) const override;
};

}//namespace ast