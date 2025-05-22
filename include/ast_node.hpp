#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <optional>

#include "context/ast_context.hpp"

namespace ast {

class Node
{
public:
    virtual ~Node() = default;
    virtual void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const = 0;
    virtual void Print(std::ostream& stream) const = 0;
};

// If you don't feel comfortable using std::unique_ptr, you can switch NodePtr to be defined
// as a raw pointer instead here and your project should still compile, although you'll need
// to add destructors to avoid leaking memory
// (and get rid of the now unnecessary std::move-s)
using NodePtr = std::unique_ptr<const Node>; //a smart pointer to a node

class NodeList : public Node
{
private:
    std::vector<NodePtr> nodes_;

public:
    //Add empty nodelist constructor for empty compound statement
    NodeList() {}
    NodeList(NodePtr first_node) {
        if(first_node){
            nodes_.push_back(std::move(first_node));
        }
    }
    virtual ~NodeList();

    void PushBack(NodePtr item);
    virtual void EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const override;
    virtual void Print(std::ostream& stream) const override;
    std::vector<NodePtr> const& get_nodes() const;
};

} // namespace ast
