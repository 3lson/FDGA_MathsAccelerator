#include "../../include/statements/ast_declaration.hpp"
namespace ast {

void Declaration::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    (void)context;
    (void)dest_reg;
    const Typedef *typedef_definition = dynamic_cast<const Typedef *>(type_specifier_.get());
    if (typedef_definition){
        return;
    }

    const Specifier *specifier = dynamic_cast<const Specifier*>(type_specifier_.get());
    if (!specifier) {
        throw std::runtime_error("Declaration::EmitRISC: Invalid type specifier");
    }
    specifier->define_spec(context);

    const StructSpecifier *struct_specifier = dynamic_cast<const StructSpecifier *>(type_specifier_.get());
    if(struct_specifier){
        struct_specifier->define_spec(context);
    }

    if (declarator_list_ == nullptr)
    {
        return;
    }

    Type type = GetType();
    int type_size = types_size.at(type);

    // Cast to const NodeList since declarator_list_ is a unique_ptr<const Node>
    const NodeList *declarator_list = dynamic_cast<const NodeList *>(declarator_list_.get());

    for (const auto &declarator_ptr : declarator_list->get_nodes()) // Use const reference
    {
        // Dynamic ast different declaration types
        const Assignment *assignment = dynamic_cast<const Assignment *>(declarator_ptr.get());
        const Identifier *identifier = dynamic_cast<const Identifier *>(declarator_ptr.get());
        const ArrayDeclaration * array_declaration = dynamic_cast<const ArrayDeclaration *>(declarator_ptr.get());
        const PointerDeclaration *pointer_declaration = dynamic_cast<const PointerDeclaration *>(declarator_ptr.get());

        int offset = context.get_stack_offset();

        if (assignment != nullptr)
        {
            assignment->local_init(type, offset, stream, context);
        }
        else if (identifier != nullptr)
        {
            context.increase_stack_offset(type_size);
            std::string variable_name = identifier->GetId();
            Variable variable(false, false, type, offset, 0);
            context.define_variable(variable_name, variable);
        }
        else if (array_declaration != nullptr)
        {
            int array_size = array_declaration->GetArraySize();

            if (array_declaration->GetArraySize() == -1)
            {
                throw std::runtime_error("Declaration EmitRISC: Array size not specified");
            }

            context.increase_stack_offset(type_size * array_size);

            std::string variable_name = array_declaration->GetId();

            Variable variable(false, true, array_size, type, offset, 0);
            context.define_variable(variable_name, variable);
        }
        else if (pointer_declaration != nullptr)
        {
            context.increase_stack_offset(types_size.at(Type::_INT));

            // Get variable name
            std::string variable_name = pointer_declaration->GetId();

            // Add variable to bindings
            int dereference_num = pointer_declaration->get_deref();
            Variable variable(true, false, type, offset, dereference_num);
            context.define_variable(variable_name, variable);
        }
        else
        {
            throw std::runtime_error("Declaration EmitRISC: Unknown declarator type");
        }
    }
}

void Declaration::Print(std::ostream &stream) const
{
    type_specifier_->Print(stream);
    stream << " ";

    if (declarator_list_)
    {
        declarator_list_->Print(stream);
    }
    stream << ";" << std::endl;
}

int Declaration::get_offset(Context &context) const
{
    (void)context;

    std::cout << "Entering Declaration::get_offset()" << std::endl;
    std::cout << "Declaration::get_offset: type_specifier_ = " << type_specifier_.get() << std::endl;
    std::cout << "Declaration::get_offset: declarator_list_ = " << declarator_list_.get() << std::endl;
    if (!type_specifier_) {
        throw std::runtime_error("Declaration::get_offset: type_specifier_ is null");
    }
    const Specifier *type_specifier = dynamic_cast<const Specifier*>(type_specifier_.get());
    if (!type_specifier) {
        throw std::runtime_error("Declaration::get_offset: Invalid type specifier");
    }
    Type type = type_specifier->GetType();
    int type_size = types_size.at(type);

    const NodeList *declarator_list = dynamic_cast<const NodeList *>(declarator_list_.get());

    int total_size = 0;

    if (declarator_list_ != nullptr)
    {
        for (const auto &declarator_ : declarator_list->get_nodes()) {
            std::cout << "Processing declarator..." << std::endl;
            const Assignment *assignment = dynamic_cast<const Assignment *>(declarator_.get());
            const ArrayDeclaration *array_declaration = dynamic_cast<const ArrayDeclaration *>(declarator_.get());
            const Identifier *identifier = dynamic_cast<const Identifier*>(declarator_.get());
            const PointerDeclaration *pointer_declaration = dynamic_cast<const PointerDeclaration*>(declarator_.get());

            if (assignment != nullptr) {
                //int actual_size = assignment->isPointerInitialization() ? types_size.at(Type::_INT) : type_size;
                type_size = total_size + type_size * assignment->GetArraySize();
            }
            else if (array_declaration != nullptr) {
                int actual_type_size = array_declaration->isPointer() ? types_size.at(Type::_INT) : type_size;
                total_size = total_size + actual_type_size * array_declaration->GetArraySize();
            } else if (identifier != nullptr) {
                type_size = total_size + type_size;
            }
            else if (pointer_declaration != nullptr){
                type_size = total_size + types_size.at(Type::_INT);
            }

            if (type_size % 4 != 0){
                type_size = type_size + 4 - (type_size% 4);
            }
        }
    }
    return total_size;
}

