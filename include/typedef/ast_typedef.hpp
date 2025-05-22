#pragma once

#include "../ast_node.hpp"
#include "../specifiers/ast_specifier.hpp"
#include "../arrays/ast_array_declaration.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../pointers/ast_addressof.hpp"
#include "../pointers/ast_pointer_declaration.hpp"

namespace ast{

class Typedef : public Node
{
private:
    NodePtr declaration_specifier_;
    Type type;
    int initial_ptrs = 0;
    std::string base_type;
    int initial_array_size = 0;

public:
    Typedef(NodePtr declaration_specifier) : declaration_specifier_(std::move(declaration_specifier)) { GetDeclarationSpecifier(); }

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const;
    void Print(std::ostream& stream) const;

    void DefineTypedef(NodeList *aliases);
    void GetDeclarationSpecifier();

};
}//namespace ast