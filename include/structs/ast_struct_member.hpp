#pragma once

#include "../ast_node.hpp"
#include "../specifiers/ast_specifier.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../ast_direct_declarator.hpp"
#include "../ast_declarator.hpp"
#include "../pointers/ast_pointer_declaration.hpp"

namespace ast{
class StructMember : public Node{
private:
    NodePtr specifier_qualifier_list_;
    NodePtr struct_declarator_list_;

public:
    StructMember(NodePtr specifier_qualifier_list, NodePtr struct_declarator_list)
    : specifier_qualifier_list_(std::move(specifier_qualifier_list)), struct_declarator_list_(std::move(struct_declarator_list)) {}

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
    std::string GetId() const;
    Type GetType() const;

};


}//namespace ast