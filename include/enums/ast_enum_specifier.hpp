#pragma once

#include "../ast_node.hpp"
#include "../specifiers/ast_specifier.hpp"
#include "../symbols/ast_identifier.hpp"
#include "ast_enum.hpp"

namespace ast {

class EnumeratorSpecifier : public Specifier
{
private:
    std::string *identifier_;
    NodeList *enumerator_list_;

public:
    EnumeratorSpecifier(std::string *identifier)
        : identifier_(identifier), enumerator_list_(nullptr) {}

    EnumeratorSpecifier(NodeList *enumerator_list)
        : identifier_(nullptr), enumerator_list_(enumerator_list) {}

    EnumeratorSpecifier(std::string *identifier, NodeList *enumerator_list)
        : identifier_(identifier), enumerator_list_(enumerator_list) {}

    ~EnumeratorSpecifier()
    {
        if (identifier_)
        {
            delete identifier_;
        }
        if (enumerator_list_)
        {
            delete enumerator_list_;
        }
    }

    void define_spec(Context &context) const override;
    virtual Type GetType() const override;
    virtual void Print(std::ostream &stream) const override;
};

} // namespace ast

