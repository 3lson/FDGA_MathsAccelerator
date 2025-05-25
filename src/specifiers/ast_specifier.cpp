#include "../../include/specifiers/ast_specifier.hpp"
namespace ast{
void Specifier::EmitElsonV(std::ostream &stream, Context &context, std::string dest_reg) const{
    throw std::runtime_error("Specifier::EmitElsonV not implemented");
    (void)context;
    (void)dest_reg;
    (void)stream;
}
}//namespace ast
