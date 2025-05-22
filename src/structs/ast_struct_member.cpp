#include "../../include/structs/ast_struct_member.hpp"
#include <typeinfo>

namespace ast{

void StructMember::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const{
    //Struct members handled during struct definition
    (void)context;
    (void)dest_reg;
    (void)stream;
}

void StructMember::Print(std::ostream &stream) const{
    specifier_qualifier_list_->Print(stream);
    stream << " ";
    struct_declarator_list_->Print(stream);
}

std::string StructMember::GetId() const{
    const NodeList* node_list = dynamic_cast<const NodeList*>(struct_declarator_list_.get());
    if (!node_list || node_list->get_nodes().empty()) {
        throw std::runtime_error("StructMember::GetId - Empty or invalid struct_declarator_list_");
    }

    // Get the first node in the list
    const Node* first_node = node_list->get_nodes().front().get();
    if(!first_node){
        throw std::runtime_error("StructMember::GetId - Empty struct_declarator_list_");
    }
    const DirectDeclarator* direct_declarator = dynamic_cast<const DirectDeclarator *>(first_node);
    if (direct_declarator){
        return direct_declarator->GetId();
    }
    const Identifier* identifier = dynamic_cast<const Identifier*>(first_node);
    if (identifier) {
        return identifier->GetId();
    }

    throw std::runtime_error("StructMember::GetId - Invalid declarator type");
}

Type StructMember::GetType() const{
    const Specifier* specifier = dynamic_cast<const Specifier *>(specifier_qualifier_list_.get());
    if (specifier){
        return specifier->GetType();
    }
    throw std::runtime_error("StructMember::GetType - Invalid specifier");
}

}//namespace ast