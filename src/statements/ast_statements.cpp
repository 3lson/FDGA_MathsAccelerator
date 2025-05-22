#include "../../include/statements/ast_statements.hpp"
#include "../../include/control_flow/ast_switch.hpp"
#include <iostream>

namespace ast {

void CompoundStatement::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    context.create_scope();
    for (const auto& statement : get_nodes())
    {
        statement->EmitRISC(stream, context, dest_reg);
    }
    context.pop_scope();
}

int CompoundStatement::get_offset(Context &context) const
{
    int offset = 0;
    for (const auto& node : get_nodes())
    {
        if (!node){
            continue;
        }
        const Statement *statement = dynamic_cast<const Statement *>(node.get());
        if (statement){
            offset += statement->get_offset(context);
        }
        else {

        }
    }
    return offset;
}

void CompoundStatement::GetCases(std::ostream &stream, Context &context, std::string condition, std::string dest_reg) const{
    (void)condition;

    std::vector<std::string> caselabels;
    std::vector<const CaseStatement*> casesList;

    std::string endcases = context.create_label("endcases"); // to be used for when breaks are introduced
    context.push_end_label(endcases);

    for(const auto& node : get_nodes()){
        if(!node){
            continue;
        }
        const StatementList *statement_list = dynamic_cast<const StatementList*>(node.get());
        if(statement_list){
            statement_list->GetCases(stream, context,dest_reg,casesList);
        }
    }

    for(const auto& caseitem : casesList){
        std::string caselabel = context.create_label("case");
        std::string casereg = caseitem->Getcasereg(stream,context,dest_reg);
        if(casereg == "default"){
            stream << "j " << caselabel << std::endl;
        }
        else{
            stream << "beq " << condition << ", " << casereg << ", " << caselabel << std::endl;
        }
        caselabels.push_back(caselabel);
    }

    stream << "j " << endcases << std::endl;

    int count = 0;
    for(const auto& caseitem : casesList){
        stream << caselabels[count] << ":" << std::endl;
        caseitem->EmitRISC(stream,context,dest_reg);
        count++;
    }

    stream << endcases << ":" << std::endl;

    context.pop_end_label();
}

void StatementList::GetCases(std::ostream &stream, Context &context, std::string dest_reg,std::vector<const CaseStatement*> &casesList) const{
    (void)stream;
    (void)context;
    (void)dest_reg;

    for (const auto& statement : get_nodes())
    {
        const CaseStatement *caseinstance = dynamic_cast<const CaseStatement*>(statement.get());
        if(caseinstance){
            casesList.push_back(caseinstance);
            caseinstance->GetStatementList()->GetCases(stream,context,dest_reg,casesList);
        }

    }

}


void StatementList::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{

    for (const auto& statement : get_nodes())
    {
        statement->EmitRISC(stream, context, dest_reg);
    }

}

int StatementList::get_offset(Context &context) const
{
    (void)context;
    return 0;
}

int DeclarationList::get_offset(Context &context) const
{
    std::cout << "Entering DeclarationList::get_offset" <<std::endl;
    int offset = 0;
    for (const auto& node : get_nodes())
    {
        const Declaration *declaration = dynamic_cast<const Declaration *>(node.get());
        if (!declaration) {
            continue; // Skip null entries
        }

        std::cout << "Processing a declaration..." << std::endl;
        offset += declaration->get_offset(context);

        if (offset % 4 !=0){
            offset += 4 - (offset % 4);
        }
    }
    return offset;
}

}

