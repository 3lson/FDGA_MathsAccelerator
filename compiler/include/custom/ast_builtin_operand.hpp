#pragma once

#include "operations/ast_operand.hpp"

namespace ast {

class BuiltInOperand : public Operand {
private:
    std::string name_;
    int reg_id_;

public:
    BuiltInOperand(const std::string& name, int reg_id);

    Type GetType(Context& context) const override;
    void EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
    bool isPointerOp(Context& context) const override;
};

}
