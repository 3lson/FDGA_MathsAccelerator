#pragma once

#include "ast_node.hpp"
#include "operations/ast_operand.hpp"

namespace ast{

class BuiltInFunction : public Operand {
private:
    std::string func_name_;
    NodePtr argument_;

public:
    BuiltInFunction(const std::string& func_name, NodePtr argument) : func_name_(func_name), argument_(std::move(argument)) {}

    Type GetType(Context& context) const override;
    void EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
    bool isPointerOp(Context &context) const override;
};

}//namespace ast