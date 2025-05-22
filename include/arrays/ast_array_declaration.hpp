#pragma once

#include "../ast_node.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../symbols/ast_constant.hpp"
#include "../pointers/ast_pointer_declaration.hpp"
#include "../ast_declarator.hpp"

namespace ast{

class ArrayDeclaration : public Declarator{
private:
    NodePtr identifier_;
    NodePtr constant_expression_;

public:
    ArrayDeclaration(NodePtr identifier, NodePtr constant_expression) : identifier_(std::move(identifier)), constant_expression_(std::move(constant_expression)) {}

    ArrayDeclaration(NodePtr identifier) : identifier_(std::move(identifier)), constant_expression_(nullptr) {}

    std::string GetId() const override;
    int GetArraySize() const;
    bool isPointer() const override;

    virtual std::vector<Parameter>get_param(Context &context) const override;
    virtual void store_param(std::ostream &stream, Context &context, std::string dest_reg) const override;
    virtual int get_offset() const override;

    int get_deref() const override;

    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const;
    void Print(std::ostream& stream) const;
};

}