Type Declaration::GetType() const {
    const Specifier *type_specifier = dynamic_cast<const Specifier *>(type_specifier_.get());
    if (!type_specifier)
    {
        throw std::runtime_error("Declaration::GetType: Invalid type specifier");
    }

    return type_specifier->GetType();
}

void Declaration::DeclareGlobal(std::ostream &stream, Context &context, std::string dest_reg) const
{
    (void)dest_reg;

    const Typedef *typedef_definition = dynamic_cast<const Typedef *>(type_specifier_.get());
    if (typedef_definition){
        return;
    }

    const Specifier *specifier = dynamic_cast<const Specifier *>(type_specifier_.get());
    if (!specifier) {
        throw std::runtime_error("Declaration::DeclareGlobal: Invalid type specifier");
    }
    specifier->define_spec(context);

    if (declarator_list_ == nullptr)
    {
        return;
    }
    Type type = GetType();
    //int type_size = types_size.at(type);

    const NodeList *declarator_list = dynamic_cast<const NodeList *>(declarator_list_.get());
    for (const auto& declarator_ : declarator_list->get_nodes())
    {
        const Assignment *assignment = dynamic_cast<const Assignment *>(declarator_.get());
        const Identifier *identifier = dynamic_cast<const Identifier *>(declarator_.get());
        const ArrayDeclaration *array_declaration = dynamic_cast<const ArrayDeclaration *>(declarator_.get());
        const PointerDeclaration *pointer_declaration = dynamic_cast<const PointerDeclaration*>(declarator_.get());
        const Declarator *declarator = dynamic_cast<const Declarator*>(declarator_.get());

        //int offset = context.get_stack_offset();

        if (assignment != nullptr)
        {
            int array_size = assignment->GetArraySize();

            bool is_array = assignment->isArrayInitialization();
            bool is_pointer = assignment->isPointerInitialization();
            int dereference_num = assignment->get_deref();

            std::string global_name = assignment->GetId();
            Global global(is_pointer, is_array, array_size, type, dereference_num);
            assignment->global_init(stream, context, global);
            context.define_global(global_name, global);
        }

        else if (identifier != nullptr)
        {
            std::string global_name = identifier->GetId();

            Global global = Global(false, false, type, 0);
            context.define_global(global_name, global);
        }

        else if (array_declaration != nullptr)
        {
            int array_size = array_declaration->GetArraySize();

            if (array_declaration->GetArraySize() == -1)
            {
                throw std::runtime_error("Declaration EmitRISC: Array size not specified");
            }

            std::string global_name = array_declaration->GetId();
            int dereference_num = array_declaration->get_deref();
            Global global(false, true, array_size, type, dereference_num);
            context.define_global(global_name, global);
        }
        else if (pointer_declaration != nullptr)
        {
            std::string global_name = pointer_declaration->GetId();

            int dereference_num = pointer_declaration->get_deref();
            Global global(true, false, type, dereference_num);
            context.define_global(global_name, global);
        }
        else if (declarator != nullptr)
        {
            // Get function name
            std::string function_name = declarator->GetId();

            // Define function return value and parameters
            bool return_is_pointer = declarator->isPointer();
            int dereference_num = declarator->get_deref();
            ReturnValue return_value = ReturnValue(return_is_pointer, false, type, dereference_num);
            std::vector<Parameter> arguments = declarator->get_param(context);

            Function function = Function(return_value, arguments);
            context.define_function(function_name, function);
        }

        else
        {
            throw std::runtime_error("Declaration DeclareGlobal: Unknown declarator type");
        }
    }
}

}
