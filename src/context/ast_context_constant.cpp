#include "../../include/context/ast_context.hpp"
#include "../../include/context/ast_context_types.hpp"

namespace ast{


Type ContextConstant::GetType() const
{
    return type_;
}


void ContextConstant::PrintConst(std::ostream &stream) const {
    switch(type_){
        case Type::_FLOAT:
            stream << "\t" << assembler_directives.at(Type::_FLOAT) << " " << low_bits << std::endl;
            break;
            case Type::_DOUBLE:
            stream << "\t" << assembler_directives.at(Type::_DOUBLE) << " " << low_bits << std::endl;
            stream << "\t" << assembler_directives.at(Type::_DOUBLE) << " " << high_bits << std::endl;
            break;
        default:
            throw std::runtime_error("ContextConstant::PrintConst: Unsupported type");
    }
}

int Context::registerConstant(float value) {
    ContextConstant constant(value);
    constants.push_back(constant);
    return constantIndex++;
}

int Context::registerConstant(double value) {
    ContextConstant constant(value);
    constants.push_back(constant);
    return constantIndex++;
}

void Context::constDecl(std::ostream &stream) const {
    int i = 0;
    stream << "\t.section .rodata" << std::endl;
    for (const auto &constant : constants) {
        stream << "\t.align " << types_mem_shift.at(constant.GetType()) << std::endl;
        stream << ".LC" << i++ << ":" << std::endl;
        constant.PrintConst(stream);
    }
}
}



