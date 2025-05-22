#pragma once

#include "../ast_node.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../ast_direct_declarator.hpp"
#include "../ast_declarator.hpp"

namespace ast{

class PointerDeclaration : public Declarator
{
private:
    NodePtr direct_declarator_;

public:
    PointerDeclaration(NodePtr direct_declarator) : direct_declarator_(std::move(direct_declarator)) {}

    std::string GetId() const override;
    Type GetType() const;

    virtual std::vector<Parameter>get_param(Context &context) const override;
    virtual int get_offset() const override;
    virtual void store_param(std::ostream &stream, Context &context, std::string dest_reg) const override;
    virtual bool isPointer() const override;
    int get_deref() const override;

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
};
}//namespace ast