#include "../../include/control_flow/ast_continue.hpp"

namespace ast{
void ContinueStatement::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const{
    (void)dest_reg;
    stream <<  "j " << context.get_start_label() << std::endl;
}

void ContinueStatement::Print(std::ostream& stream) const {
    stream << "continue;" << std::endl;
}

}
