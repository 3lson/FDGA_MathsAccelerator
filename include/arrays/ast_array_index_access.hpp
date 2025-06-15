#pragma once

#include "../ast_node.hpp"
#include "../operations/ast_operand.hpp"
#include "../symbols/ast_constant.hpp"
#include "../symbols/ast_identifier.hpp"
#include <vector>

namespace ast{
class ArrayIndexAccess : public Operand{
private:
    NodePtr identifier_;
    NodePtr index_;
public:
    ArrayIndexAccess(NodePtr identifier, NodePtr index) : identifier_(std::move(identifier)), index_(std::move(index)) {}

    std::vector<int> get_linear_index(Context &context) const;
    std::string GetId() const;
    Type GetType(Context& context) const;
    void get_position(std::ostream& stream, Context& context, std::string dest_reg, Type type, Variable& variable) const;
    bool isPointerOp(Context &context) const override;

    void EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const;
    void Print(std::ostream& stream) const;

};

}//namespace ast
