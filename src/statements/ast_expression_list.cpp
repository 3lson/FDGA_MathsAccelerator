#include "../../include/statements/ast_expression_list.hpp"

namespace ast{

Type Expression::GetType(Context &context) const
{
    return dynamic_cast<const Operand *>(nodes_[0].get())->GetType(context);
}

void Expression::PushBack(NodePtr item)
{
    nodes_.push_back(std::move(item));
}

void Expression::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
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

void Expression::Print(std::ostream &stream) const
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


void ExpressionList::GetArguments(std::ostream &stream, Context &context, std::string dest_reg) const
{
    (void)dest_reg;
    Function function = context.get_function_call();
    int int_reg = 10, float_reg = 42;
    int arg_reg_num;

    Type type;

    for (size_t arg = 0; arg < function.get_parameters().size(); ++arg)
    {
        type = function.get_parameters()[arg].is_pointer() ? Type::_INT : function.get_parameters()[arg].get_type();
        switch(type)
        {
            case Type::_CHAR:
            case Type::_UNSIGNED_INT:
            case Type::_INT:
                arg_reg_num = int_reg++;
                break;
            case Type::_FLOAT:
            case Type::_DOUBLE:
                arg_reg_num = float_reg++;
                break;
            default:
                throw std::runtime_error("ExpressionList GetArguments: Unknown Type");
        }

        std::string arg_reg_name = context.get_register_name(arg_reg_num);
        context.push_operation_type(type);
        dynamic_cast<const Operand*>(get_nodes()[arg].get())->EmitRISC(stream, context, arg_reg_name);
        context.pop_operation_type();
    }
}

void ExpressionList::Print(std::ostream &stream) const
{
    for (const auto& node : get_nodes())
    {
        if (!node){
            continue;
        }
        node->Print(stream);
        stream << ", ";
    }
    stream << std::endl;
}

bool Expression::isPointerOp(Context &context) const
{
    return dynamic_cast<const Operand *>(nodes_[0].get())->isPointerOp(context);
}

}
