#include "../../include/operations/ast_arithmetic_operation.hpp"

namespace ast {

std::string ArithExpression::GetOperation(Type type) const {
    static const std::unordered_map<ArithOp, std::unordered_map<Type, std::string>> opMap = {
        {ArithOp::ADD, {
            {Type::_INT, "add"}, {Type::_UNSIGNED_INT, "add"}, {Type::_CHAR, "add"},
            {Type::_SHORT, "add"}, {Type::_LONG, "add"},
            {Type::_FLOAT, "fadd.s"}, {Type::_DOUBLE, "fadd.d"}
        }},
        {ArithOp::SUB, {
            {Type::_INT, "sub"}, {Type::_UNSIGNED_INT, "sub"}, {Type::_CHAR, "sub"},
            {Type::_SHORT, "sub"}, {Type::_LONG, "sub"},
            {Type::_FLOAT, "fsub.s"}, {Type::_DOUBLE, "fsub.d"}
        }},
        {ArithOp::MUL, {
            {Type::_INT, "mul"}, {Type::_UNSIGNED_INT, "mul"}, {Type::_CHAR, "mul"},
            {Type::_SHORT, "mul"}, {Type::_LONG, "mul"},
            {Type::_FLOAT, "fmul.s"}, {Type::_DOUBLE, "fmul.d"}
        }},
        {ArithOp::DIV, {
            {Type::_INT, "div"}, {Type::_UNSIGNED_INT, "div"}, {Type::_CHAR, "div"},
            {Type::_SHORT, "div"}, {Type::_LONG, "div"},
            {Type::_FLOAT, "fdiv.s"}, {Type::_DOUBLE, "fdiv.d"}
        }},
        {ArithOp::MOD, {
            {Type::_INT, "rem"}, {Type::_UNSIGNED_INT, "rem"}, {Type::_CHAR, "rem"},
            {Type::_SHORT, "rem"}, {Type::_LONG, "rem"},
            {Type::_FLOAT, "frem.s"}, {Type::_DOUBLE, "frem.d"}
        }}
    };

    auto it = opMap.find(op_);
    if (it != opMap.end()) {
        auto typeIt = it->second.find(type);
        if (typeIt != it->second.end()) {
            return typeIt->second;
        }
    }

    throw std::runtime_error("Unsupported arithmetic operation or type.");
}

void ArithExpression::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    Type type = std::max(context.get_operation_type(), GetType(context));
    type = isPointerOp(context) ? Type::_INT : type;
    context.push_operation_type(type);

    std::string left_register = context.get_register(type);
    left_->EmitRISC(stream, context, left_register);
    ShiftPointerOp(stream, context, left_register, left_);
    context.add_reg_to_set(left_register);

    std::string right_register = context.get_register(type);
    right_->EmitRISC(stream, context, right_register);
    ShiftPointerOp(stream, context, right_register, right_);

    // Handle struct access (as per your existing logic)
    const StructAccess* leftStructAccess = dynamic_cast<const StructAccess*>(left_.get());
    const StructAccess* rightStructAccess = dynamic_cast<const StructAccess*>(right_.get());

    if (leftStructAccess) {
        leftStructAccess->EmitRISC(stream, context, left_register);
    }

    if (rightStructAccess) {
        rightStructAccess->EmitRISC(stream, context, right_register);
    }

    // Handle integer to floating-point conversion
    const Operand* left_operand = dynamic_cast<const Operand*>(left_.get());
    const Operand* right_operand = dynamic_cast<const Operand*>(right_.get());

    if (left_operand && right_operand) {
        Type left_type = left_operand->GetType(context);
        Type right_type = right_operand->GetType(context);

        // If left operand is an integer and right operand is a double, we convert the left operand (int) to a double
        if (left_type == Type::_INT && right_type == Type::_DOUBLE) {
            std::string temp_reg = context.get_register(Type::_FLOAT);
            stream << "fcvt.d.w " << temp_reg << ", " << left_register << std::endl;
            left_register = temp_reg;
        }
        // If right operand is an integer and left operand is a double, we convert the right operand (int) to a double
        else if (right_type == Type::_INT && left_type == Type::_DOUBLE) {
            std::string temp_reg = context.get_register(Type::_FLOAT);
            stream << "fcvt.d.w " << temp_reg << ", " << right_register << std::endl;
            right_register = temp_reg;
        }
    }

