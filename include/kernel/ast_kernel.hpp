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

    // Core kernel initialization and setup
    void InitializeKernel(std::string start_kernel_label, std::ostream& stream, Context& context) const;
    void InitializeFirstWarp(std::ostream& stream, Context& context) const;
    
    // Warp management and switching
    void InitializeWarp(std::ostream& stream, Context& context, Warp& warp) const;
    void StoreWarpRegisters(std::ostream& stream, Context& context, Warp& warp) const;
    void EmitWarpSwitchLogic(std::ostream& stream, Context& context, 
                           const std::string& kernel_start_label,
                           const std::string& kernel_end_label) const;
    
    // Kernel cleanup and finalization
    void EmitKernelCleanup(std::ostream& stream, Context& context) const;
    
    // Utility functions
    int KernelStackSize(Context& context) const;

public:
    KernelStatement(NodePtr threads, NodePtr compound_statement)
        : threads_(std::move(threads)), compound_statement_(std::move(compound_statement)) {}

    void EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const override;
    void Print(std::ostream& stream) const override;
};

}