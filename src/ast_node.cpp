#include "ast_node.hpp"

namespace ast {

void NodeList::PushBack(NodePtr item)
{
    if (item) {
        nodes_.push_back(std::move(item));
    }
}

void NodeList::EmitRISC(std::ostream& stream, Context& context, std::string dest_reg) const
{
    for (const auto& node : nodes_)
    {
        if (node == nullptr)
        {
            continue;
        }
        node->EmitRISC(stream, context, dest_reg);
    }
}

void NodeList::Print(std::ostream& stream) const
{
    for (const auto& node : nodes_)
    {
        if (node == nullptr)
        {
            continue;
        }
        node->Print(stream);
    }
}

std::vector<NodePtr> const& NodeList::get_nodes() const
{
    return nodes_;
}

NodeList::~NodeList() = default;

}
