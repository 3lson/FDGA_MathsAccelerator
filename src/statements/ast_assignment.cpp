#include "../../include/statements/ast_assignment.hpp"
namespace ast{
void Assignment::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    (void)dest_reg;
    std::cout << "The type of expression_ is " << typeid(*unary_expression_).name() << std::endl;
    const StructAccess * struct_access = dynamic_cast<const StructAccess*>(unary_expression_.get());
    if (struct_access) {
        std::cout << "Entering Assignment::EmitRISC for struct access" << std::endl;
        Type type = struct_access->GetType(context);
        std::cout << "The struct_access type: " << static_cast<int>(type) << std::endl;

        // Evaluate right-hand side first
        std::string value_reg = context.get_register(type);
        expression_->EmitRISC(stream, context, value_reg);

        int offset = struct_access->get_offset(context);
        // Store the value into the struct member
        stream << context.store_instr(type) << " " << value_reg << ", " << offset << "(s0)" << std::endl;

        context.deallocate_register(value_reg);
        return;
    } else {
        std::cout << "Entering Assignment::EmitRISC for everything else" << std::endl;
        Variable variable = context.get_variable(GetId());
        Type type = variable.is_pointer() ? Type::_INT : variable.get_type();
        int offset = variable.get_offset();
        context.push_operation_type(type);

        std::string reg = context.get_register(type);
        expression_->EmitRISC(stream, context, reg);

        if (isArrayInitialization())
        {
            const ArrayInitialization *array_init = dynamic_cast<const ArrayInitialization *>(expression_.get());
            if (array_init) {
                array_init->SaveValue(stream, context, variable, GetId());
            } else {
                throw std::runtime_error("Assignment EmitRISC: Expected an array initialization");
            }
        }

        else
        {
            const Identifier *identifier = dynamic_cast<const Identifier *>(unary_expression_.get());
            const ArrayIndexAccess *array_index_access = dynamic_cast<const ArrayIndexAccess *>(unary_expression_.get());
            const PointerDeclaration *pointer_declaration = dynamic_cast<const PointerDeclaration *>(unary_expression_.get());
            const AddressOf *address_of = dynamic_cast<const AddressOf *>(unary_expression_.get());
            const Dereference *dereference = dynamic_cast<const Dereference *>(unary_expression_.get());

            if (identifier != nullptr)
            {
                if (variable.get_scope() == ScopeLevel::LOCAL)
                {
                    stream << context.store_instr(type) << " " << reg << ", " << offset << "(s0)" << std::endl;
                }

                else if (variable.get_scope() == ScopeLevel::GLOBAL)
                {
                    std::string global_memory_location = "global_" + GetId();
                    std::string global_memory_register = context.get_register(Type::_INT);

                    stream << "lui " << global_memory_register << ", " << "%hi(" << global_memory_location << ")" << std::endl;
                    stream << context.store_instr(type) << " " << reg << ", %lo(" << global_memory_location << ")(" << global_memory_register << ")" << std::endl;
                    context.deallocate_register(global_memory_register);
                }

                else
                {
                    throw std::runtime_error("Assignment EmitRISC: Invalid scope in Identifier");
                }
            }

            // If array access, load expression into specific element by first evaluating index
            else if (array_index_access != nullptr)
            {
                type = array_index_access->isPointerOp(context) ? Type::_INT : array_index_access->GetType(context);
                std::string index_register = context.get_register(Type::_INT);
                array_index_access->get_position(stream, context, index_register, type);

                if (variable.get_scope() == ScopeLevel::LOCAL)
                {
                    if (variable.is_array())
                    {
                        // Add index to base pointer
                        stream << "add " << index_register << ", " << index_register << ", s0" << std::endl;
                        stream << "addi " << index_register << ", " << index_register << ", " << offset << std::endl;
                    }
                    else if (variable.is_pointer())
                    {
                        std::string pointer_register = context.get_register(Type::_INT);
                        stream << context.load_instr(Type::_INT) << " " << pointer_register << ", " << offset << "(s0)" << std::endl;
                        stream << "add " << index_register << ", " << index_register << ", " << pointer_register << std::endl;
                        context.deallocate_register(pointer_register);
                    }
                    else
                    {
                        throw std::runtime_error("Assignment EmitRISC: Variable is not a pointer or array in ArrayAccess LOCAL");
                    }

                    stream << context.store_instr(type) << " " << reg << ", 0(" << index_register << ")" << std::endl;
                }

                // If global scope, access global memory by targetting global label
                else if (variable.get_scope() == ScopeLevel::GLOBAL)
                {
                    std::string global_memory_location = "global_" + GetId();
                    std::string global_memory_register = context.get_register(Type::_INT);

                    // Access global memory by targetting global label
                    stream << "lui " << global_memory_register << ", " << "%hi(" << global_memory_location << ")" << std::endl;
                    stream << "add " << global_memory_register << ", " << global_memory_register << ", " << index_register << std::endl;
                    stream << context.store_instr(type) << " " << reg << ", %lo(" << global_memory_location << ")(" << global_memory_register << ")" << std::endl;

                    context.deallocate_register(global_memory_register);
                }

                else
                {
                    throw std::runtime_error("Assignment EmitRISC: Invalid scope in ArrayAccess");
                }

                context.deallocate_register(index_register);
            }
            else if (pointer_declaration != nullptr)
            {
                // If local scope, access variable through offset specified in bindings
                if (variable.get_scope() == ScopeLevel::LOCAL)
                {
                    stream << context.store_instr(type) << " " << reg << ", " << variable.get_offset() << "(s0)" << std::endl;
                }

                // If global scope, access global memory by targetting global label
                else if (variable.get_scope() == ScopeLevel::GLOBAL)
                {
                    std::string global_memory_location = "global_" + GetId();
                    std::string global_memory_register = context.get_register(Type::_INT);

                    // Access global memory by targetting global label
                    stream << "lui " << global_memory_register << ", " << "%hi(" << global_memory_location << ")" << std::endl;
                    stream << context.store_instr(type) << " " << reg << ", %lo(" << global_memory_location << ")(" << global_memory_register << ")" << std::endl;
                    context.deallocate_register(global_memory_register);
                }
            }
            else if (address_of != nullptr)
            {
                if (variable.get_scope() == ScopeLevel::LOCAL)
                {
                    stream << context.store_instr(type) << " " << reg << ", " << variable.get_offset() << "(s0)" << std::endl;
                }

                else if (variable.get_scope() == ScopeLevel::GLOBAL)
                {
                    std::string global_memory_location = "global_" + GetId();
                    std::string global_memory_register = context.get_register(Type::_INT);

                    stream << "lui " << global_memory_register << ", " << "%hi(" << global_memory_location << ")" << std::endl;
                    stream << context.store_instr(type) << " " << reg << ", %lo(" << global_memory_location << ")(" << global_memory_register << ")" << std::endl;
                    context.deallocate_register(global_memory_register);
                }

                else
                {
                    throw std::runtime_error("Assignment EmitRISC: Invalid scope in AddressOf");
                }
            }
            else if (dereference != nullptr)
            {
                std::string address_register = context.get_register(Type::_INT);
                Type type = dereference->GetType(context);

                dereference->StartingOffset(stream, context, address_register);
                dereference->deref_route(stream, context, address_register);
                stream << context.store_instr(type) << " " << reg << ", 0(" << address_register << ")" << std::endl;

                context.deallocate_register(address_register);
            }

            else
            {
                throw std::runtime_error("Assignment EmitRISC: Not an identifier or array access or pointer declaration");
            }
        }

        context.deallocate_register(reg);
        context.pop_operation_type();
        return;
    }
    throw std::runtime_error("Assignment::EmitRISC - no identifier found");
}

