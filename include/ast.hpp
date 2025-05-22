#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <string>
#include <vector>

#include "arrays/ast_array_declaration.hpp"
#include "arrays/ast_array_initialization.hpp"
#include "arrays/ast_array_index_access.hpp"

#include "functions/ast_function_definition.hpp"
#include "functions/ast_parameter_definition.hpp"
#include "functions/ast_function_invocation.hpp"

#include "operations/ast_arithmetic_operation.hpp"
#include "operations/ast_relational_operation.hpp"
#include "operations/ast_equality.hpp"
#include "operations/ast_bitwise_operation.hpp"
#include "operations/ast_logical_operation.hpp"
#include "operations/ast_unary_operation.hpp"
#include "operations/ast_operand.hpp"
#include "operations/ast_sizeof.hpp"

#include "statements/ast_return.hpp"
#include "statements/ast_statements.hpp"
#include "statements/ast_declaration.hpp"
#include "statements/ast_assignment.hpp"
#include "statements/ast_expression_list.hpp"

#include "symbols/ast_identifier.hpp"
#include "symbols/ast_constant.hpp"

#include "strings/ast_character_literal.hpp"
#include "strings/ast_string_literal.hpp"

#include "control_flow/ast_switch.hpp"
#include "control_flow/ast_ifelse.hpp"
#include "control_flow/ast_while.hpp"
#include "control_flow/ast_forloop.hpp"
#include "control_flow/ast_dowhile.hpp"
#include "control_flow/ast_continue.hpp"
#include "control_flow/ast_break.hpp"

#include "specifiers/ast_specifier.hpp"
#include "specifiers/ast_type_specifier.hpp"

#include "enums/ast_enum_specifier.hpp"
#include "enums/ast_enum.hpp"

#include "structs/ast_struct_specifier.hpp"
#include "structs/ast_struct_declaration.hpp"
#include "structs/ast_struct_access.hpp"
#include "structs/ast_struct_member.hpp"

#include "pointers/ast_pointer_declaration.hpp"
#include "pointers/ast_addressof.hpp"
#include "pointers/ast_dereference.hpp"

#include "typedef/ast_typedef.hpp"

#include "ast_translation_unit.hpp"
#include "ast_direct_declarator.hpp"
#include "ast_declarator.hpp"
#include "ast_node.hpp"

#include "context/ast_context.hpp"

ast::NodePtr ParseAST(std::string file_name);

#endif
