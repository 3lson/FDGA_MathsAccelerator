#include "../../include/enums/ast_enum_specifier.hpp"

namespace ast{
Type EnumeratorSpecifier::GetType() const
{
    return Type::_INT;
}

void EnumeratorSpecifier::define_spec(Context &context) const
{
    if (enumerator_list_ != nullptr)
    {
        int enum_value = 0;
        std::vector<std::string> labels;

        for (const auto& node_ : enumerator_list_->get_nodes())
        {
            const Enumerator *enumerator_ = dynamic_cast<const Enumerator *>(node_.get());
            if (!enumerator_){
                throw std::runtime_error("Node in enumerator_list_ is not an Enumerator");
            }
            enum_value = enumerator_->define_enumerator(context, enum_value);
            labels.push_back(enumerator_->GetLabel());
        }

    }
    if (identifier_ != nullptr)
    {
        context.define_enum(*identifier_);
    }
}

void EnumeratorSpecifier::Print(std::ostream &stream) const
{
    stream << "enum ";

    if (identifier_)
    {
        stream << *identifier_;
    }

    if (enumerator_list_)
    {
        stream << "{";
        enumerator_list_->Print(stream);
        stream << "}";
    }
}

} //namespace ast
