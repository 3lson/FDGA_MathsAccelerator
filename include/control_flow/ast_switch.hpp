#pragma once

#include <vector>
#include "ast_node.hpp"
#include "../statements/ast_statements.hpp"
#include "../ast_direct_declarator.hpp"


namespace ast {
    class Statement; //forward declaration
    class StatementList; //second forward declaration

class SwitchStatement : public Node {
private:
    NodePtr condition_;
    NodePtr switch_cases_;

public:
    SwitchStatement(NodePtr condition, NodePtr switch_cases)
        : condition_(std::move(condition)), switch_cases_(std::move(switch_cases)) {}

    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
};

class CaseStatement : public Node {
private:
    NodePtr condition_;
    NodePtr statement_list_;
public:
    CaseStatement(NodePtr condition, NodePtr statement_list)
        : condition_(std::move(condition)), statement_list_(std::move(statement_list)){}


    const StatementList* GetStatementList() const;
    std::vector<const Node*> GetStatements(std::ostream &stream,Context &context, std::string dest_reg) const;
    std::string Getcasereg(std::ostream& stream, Context& context, std::string dest_reg) const;
    void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
};

}
