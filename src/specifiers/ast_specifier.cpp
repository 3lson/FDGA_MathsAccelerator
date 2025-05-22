#include "../../include/specifiers/ast_specifier.hpp"
namespace ast{
void Specifier::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const{
    throw std::runtime_error("Specifier::EmitRISC not implemented");
    (void)context;
    (void)dest_reg;
    (void)stream;
}
}//namespace ast
