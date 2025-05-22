#pragma once

#include "../ast_node.hpp"
#include "../symbols/ast_identifier.hpp"
#include "../symbols/ast_constant.hpp"

namespace ast{

class Enumerator : public Node
{
private:
    std::string label_;
    NodePtr constant_expression_;

public:
    Enumerator(std::string *label) : label_(*label), constant_expression_(nullptr) {delete label;}
    Enumerator(std::string *label, NodePtr constant_expression) : label_(*label), constant_expression_(std::move(constant_expression)) {delete label;}

    int define_enumerator(Context &context, int value) const;
    std::string GetLabel() const;

    int get_val(Context &context) const;
    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
};

}//namespace ast
