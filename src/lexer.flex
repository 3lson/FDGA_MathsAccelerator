%option noyywrap

%{
  // A lot of this lexer is based off the ANSI C grammar:
  // https://www.lysator.liu.se/c/ANSI-C-grammar-l.html#MUL-ASSIGN
  // Avoid error "error: `fileno' was not declared in this scope"
  extern "C" int fileno(FILE *stream);

  #include "parser.tab.hpp"
  #include <string>
  #include <cstdlib>

  //Functions
  void push_back_string(const std::string &s);
  void use_space();
  void process_typedef(TypedefSpec spec);

  // Suppress warning about unused function
  [[maybe_unused]] static void yyunput (int c, char * yy_bp );
%}

D	  [0-9]
L	  [a-zA-Z_]
H   [a-fA-F0-9]
E	  [Ee][+-]?{D}+
FSF (f|F)
FSL (l|L)
IS  (u|U|l|L)*

%%
"/*"			{/* consumes comment - TODO you might want to process and emit it in your assembly for debugging */}

"auto"			{return(AUTO);}
"break"			{return(BREAK);}
"case"			{return(CASE);}
"char"			{return(CHAR);}
"const"			{return(CONST);}
"continue"      {return(CONTINUE);}
"default"		{return(DEFAULT);}
"do"			{return(DO);}
"double"		{return(DOUBLE);}
"else"			{return(ELSE);}
"enum"			{return(ENUM);}
"extern"		{return(EXTERN);}
"float"			{return(FLOAT);}
"for"			{return(FOR);}
"goto"			{return(GOTO);}
"if"			{return(IF);}
"int"			{return(INT);}
"long"			{return(LONG);}
"register"	    {return(REGISTER);}
"return"		{return(RETURN);}
"short"			{return(SHORT);}
"signed"		{return(SIGNED);}
"sizeof"		{return(SIZEOF);}
"static"		{return(STATIC);}
"struct"		{return(STRUCT);}
"switch"		{return(SWITCH);}
"typedef"		{return(TYPEDEF);}
"union"			{return(UNION);}
"unsigned"	    {return(UNSIGNED);}
"void"			{return(VOID);}
"volatile"	    {return(VOLATILE);}
"while"			{return(WHILE);}

{L}({L}|{D})*		{
  std::string *str = new std::string(yytext);
  if (context.is_enum(*str)){
    yylval.number_int = context.get_enum_label(*str);
    delete str;
    return INT_CONSTANT;
  } else if(context.is_typedef(*str)){
    process_typedef(context.get_typedef(*str));
    delete str;
  } else {
    yylval.string = str;
    return(IDENTIFIER);
  }
}

0[xX]{H}+{IS}?		{yylval.number_int = (int)strtol(yytext, NULL, 0); return(INT_CONSTANT);}
0{D}+{IS}?		    {yylval.number_int = (int)strtol(yytext, NULL, 0); return(INT_CONSTANT);}
{D}+{IS}?		      {yylval.number_int = (int)strtol(yytext, NULL, 0); return(INT_CONSTANT);}
L?'(\\.|[^\\'])+' {yylval.number_int = yytext[1]; return(INT_CONSTANT);}

{D}+{E}{FSF}?		        {yylval.number_float = strtod(yytext, NULL); return(FLOAT_CONSTANT);}
{D}*"."{D}+({E})?{FSF}?	{yylval.number_float = strtod(yytext, NULL); return(FLOAT_CONSTANT);}
{D}+"."{D}*({E})?{FSF}?	{yylval.number_float = strtod(yytext, NULL); return(FLOAT_CONSTANT);}

{D}+{E}{FSL}		        {yylval.number_double = strtod(yytext, NULL); return(DOUBLE_CONSTANT);}
{D}*"."{D}+({E})?{FSL}	{yylval.number_double = strtod(yytext, NULL); return(DOUBLE_CONSTANT);}
{D}+"."{D}*({E})?{FSL}	{yylval.number_double = strtod(yytext, NULL); return(DOUBLE_CONSTANT);}

L?'(\\.|[^\\'])' {
    // Process the char literal
    char* characters = context.process_characters(yytext);

    yylval.character = characters[0];
    std::cout << "Character literal: " << yylval.character << std::endl;

    delete[] characters;
    return(CHAR_LITERAL);
}

L?\"(\\.|[^\\"])*\" {
    // Process the string literal
    char* characters = context.process_characters(yytext);
    std::string output = context.escape_sequences(characters);

    yylval.string = new std::string(output);
    std::cout << "String literal: " << *(yylval.string) << std::endl;

    delete[] characters;
    return(STRING_LITERAL);
}

(\*+&+)+	{
    ///Pointer implementation to handle *&*&
              char* result = context.cancel_complement(yytext);
              if (result[0] != '\0') {
                  for (int i = strlen(result) - 1; i >= 0; i--) {
                      unput(result[i]);
                  }
              }
}

