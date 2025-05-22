#pragma once

#include "../ast_node.hpp"
#include "../operations/ast_operand.hpp"
#include <memory>

namespace ast {

class Identifier : public Operand
{
private:
    std::string identifier_;

public:
    Identifier(std::string *identifier) : identifier_(std::move(*identifier)){ delete identifier; };

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;

    std::string GetId() const override;

    Type GetType(Context &context) const override;

    int get_val(Context &context) const;
    bool isPointerOp(Context &context) const override;
};

}//namespace ast
