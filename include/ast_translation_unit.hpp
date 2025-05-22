#pragma once

#include "ast_node.hpp"
#include "functions/ast_function_definition.hpp"

namespace ast{
class TranslationUnit : public NodeList{
public:
    using NodeList::NodeList;

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
};

} //namespace ast
