#include "../../include/functions/ast_parameter_definition.hpp"

namespace ast{

void ParameterDefinition::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    Type type = isPointer() ? Type::_INT : GetType();
    int offset = context.get_stack_offset();
    stream << context.store_instr(type) << " " << dest_reg << ", " << offset << "(s0)" << std::endl;

    Variable variable(isPointer(), false, GetType(), offset, get_deref());
    context.define_variable(GetId(), variable);
    context.increase_stack_offset(GetTypeSize());
}

void ParameterDefinition::Print(std::ostream &stream) const{
    type_specifier_->Print(stream);
    stream << " ";
    declarator_->Print(stream);
}

void ParameterList::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const{
    int register_num;
    int int_register = 10; //parameter register a0 start
    int float_register = 42;
    std::string register_name;

    int stack_offset = context.get_stack_offset();

    for (size_t i=0; i < get_nodes().size(); ++i){
        const auto& node = get_nodes()[i];

        if (!node){
            continue;
        }
        const ParameterDefinition* parameter = dynamic_cast< const ParameterDefinition*>(node.get());

        Type type = parameter->isPointer() ? Type::_INT : parameter->GetType();

        if (i<8) {
            switch(type){
                case Type::_CHAR:
                case Type::_SHORT:
                case Type::_INT:
                case Type::_UNSIGNED_INT:
                case Type::_LONG:
                    register_num = int_register++;
                    break;
                case Type::_FLOAT:
                case Type::_DOUBLE:
                    register_num = float_register++;
                    break;
                default:
                    throw std::runtime_error("ParameterDefinition::EmitRISC - Invalid type");
            }

            register_name = context.get_register_name(register_num);
            parameter->EmitRISC(stream, context, register_name);
        } else {
            stack_offset -= types_size.at(type);

            std::cout << "Stack_offset: " << stack_offset << std::endl;

            //Alignment
            int offset = (-1 * stack_offset) + 4;

            std::cout << "Offset: " << offset << std::endl;


            std::string temp_reg = context.get_register(type);
            stream << context.load_instr(type) << " " << temp_reg << ", " << offset << "(sp)" << std::endl;

            parameter->EmitRISC(stream, context, temp_reg);

            context.deallocate_register(temp_reg);
        }

    }
    (void)dest_reg;

}

std::vector<Parameter> ParameterList::get_param(Context &context) const {
    int start_offset = context.get_stack_offset();
    std::vector<Parameter> parameters;

    for (const auto& node : get_nodes()) {
        const ParameterDefinition* parameter = dynamic_cast<const ParameterDefinition*>(node.get());
        Parameter Parameter = parameter->GetParameter(context, start_offset - get_offset());
        parameters.push_back(Parameter);
    }
    return parameters;
}

int ParameterList::get_offset() const {
    int size = 0;

    for (Parameter Parameter : parameters)
    {
        size += Parameter.GetTypeSize();

        // Align offset to 4 bytes
        if (size % 4 != 0)
        {
            size += 4 - (size % 4);
        }
    }
    return size;
}

std::string ParameterDefinition::GetId() const
{
    const Identifier* identifier = dynamic_cast<const Identifier*>(declarator_.get());
    const PointerDeclaration *pointer_declaration = dynamic_cast<const PointerDeclaration *>(declarator_.get());
    if (identifier){
        return identifier->GetId();
    }
    else if (pointer_declaration)
    {
        return pointer_declaration->GetId();
    }
    throw std::runtime_error("ParameterDeclaration::GetId() - declarator_ is not an Identifier or PointerDeclarator");
}

Type ParameterDefinition::GetType() const
{
    const Specifier* type_specifier_ptr = dynamic_cast<const Specifier*>(type_specifier_.get());
    (void)context;
    return type_specifier_ptr->GetType();
}

Parameter ParameterDefinition::GetParameter(Context &context, int offset) const
{
    (void)context;
    if (isPointer())
    {
        return Parameter(GetId(), true, false, GetType(), offset, get_deref());
    }
    return Parameter(GetId(), false, false, GetType(), offset, 0);
}

int ParameterDefinition::GetTypeSize() const
{
    if (isPointer()){
        types_size.at(Type::_INT);
    }
    return types_size.at(GetType());
}


bool ParameterDefinition::isPointer() const
{

    if (dynamic_cast<const PointerDeclaration *>(declarator_.get()))
    {
        return true;
    }

    return false;
}

int ParameterDefinition::get_deref() const
{
    // Get dereference number for parameter
    const Declarator *declarator = dynamic_cast<const Declarator *>(declarator_.get());

    if (declarator)
    {
        return declarator->get_deref();
    }

    return 0;
}


}