    // Emit the final operation, adding the two operands
    stream << GetOperation(type) << " " << dest_reg << ", " << left_register << ", " << right_register << std::endl;

    // Clean up the registers
    context.deallocate_register(right_register);
    context.deallocate_register(left_register);
    context.remove_reg_from_set(left_register);

    context.pop_operation_type();
}

void ArithExpression::Print(std::ostream &stream) const
{
    left_->Print(stream);
    switch(op_){
        case ArithOp::ADD: stream << " + "; break;
        case ArithOp::SUB: stream << " - "; break;
        case ArithOp::MUL: stream << " * "; break;
        case ArithOp::DIV: stream << " / "; break;
        case ArithOp::MOD: stream << " % "; break;
    }
    right_->Print(stream);
}

Type ArithExpression::GetType(Context &context) const {
    if (!left_ || !right_) {
        throw std::runtime_error("Null pointer encountered in ArithExpression operands.");
    }

    // Handle left operand
    Type leftType;
    const Operand* leftOperand = dynamic_cast<const Operand*>(left_.get());
    const StructAccess* leftStructAccess = dynamic_cast<const StructAccess*>(left_.get());

    if (leftOperand) {
        leftType = leftOperand->GetType(context);
    } else if (leftStructAccess) {
        // Handle StructAccess by determining the type of the struct member
        leftType = leftStructAccess->GetType(context); // Assuming StructAccess has a GetType method
    } else {
        throw std::runtime_error("Invalid left operand type: " + std::string(typeid(*left_).name()));
    }

    // Handle right operand
    Type rightType;
    const Operand* rightOperand = dynamic_cast<const Operand*>(right_.get());
    const StructAccess* rightStructAccess = dynamic_cast<const StructAccess*>(right_.get());

    if (rightOperand) {
        rightType = rightOperand->GetType(context);
    } else if (rightStructAccess) {
        // Handle StructAccess by determining the type of the struct member
        rightType = rightStructAccess->GetType(context); // Assuming StructAccess has a GetType method
    } else {
        throw std::runtime_error("Invalid right operand type: " + std::string(typeid(*right_).name()));
    }

    return std::max(leftType, rightType);
}


bool ArithExpression::isPointerOp(Context &context) const
{
    // Attempt to cast left_ and right_ to Operand
    const Operand *left_operand = dynamic_cast<const Operand *>(left_.get());
    const Operand *right_operand = dynamic_cast<const Operand *>(right_.get());

    bool left_is_pointer = left_operand ? left_operand->isPointerOp(context) : false;
    bool right_is_pointer = right_operand ? right_operand->isPointerOp(context) : false;

    // Return true if either operand is a pointer
    return left_is_pointer || right_is_pointer;
}

void ArithExpression::ShiftPointerOp(std::ostream &stream, Context &context, std::string dest_reg, const NodePtr& node) const
{
    if (isPointerOp(context))
    {
        if (!dynamic_cast<const Operand *>(node.get())->isPointerOp(context))
        {
            Type type = NewPointerType(context);
            stream << "slli " << dest_reg << ", " << dest_reg << ", " << types_mem_shift.at(type) << std::endl;
        }
    }
}

Type ArithExpression::NewPointerType(Context &context) const
{
    const Operand *left_operand = dynamic_cast<const Operand *>(left_.get());
    const Operand *right_operand = dynamic_cast<const Operand *>(right_.get());

    if (left_operand->isPointerOp(context))
    {
        return left_operand->GetType(context);
    }
    return right_operand->GetType(context);
}


}
