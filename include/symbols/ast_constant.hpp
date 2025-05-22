#pragma once

#include "../ast_node.hpp"
#include "../operations/ast_operand.hpp"

namespace ast{

class Constant : public Operand
{
public:
    Type GetType(Context &context) const override = 0;
    virtual void SaveValue(Global &global) const =0;
    bool isPointerOp(Context &context) const override;

    void EmitRISC(std::ostream &stream, Context &context, std::string passed_reg) const override = 0;
    void Print(std::ostream &stream) const override = 0;
};

class IntConstant : public Constant
{
private:
    int value_;

public:
    IntConstant(int value) : value_(value) {}

    Type GetType(Context &context) const override;
    int get_val() const;
    void SaveValue(Global &global) const override;

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
};

class FloatConstant : public Constant
{
private:
    float value_;

public:
    FloatConstant(float value) : value_(value) {}

    Type GetType(Context &context) const override;
    void SaveValue(Global &global) const override;

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
};

class DoubleConstant : public Constant
{
private:
    double value_;

public:
    DoubleConstant(double value) : value_(value) {}

    Type GetType(Context &context) const override;
    void SaveValue(Global &global) const override;

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
};


}//namespace ast