void Assignment::Print(std::ostream &stream) const
{
    unary_expression_->Print(stream);
    stream << " = ";
    expression_->Print(stream);
    stream << ";" << std::endl;
}

std::string Assignment::GetId() const
{
    const Identifier *identifier = dynamic_cast<const Identifier *>(unary_expression_.get());
    const ArrayIndexAccess *array_index_access = dynamic_cast<const ArrayIndexAccess *>(unary_expression_.get());
    const Declarator *declarator = dynamic_cast<const Declarator *>(unary_expression_.get());
    const Dereference *dereference = dynamic_cast<const Dereference *>(unary_expression_.get());
    const AddressOf *address_of = dynamic_cast<const AddressOf *>(unary_expression_.get());
    const StructAccess *struct_access = dynamic_cast<const StructAccess *>(unary_expression_.get());
    if (identifier != nullptr)
    {
        return identifier->GetId();
    }
    else if (array_index_access != nullptr)
    {
        return array_index_access->GetId();
    }
    else if (declarator != nullptr)
    {
        return declarator->GetId();
    }
    else if (dereference != nullptr)
    {
        return dereference->GetId();
    }
    else if (address_of != nullptr)
    {
        return address_of->GetId();
    }
    else if (struct_access){
        return struct_access->GetId();
    }
    throw std::runtime_error("Assignment GetId: Not an identifier, array access, array declarator, declarator, or struct access");
}

