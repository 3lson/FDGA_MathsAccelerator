#include "../../include/control_flow/ast_break.hpp"

namespace ast{
void BreakStatement::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const{
    (void)dest_reg;
    stream << "j " << context.get_end_label() << std::endl;
}

void BreakStatement::Print(std::ostream& stream) const {
    stream << "break;" << std::endl;
}

}
