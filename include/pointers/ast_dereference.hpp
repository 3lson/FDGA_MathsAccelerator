#pragma once

#include "ast_node.hpp"
#include "../operations/ast_operand.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../arrays/ast_array_index_access.hpp"

namespace ast {

class Dereference : public Operand {
private:
    NodePtr operand_;

public:
    Dereference(NodePtr operand) : operand_(std::move(operand)) {}

    Type GetType(Context& context) const override;
    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
    std::string GetId() const;
    int dereferenceCount() const;
    bool isPointer(Context &context) const;
    bool isPointerOp(Context &context) const override;
    void StartingOffset(std::ostream &stream, Context &context, std::string address_reg) const;
    void deref_route(std::ostream &stream, Context &context, std::string address_reg) const;
};

} // namespace ast