int Assignment::GetArraySize() const
{
    const ArrayDeclaration *array_declaration = dynamic_cast<const ArrayDeclaration *>(unary_expression_.get());

    if (array_declaration != nullptr)
    {

        if (array_declaration->GetArraySize() == -1)
        {
            return dynamic_cast<const ArrayInitialization *>(expression_.get())->GetArraySize();
        }
        return array_declaration->GetArraySize();
    }

    return 1;
}

bool Assignment::isArrayInitialization() const
{
    return dynamic_cast<const ArrayDeclaration *>(unary_expression_.get()) != nullptr;
}

void Assignment::global_init(std::ostream &stream, Context &context, Global &global) const
{
    const Constant *constant = dynamic_cast<const Constant *>(expression_.get());
    const StringLiteral *string_literal = dynamic_cast<const StringLiteral *>(expression_.get());
    const ArrayInitialization *array_initialization = dynamic_cast<const ArrayInitialization *>(expression_.get());

    if (constant != nullptr)
    {
        constant->SaveValue(global);
    }

    else if (string_literal != nullptr)
    {
        string_literal->SaveValue(context, global);
    }

    else if (array_initialization != nullptr)
    {
        array_initialization->global_init(stream, context, global);
    }

    else
    {
        std::cout << "Assignment global_init: type is " << typeid(expression_).name() << std::endl;
        throw std::runtime_error("Assignment global_init: Not a constant or string literal");
    }
}

bool Assignment::isPointerInitialization() const
{
    const Declarator *declarator = dynamic_cast<const Declarator *>(unary_expression_.get());
    if (declarator){
        return declarator->isPointer();
    }
    return false;
}

void Assignment::local_init(Type type, int offset, std::ostream &stream, Context &context) const
{
    int array_size = GetArraySize();

    bool is_array = isArrayInitialization();
    bool is_pointer = isPointerInitialization();

    // Increase stack offset to account for new variable
    Type actual_type = is_pointer ? Type::_INT : type;
    int actual_type_size = types_size.at(actual_type);
    context.increase_stack_offset(actual_type_size * array_size);

    std::string variable_name = GetId();

    int dereference_num = get_deref();
    Variable variable(is_pointer, is_array, array_size, type, offset, dereference_num);
    context.define_variable(variable_name, variable);

    // Evaluate expression and store in variable
    EmitRISC(stream, context, "unused");
}

int Assignment::get_deref() const
{
    const Declarator *declarator = dynamic_cast<const Declarator *>(unary_expression_.get());
    if (declarator != nullptr)
    {
        return declarator->get_deref();
    }

    return 0;
}



}
