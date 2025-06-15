#pragma once

#include "ast_node.hpp"
#include "../context/ast_context.hpp"
#include "../context/ast_context_kernel.hpp"
#include "../symbols/ast_constant.hpp"

namespace ast {

class KernelStatement : public Node {
private:
    NodePtr threads_;
    NodePtr compound_statement_;

    void InitializeKernel(Context& context) const;

public:
    KernelStatement(NodePtr threads, NodePtr compound_statement)
        : threads_(std::move(threads)), compound_statement_(std::move(compound_statement)) {}

    void EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
};

}