"..."      {return(ELLIPSIS);}
">>="			 {return(RIGHT_ASSIGN);}
"<<="      {return(LEFT_ASSIGN);}
"+="			 {return(ADD_ASSIGN);}
"-="       {return(SUB_ASSIGN);}
"*="       {return(MUL_ASSIGN);}
"/="			 {return(DIV_ASSIGN);}
"%="			 {return(MOD_ASSIGN);}
"&="       {return(AND_ASSIGN);}
"^="			 {return(XOR_ASSIGN);}
"|="       {return(OR_ASSIGN);}
">>"       {return(RIGHT_OP);}
"<<"       {return(LEFT_OP);}
"++"			 {return(INC_OP);}
"--"			 {return(DEC_OP);}
"->"			 {return(PTR_OP);}
"&&"			 {return(AND_OP);}
"||"			 {return(OR_OP);}
"<="			 {return(LE_OP);}
">="			 {return(GE_OP);}
"=="			 {return(EQ_OP);}
"!="			 {return(NE_OP);}
";"			   {return(';');}
("{"|"<%") {return('{');}
("}"|"%>") {return('}');}
","			   {return(',');}
":"			   {return(':');}
"="			   {return('=');}
"("		     {return('(');}
")"			   {return(')');}
("["|"<:") {return('[');}
("]"|":>") {return(']');}
"."			   {return('.');}
"&"			   {return('&');}
"!"			   {return('!');}
"~"			   {return('~');}
"-"			   {return('-');}
"+"			   {return('+');}
"*"			   {return('*');}
"/"			   {return('/');}
"%"			   {return('%');}
"<"			   {return('<');}
">"			   {return('>');}
"^"			   {return('^');}
"|"			   {return('|');}
"?"			   {return('?');}

[ \a\b\t\v\f\n\r]		{/* ignore new lines and special sequences */}
.			              {/* ignore bad characters */}

%%

void yyerror (char const *s)
{
  fprintf(stderr, "Lexing error: %s\n", s);
  exit(1);
}

// Push back a string in reverse order
void push_back_string(const std::string &s) {
    use_space();
    for (int i = s.size() - 1; i >= 0; --i) {
        unput(s[i]);
    }
    unput(' ');
}

void use_space() {
    char next_char;
    while ((next_char = yyinput()) == ' ' || next_char == '\t') {
    }
    unput(next_char);
}


void process_single_parameter(TypedefSpec typedef_spec, std::vector<std::string>& parameters, std::vector<int>& pointer_counts, std::vector<char>& separators) {
    char next_char;
    std::string variable_name = "";
    bool is_initializer = false;  // Flag to track if we're inside an initializer
    int brace_count = 0;  // Track braces for initializer lists

    while (true) {
        next_char = yyinput();

        if (next_char == ',' && !is_initializer) {
            // Comma is delimiter
            break;
        } else if (next_char == ';' && brace_count == 0) {
            // Indicate the end
            break;
        } else if (next_char == '(' || next_char == ')') {
            // For arg list in functions
            break;
        } else if (next_char == '=') {
            is_initializer = true;
            variable_name += " = ";
        } else if (next_char == '{') {
            brace_count++;
            variable_name += next_char;
        } else if (next_char == '}') {
            brace_count--;
            variable_name += next_char;

            if (brace_count == 0) {
                next_char = yyinput();
                if (next_char == ';' || next_char == ',') {
                    break;
                } else {
                    variable_name += next_char;
                }
            }
        } else {
            variable_name += next_char;
        }
    }

    if (typedef_spec.get_array_size() > 1) {
        size_t equals_pos = variable_name.find('=');
        if (equals_pos != std::string::npos) {
            variable_name.insert(equals_pos, "[" + std::to_string(typedef_spec.get_array_size()) + "]");
        } else {
            variable_name += "[" + std::to_string(typedef_spec.get_array_size()) + "]";
        }
    }

    parameters.push_back(variable_name);
    pointer_counts.push_back(typedef_spec.get_number_pointers());
    separators.push_back(next_char);

    if (next_char == ',' && brace_count == 0) {
        process_single_parameter(typedef_spec, parameters, pointer_counts, separators);
    }
}

void process_typedef(TypedefSpec typedef_spec) {
    std::vector<std::string> parameters;
    std::vector<int> pointer_counts;
    std::vector<char> separators;

    process_single_parameter(typedef_spec, parameters, pointer_counts, separators);

    for (int i = parameters.size() - 1; i >= 0; --i) {
        unput(' ');
        unput(separators[i]);  // Push the separator character
        push_back_string(parameters[i]);
        unput(' ');

        // Push back any pointer symbols ('*' or '&')
        int number_pointers = pointer_counts[i];
        if (number_pointers > 0) {
            for (int j = 0; j < number_pointers; ++j) {
                unput('*');
            }
        } else if (number_pointers < 0) {
            for (int j = 0; j > number_pointers; --j) {
                unput('&');
            }
        }
        unput(' ');
    }

    // Push back the type string at the end
    std::string type;

    switch (typedef_spec.get_type()) {
        case Type::_CHAR: type = "char "; break;
        case Type::_SHORT: type = "short "; break;
        case Type::_UNSIGNED_INT: type = "unsigned int "; break;
        case Type::_INT: type = "int "; break;
        case Type::_LONG: type = "long "; break;
        case Type::_FLOAT: type = "float "; break;
        case Type::_DOUBLE: type = "double "; break;
        case Type::_STRUCT: type = "struct " + typedef_spec.get_base_class() + " "; break;
        default: throw std::runtime_error("Lexer - Typedef unrecognized type");
    }

    push_back_string(type);
}
