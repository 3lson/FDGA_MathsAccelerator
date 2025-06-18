#include "../../include/operations/ast_unary_operation.hpp"

namespace ast {

std::string UnaryExpression::GetOperation(Type type) const {
    static const std::unordered_map<UnaryOp, std::unordered_map<Type, std::string>> opMap = {
        {UnaryOp::INC, {
            {Type::_INT, "addi"}, {Type::_UNSIGNED_INT, "addi"}, {Type::_CHAR, "addi"},
            {Type::_SHORT, "addi"}, {Type::_LONG, "addi"}, {Type::_FLOAT, "add"}, {Type::_DOUBLE, "add"}
        }},
        {UnaryOp::DEC, {
            {Type::_INT, "addi"}, {Type::_UNSIGNED_INT, "addi"}, {Type::_CHAR, "addi"},
            {Type::_SHORT, "addi"}, {Type::_LONG, "addi"}, {Type::_FLOAT, "fsub.s"}, {Type::_DOUBLE, "fsub.d"}
        }},
        {UnaryOp::PLUS, {
            {Type::_INT, "mv"}, {Type::_UNSIGNED_INT, "mv"}, {Type::_CHAR, "mv"},
            {Type::_SHORT, "mv"}, {Type::_LONG, "mv"},
            {Type::_FLOAT, "fadd.s"}, {Type::_DOUBLE, "fadd.d"}
        }},
        {UnaryOp::MINUS, {
            {Type::_INT, "neg"}, {Type::_UNSIGNED_INT, "sub"}, {Type::_CHAR, "sub"},
            {Type::_SHORT, "sub"}, {Type::_LONG, "sub"},
            {Type::_FLOAT, "fneg.s"}, {Type::_DOUBLE, "fneg.d"}
        }},
        {UnaryOp::BITWISE_NOT, {
            {Type::_INT, "not"}, {Type::_UNSIGNED_INT, "not"}, {Type::_CHAR, "not"},
            {Type::_SHORT, "not"}, {Type::_LONG, "not"},
        }},
        {UnaryOp::LOGICAL_NOT, {
            {Type::_INT, "seqz"}, {Type::_UNSIGNED_INT, "seqz"}, {Type::_CHAR, "seqz"},
            {Type::_SHORT, "seqz"}, {Type::_LONG, "seqz"}, {Type::_FLOAT, "feq.s"},
        }},
    };

    auto it = opMap.find(op_);
    if (it != opMap.end()) {
        auto typeIt = it->second.find(type);
        if (typeIt != it->second.end()) {
            return typeIt->second;
        }
    }

    std::cerr << "Unsupported unary operation or type: Operation = "
    << static_cast<int>(op_) << ", Type = " << static_cast<int>(type) << std::endl;

    throw std::runtime_error("Unsupported unary operation or type.");
}

void UnaryExpression::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {
    Type type = GetType(context);

    context.push_operation_type(type);

    std::string operand_register = context.get_register(type);


    // Handle struct access (as per your existing logic)
    const StructAccess* structAccess = dynamic_cast<const StructAccess*>(operand_.get());
    if (structAccess) {
        structAccess->EmitElsonV(stream, context, operand_register);
    }else{
        operand_->EmitElsonV(stream, context, operand_register);
    }

    std::string operation = GetOperation(type);

    //temporary fix for zero reg issue (IMPORTANT)
    if (dest_reg == "zero") {
        dest_reg = context.get_register(type); // Allocate a new temporary register
    }

    // Emit the RISC-V instruction for the unary operation
    if (op_ == UnaryOp::INC) {
        stream << asm_prefix.at(context.get_instruction_state()) << operation << " " << dest_reg << ", " << operand_register << ", 1" << std::endl;
    } else if (op_ == UnaryOp::DEC) {
        stream << asm_prefix.at(context.get_instruction_state()) << operation << " " << dest_reg << ", " << operand_register << ", -1" << std::endl;
    } else if (op_ == UnaryOp::PLUS) {
        stream << asm_prefix.at(context.get_instruction_state()) << operation << " " << dest_reg << ", " << operand_register << std::endl;
    } else if (op_ == UnaryOp::MINUS) {
        stream << asm_prefix.at(context.get_instruction_state()) << operation << " " << dest_reg << ", " << operand_register << std::endl;
    } else if (op_ == UnaryOp::BITWISE_NOT) {
        stream << asm_prefix.at(context.get_instruction_state()) << operation << " " << dest_reg << ", " << operand_register << std::endl;
    } else if (op_ == UnaryOp::LOGICAL_NOT) {
        if (type == Type::_FLOAT) {
            std::string zero_reg = context.get_register(type); 
            if (zero_reg.rfind("f", 0) == 0) {
                // It's a floating-point register (e.g., ft0, fa1, etc.)
                std::string temp_int_reg = context.get_register(Type::_INT);
                stream << asm_prefix.at(context.get_instruction_state()) << "li " << temp_int_reg << ", 0" << std::endl;
                stream << asm_prefix.at(context.get_instruction_state()) << "fcvt.s.w " << zero_reg << ", " << temp_int_reg << std::endl;
            } else {
                // It's an integer register
                stream << asm_prefix.at(context.get_instruction_state()) <<  "li " << zero_reg << ", 0" << std::endl;
            }
            stream << asm_prefix.at(context.get_instruction_state()) <<"feq.s " << dest_reg << ", " << operand_register << ", " << zero_reg << std::endl;
            context.deallocate_register(zero_reg);
        } else {
            stream << asm_prefix.at(context.get_instruction_state()) << operation << " " << dest_reg << ", " << operand_register << std::endl;
        }
    }

    //If the operand is an identifier, store the result back to memory
    const Identifier* identifier = dynamic_cast<const Identifier*>(operand_.get());
    if (identifier){
        Variable variable = context.get_variable(identifier->GetId());
        int offset = variable.get_offset();
        stream << asm_prefix.at(context.get_instruction_state()) << context.store_instr(type) << " " << dest_reg << ", " << offset << "(s0)" <<std::endl;
    }

    context.deallocate_register(operand_register);
    context.pop_operation_type();
}

void UnaryExpression::Print(std::ostream& stream) const {
    switch (op_) {
        case UnaryOp::INC: stream << "++"; break;
        case UnaryOp::DEC: stream << "--"; break;
        case UnaryOp::PLUS: stream << "+"; break;
        case UnaryOp::MINUS: stream << "-"; break;
        case UnaryOp::BITWISE_NOT: stream << "!"; break;
        case UnaryOp::LOGICAL_NOT: stream << "~"; break;
    }

    operand_->Print(stream);
}

Type UnaryExpression::GetType(Context& context) const {
    if (!operand_) {
        throw std::runtime_error("Null pointer encountered in UnaryExpression operand.");
    }

    // Check if the operand is a struct access
    const StructAccess* structAccess = dynamic_cast<const StructAccess*>(operand_.get());
    if (structAccess) {
        // If it's a struct access, get the type of the struct field
        return structAccess->GetType(context);
    }

    // Otherwise, treat it as a regular operand and get the type of the operand
    const Operand* operandObj = dynamic_cast<const Operand*>(operand_.get());
    if (!operandObj) {
        throw std::runtime_error("Invalid operand type: " + std::string(typeid(*operand_).name()));
    }

    return operandObj->GetType(context);
}

bool UnaryExpression::isPointerOp(Context &context) const
{
    return dynamic_cast<const Operand *>(operand_.get())->isPointerOp(context);
}

} // namespace ast
