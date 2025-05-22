#pragma once

#include "../ast_node.hpp"
#include "../specifiers/ast_type_specifier.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../ast_direct_declarator.hpp"
#include "../arrays/ast_array_declaration.hpp"
#include "ast_assignment.hpp"
#include "../typedef/ast_typedef.hpp"
#include "../structs/ast_struct_declaration.hpp"
#include "../structs/ast_struct_specifier.hpp"

namespace ast{

class Declaration : public Node
{
private:
    NodePtr type_specifier_;
    NodePtr declarator_list_;

public:
    Declaration(NodePtr type_specifier) : type_specifier_(std::move(type_specifier)), declarator_list_(nullptr) {}
    Declaration(NodePtr type_specifier, NodePtr declarator_list) : type_specifier_(std::move(type_specifier)), declarator_list_(std::move(declarator_list)) {}

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
    int get_offset(Context& context) const;
    Type GetType() const;
    void DeclareGlobal(std::ostream &stream, Context &context, std::string dest_reg) const;
};

}//namespace ast
