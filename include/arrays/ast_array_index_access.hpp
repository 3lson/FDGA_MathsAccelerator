#pragma once

#include "../ast_node.hpp"
#include "../operations/ast_operand.hpp"
#include "../symbols/ast_identifier.hpp"

namespace ast{
class ArrayIndexAccess : public Operand{
private:
    NodePtr identifier_;
    NodePtr index_;
public:
    ArrayIndexAccess(NodePtr identifier, NodePtr index) : identifier_(std::move(identifier)), index_(std::move(index)) {}

    std::string GetId() const;
    Type GetType(Context& context) const;
    void get_position(std::ostream& stream, Context& context, std::string dest_reg, Type type) const;
    bool isPointerOp(Context &context) const override;

    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const;
    void Print(std::ostream& stream) const;

};

}//namespace ast
