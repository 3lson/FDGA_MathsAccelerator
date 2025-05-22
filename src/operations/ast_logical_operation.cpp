#include "../../include/operations/ast_logical_operation.hpp"

namespace ast {

std::string LogicalExpression::GetOperation(Type type) const {
    static const std::unordered_map<LogicalOp, std::unordered_map<Type, std::string>> opMap = {
        {LogicalOp::LOGICAL_AND, {
            {Type::_INT, "and"}, {Type::_UNSIGNED_INT, "and"}, {Type::_CHAR, "and"},
            {Type::_SHORT, "and"}, {Type::_LONG, "and"}
        }},
        {LogicalOp::LOGICAL_OR, {
            {Type::_INT, "or"}, {Type::_UNSIGNED_INT, "or"}, {Type::_CHAR, "or"},
            {Type::_SHORT, "or"}, {Type::_LONG, "or"}
        }}
    };

    auto it = opMap.find(op_);
    if (it != opMap.end()) {
        auto typeIt = it->second.find(type);
        if (typeIt != it->second.end()) {
            return typeIt->second;
        }
    }

    throw std::runtime_error("Unsupported logical operation or type.");
}

void LogicalExpression::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const {
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


    if (op_ == LogicalOp::LOGICAL_AND) {
        // Logical AND (&&): Short-circuit, only evaluate right if left is non-zero
        stream << "beqz " << left_register << ", false_label" << std::endl;
        stream << "snez " << dest_reg << ", " << right_register << std::endl;
        stream << "j end_label" << std::endl;
        stream << "false_label:" << std::endl;
        stream << "li " << dest_reg << ", 0" << std::endl;
    } else if (op_ == LogicalOp::LOGICAL_OR) {
        // Logical OR (||): Short-circuit, only evaluate right if left is zero
        stream << "snez " << dest_reg << ", " << left_register << std::endl;
        stream << "bnez " << dest_reg << ", end_label" << std::endl;
        stream << "snez " << dest_reg << ", " << right_register << std::endl;
    }

    stream << "end_label:" << std::endl;

    context.deallocate_register(right_register);
    context.deallocate_register(left_register);
    context.remove_reg_from_set(left_register);

    context.pop_operation_type();
}


// Print the logical expression as a string
void LogicalExpression::Print(std::ostream &stream) const {
    left_->Print(stream);
    switch (op_) {
        case LogicalOp::LOGICAL_AND: stream << " && "; break;
        case LogicalOp::LOGICAL_OR: stream << " || "; break;
    }
    right_->Print(stream);
}

// Get the type for the logical expression based on the operands' types
Type LogicalExpression::GetType(Context &context) const {
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



bool LogicalExpression::isPointerOp(Context &context) const
{
    // Attempt to cast left_ and right_ to Operand
    const Operand *left_operand = dynamic_cast<const Operand *>(left_.get());
    const Operand *right_operand = dynamic_cast<const Operand *>(right_.get());

    bool left_is_pointer = left_operand ? left_operand->isPointerOp(context) : false;
    bool right_is_pointer = right_operand ? right_operand->isPointerOp(context) : false;

    // Return true if either operand is a pointer
    return left_is_pointer || right_is_pointer;
}

void LogicalExpression::ShiftPointerOp(std::ostream &stream, Context &context, std::string dest_reg, const NodePtr& node) const
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

Type LogicalExpression::NewPointerType(Context &context) const
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

