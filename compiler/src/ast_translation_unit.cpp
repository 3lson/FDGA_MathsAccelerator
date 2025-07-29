#include "../include/ast_translation_unit.hpp"

namespace ast{

void TranslationUnit::EmitElsonV(std::ostream &stream, Context &context, std::string dest_reg) const{
    for (const auto& node : get_nodes()){
        if (node == nullptr){
            continue;
        }
        const FunctionDefinition *function_definition = dynamic_cast<const FunctionDefinition *>(node.get());
        const Declaration *declaration = dynamic_cast<const Declaration*>(node.get());

        if (function_definition != nullptr)
        {
            function_definition->EmitElsonV(stream, context, dest_reg);
        }
        else if (declaration != nullptr)
        {
            declaration->DeclareGlobal(stream, context, dest_reg);
        }
    }
}

}//namespace ast
