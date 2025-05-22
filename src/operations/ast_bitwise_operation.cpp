#include "../../include/operations/ast_bitwise_operation.hpp"

namespace ast {

std::string BitwiseExpression::GetOperation(Type type) const {
    static const std::unordered_map<BitwiseOp, std::unordered_map<Type, std::string>> opMap = {
        {BitwiseOp::BITWISE_AND, {
            {Type::_INT, "and"}, {Type::_UNSIGNED_INT, "and"}, {Type::_CHAR, "and"},
            {Type::_SHORT, "and"}, {Type::_LONG, "and"},
        }},
        {BitwiseOp::BITWISE_OR, {
            {Type::_INT, "or"}, {Type::_UNSIGNED_INT, "or"}, {Type::_CHAR, "or"},
            {Type::_SHORT, "or"}, {Type::_LONG, "or"},
        }},
        {BitwiseOp::BITWISE_XOR, {
            {Type::_INT, "xor"}, {Type::_UNSIGNED_INT, "xor"}, {Type::_CHAR, "xor"},
            {Type::_SHORT, "xor"}, {Type::_LONG, "xor"},
        }},
        {BitwiseOp::LEFT_SHIFT, {
            {Type::_INT, "sll"}, {Type::_UNSIGNED_INT, "sll"}, {Type::_CHAR, "sll"},
            {Type::_SHORT, "sll"}, {Type::_LONG, "sll"},
        }},
        {BitwiseOp::RIGHT_SHIFT, {
            {Type::_INT, "sra"}, {Type::_UNSIGNED_INT, "srl"}, {Type::_CHAR, "sra"},
            {Type::_SHORT, "sra"}, {Type::_LONG, "sra"},
        }}
    };

    auto it = opMap.find(op_);
    if (it != opMap.end()) {
        auto typeIt = it->second.find(type);
        if (typeIt != it->second.end()) {
            return typeIt->second;
        }
    }

    throw std::runtime_error("Unsupported bitwise operation or type.");
}

void BitwiseExpression::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
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

    stream << GetOperation(type) << " " << dest_reg << ", " << left_register << ", " << right_register << std::endl;

    context.deallocate_register(right_register);
    context.deallocate_register(left_register);
    context.remove_reg_from_set(left_register);

    context.pop_operation_type();
}

void BitwiseExpression::Print(std::ostream &stream) const
{
    left_->Print(stream);
    switch(op_){
        case BitwiseOp::BITWISE_AND: stream << " & "; break;
        case BitwiseOp::BITWISE_OR: stream << " | "; break;
        case BitwiseOp::BITWISE_XOR: stream << " ^ "; break;
        case BitwiseOp::LEFT_SHIFT: stream << " << "; break;
        case BitwiseOp::RIGHT_SHIFT: stream << " >> "; break;
    }
    right_->Print(stream);
}

Type BitwiseExpression::GetType(Context &context) const {
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

bool BitwiseExpression::isPointerOp(Context &context) const
{
    // Attempt to cast left_ and right_ to Operand
    const Operand *left_operand = dynamic_cast<const Operand *>(left_.get());
    const Operand *right_operand = dynamic_cast<const Operand *>(right_.get());

    bool left_is_pointer = left_operand ? left_operand->isPointerOp(context) : false;
    bool right_is_pointer = right_operand ? right_operand->isPointerOp(context) : false;

    // Return true if either operand is a pointer
    return left_is_pointer || right_is_pointer;
}

void BitwiseExpression::ShiftPointerOp(std::ostream &stream, Context &context, std::string dest_reg, const NodePtr& node) const
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

Type BitwiseExpression::NewPointerType(Context &context) const
{
    const Operand *left_operand = dynamic_cast<const Operand *>(left_.get());
    const Operand *right_operand = dynamic_cast<const Operand *>(right_.get());

    if (left_operand->isPointerOp(context))
    {
        return left_operand->GetType(context);
    }
    return right_operand->GetType(context);
}

} // namespace ast
