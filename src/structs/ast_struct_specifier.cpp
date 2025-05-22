#include "../../include/structs/ast_struct_specifier.hpp"

namespace ast{

void StructSpecifier::define_spec(Context& context) const{
    std::cout << "Defining struct: " <<*identifier_ <<std::endl;
    std::unordered_map<std::string, Type> structMembers;
    int total_size = 0;
    Type struct_type = context.get_or_create_struct_type(*identifier_);
    std::cout << "The CUSTOM_STRUCT type "<<static_cast<int>(struct_type) << std::endl;
    for (const auto& node : struct_declaration_list_->get_nodes()){
        if (!node){
            continue;
        }
        const StructMember* member = dynamic_cast<const StructMember *>(node.get());
        if (member){
            std::string memberName = member->GetId();
            Type memberType = member->GetType();
            structMembers[memberName] = memberType;

            int memberSize = types_size.at(memberType);
            total_size += memberSize;

            std::cout << "Added Struct Member: " << memberName
            << " with type: " << static_cast<int>(memberType)
            << " size: " << memberSize << std::endl;
        }
    }
    context.struct_init(*identifier_, structMembers);
    std::cout << "For this type the size: " <<context.get_struct_size(struct_type) << std::endl;
}

void StructSpecifier::EmitRISC(std::ostream &stream, Context& context, std::string dest_reg) const{
    (void)stream;
    (void)dest_reg;
    define_spec(context);
}


void StructSpecifier::Print(std::ostream &stream) const{
    stream << "struct " << *identifier_ << "{ ";
    struct_declaration_list_->Print(stream);
    stream << "}";

}

Type StructSpecifier::GetType() const{
    return Type::_STRUCT;
}

std::string StructSpecifier::GetId() const{
    return *identifier_;
}

}//namespace ast
