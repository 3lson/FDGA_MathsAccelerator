#pragma once

#include "ast_node.hpp"
#include "../specifiers/ast_specifier.hpp"
#include "../symbols/ast_identifier.hpp"
#include "ast_struct_declaration.hpp"
#include "ast_struct_member.hpp"

namespace ast {

class StructSpecifier : public Specifier
{
private:
    std::string *identifier_;
    NodeList *struct_declaration_list_;

public:
    StructSpecifier(std::string *identifier, NodeList *struct_declaration_list)
        : identifier_(identifier), struct_declaration_list_(struct_declaration_list) {}

    ~StructSpecifier()
    {
        if (identifier_)
        {
            delete identifier_;
        }
        if (struct_declaration_list_)
        {
            delete struct_declaration_list_;
        }
    }

    void define_spec(Context &context) const override;
    virtual Type GetType() const override;
    virtual void Print(std::ostream &stream) const override;
    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    std::string GetId() const;
};

} // namespace ast

