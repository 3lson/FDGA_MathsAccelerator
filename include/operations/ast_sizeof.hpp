#pragma once

#include "../ast_node.hpp"
#include "../specifiers/ast_type_specifier.hpp"
#include "../symbols/ast_constant.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../structs/ast_struct_declaration.hpp"
#include "../structs/ast_struct_access.hpp"
#include "ast_operand.hpp"

namespace ast{

class SizeOf : public Operand
{
private:
    NodePtr unary_expression_;
    NodePtr constant_expression_;
    int pointer_;

public:
    SizeOf(NodePtr unary_expression) : unary_expression_(std::move(unary_expression)), constant_expression_(nullptr), pointer_(0) {}
    SizeOf(NodePtr unary_expression, int pointer) : unary_expression_(std::move(unary_expression)), constant_expression_(nullptr), pointer_(pointer) {}
    SizeOf(NodePtr unary_expression, NodePtr constant_expression) : unary_expression_(std::move(unary_expression)), constant_expression_(std::move(constant_expression)), pointer_(0) {}
    SizeOf(NodePtr unary_expression, int pointer, NodePtr constant_expression) : unary_expression_(std::move(unary_expression)), constant_expression_(std::move(constant_expression)), pointer_(pointer) {}

    Type GetType(Context &context) const override;
    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
    bool isPointerOp(Context &context) const override;
};

}//namespace ast