#include "../../include/control_flow/ast_switch.hpp"
#include "../../include/statements/ast_statements.hpp"

namespace ast {

    void SwitchStatement::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const {
        (void)dest_reg;
        (void)context;
        (void)stream;

        std::string condition_reg = context.get_register(Type::_INT);

        condition_->EmitRISC(stream, context, condition_reg);

        const CompoundStatement *compound_statement = dynamic_cast<const CompoundStatement*>(switch_cases_.get());

        compound_statement->GetCases(stream,context,condition_reg, dest_reg);

    }

    void SwitchStatement::Print(std::ostream& stream) const {
        (void)stream;
    }

    void CaseStatement::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const {

        std::vector<const Node*> statements = GetStatements(stream, context, dest_reg);


        for(const auto& statement : statements){
            if (statement != nullptr){ // safety check
                statement->EmitRISC(stream, context, dest_reg);
            }
        }

    }

    std::vector<const Node*> CaseStatement::GetStatements(std::ostream &stream, Context &context, std::string dest_reg) const{
        (void)dest_reg;
        (void)context;
        (void)stream;

        std::vector<const Node*> statementList;
        const StatementList *statement_list = dynamic_cast<const StatementList*>(statement_list_.get());
        for (const auto& iterator : statement_list->get_nodes())
        {
            const CaseStatement *caseinstance = dynamic_cast<const CaseStatement*>(iterator.get());
            if(!caseinstance){
                const Node *statement = dynamic_cast<const Node*>(iterator.get());
                statementList.push_back(statement);
            }
            else{
                break;
            }
        }


        return statementList;
    }

    void CaseStatement::Print(std::ostream& stream) const {
        (void)stream;
    }

    std::string CaseStatement::Getcasereg(std::ostream& stream, Context& context, std::string dest_reg) const {
        (void)dest_reg;

        std::string condition_reg;
        if(condition_){
            condition_reg = context.get_register(Type::_INT);
            condition_->EmitRISC(stream, context, condition_reg);
        }
        else{
            condition_reg = "default";
        }

        return condition_reg;
    }

    const StatementList* CaseStatement::GetStatementList() const {
        const StatementList *statement_list = dynamic_cast<const StatementList*>(statement_list_.get());
        return statement_list;
    }

 }
