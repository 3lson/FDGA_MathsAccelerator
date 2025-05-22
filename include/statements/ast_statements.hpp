#pragma once

#include "../ast_node.hpp"
#include "ast_declaration.hpp"
#include "../control_flow/ast_switch.hpp"

namespace ast {
    class SwitchStatement;
    class CaseStatement;
}


namespace ast{

class Statement : public NodeList
{
public:
    using NodeList::NodeList; //inherits all constructors from nodelist
    virtual ~Statement() override = default;

    virtual int get_offset(Context &context) const = 0;
};

class CompoundStatement : public Statement
{
public:
    using Statement::Statement;
    int get_offset(Context &context) const;
    void GetCases(std::ostream &stream,Context &context, std::string condition, std::string dest_reg) const;
    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
};

class StatementList : public Statement
{
public:
    using Statement::Statement;

    void GetCases(std::ostream &stream,Context &context, std::string dest_reg, std::vector<const CaseStatement*>& casesList) const;
    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    int get_offset(Context &context) const;
};

class DeclarationList : public Statement
{
public:
    using Statement::Statement;

    int get_offset(Context &context) const;
};

}
