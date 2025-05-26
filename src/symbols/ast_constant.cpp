#include "../../include/symbols/ast_constant.hpp"
#include "../../include/context/ast_context_constant.hpp"
namespace ast{

void IntConstant::EmitElsonV(std::ostream &stream, Context &context, std::string dest_reg) const
{
    (void)context;
    if (dest_reg.rfind("f", 0) == 0) {
        // It's a floating-point register (e.g., ft0, fa1, etc.)
        std::string temp_int_reg = context.get_register(Type::_INT);
        stream << "li " << temp_int_reg << ", " << value_ << std::endl;
        stream << "fcvt.s.w " << dest_reg << ", " << temp_int_reg << std::endl;
    } else {
        // It's an integer register
        stream << "li " << dest_reg << ", " << value_ << std::endl;
    }
}

void IntConstant::Print(std::ostream &stream) const
{
    stream << value_;
}

Type IntConstant::GetType(Context &context) const
{
    (void)context;
    return Type::_INT;
}

int IntConstant::get_val() const
{
    return value_;
}

void FloatConstant::EmitElsonV(std::ostream &stream, Context &context, std::string dest_reg) const
{
    int label_number = context.registerConstant(value_);

    std::string address_register = context.get_register(Type::_INT);
    stream << "lui " << address_register << ", %hi(.LC" << label_number << ")" << std::endl;
    stream << context.load_instr(Type::_FLOAT) << " " << dest_reg << ", %lo(.LC" << label_number << ")" << "(" << address_register << ")" << std::endl;
    context.deallocate_register(address_register);
}

void FloatConstant::Print(std::ostream &stream) const
{
    stream << value_;
}

Type FloatConstant::GetType(Context &context) const
{
    (void)context;
    return Type::_FLOAT;
}

void DoubleConstant::EmitElsonV(std::ostream &stream, Context &context, std::string dest_reg) const
{
    int label_number = context.registerConstant(value_);

    std::string address_register = context.get_register(Type::_INT);
    stream << "lui " << address_register << ", %hi(.LC" << label_number << ")" << std::endl;
    stream << context.load_instr(Type::_DOUBLE) << " " << dest_reg << ", %lo(.LC" << label_number << ")" << "(" << address_register << ")" << std::endl;
    context.deallocate_register(address_register);
}

void DoubleConstant::Print(std::ostream &stream) const
{
    stream << value_;
}

Type DoubleConstant::GetType(Context &context) const
{
    (void)context;
    return Type::_DOUBLE;
}

void IntConstant::SaveValue(Global &global) const
{
    global.push_lower(value_);
}

void FloatConstant::SaveValue(Global &global) const
{
    ContextConstant constant(value_);
    global.push_lower(constant.get_low_bits());
}

void DoubleConstant::SaveValue(Global &global) const
{
    ContextConstant constant(value_);

    global.push_lower(constant.get_low_bits());
    global.push_upper(constant.get_high_bits());
}

bool Constant::isPointerOp(Context &context) const
{
    (void)context;
    return false;
}


}
