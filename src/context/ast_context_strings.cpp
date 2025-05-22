#include "../../include/context/ast_context.hpp"

namespace ast{

void ContextStrings::PrintString(std::ostream &stream) const
{
    stream << "string_" << label_ << ":" << std::endl;
    stream << "\t.string \"" << value_ << "\"" << std::endl;
}

char *Context::process_characters(const char *sequence)
{
    // Determine the length of the sequence, excluding the enclosing single quotes
    size_t length = strlen(sequence);

    // Allocate memory for the result (excluding the quotes and null terminator)
    char *result = new char[length - 1];

    size_t result_index = 0;

    for (size_t i = 1; i < length - 1; ++i)
    {
        if (sequence[i] == '\\') // Escape sequences
        {
            auto it = escape_sequences_map.find(sequence[i + 1]);
            if (it != escape_sequences_map.end())
            {
                result[result_index++] = it->second;
                ++i;
            }
            else
            {
                delete[] result; // Clean up memory if an error occurs
                throw std::runtime_error("Context::process_char - Invalid escape sequence");
            }
        }
        else // Regular characters
        {
            result[result_index++] = sequence[i];
        }
    }

    // Null-terminate the result
    result[result_index] = '\0';

    return result;
}

std::string Context::escape_sequences(const char *processed_characters)
{
    std::string result;

    for (size_t i = 0; processed_characters[i] != '\0'; ++i)
    {
        bool found = false;

        // Iterate through the map to find the matching escape sequence
        for (const auto &pair : escape_sequences_map)
        {
            if (processed_characters[i] == pair.second)
            {
                result.push_back('\\');
                result.push_back(pair.first);
                found = true;
                break;
            }
        }

        // If the character wasn't found in the map, add it as-is
        if (!found)
        {
            result.push_back(processed_characters[i]);
        }
    }

    return result;
}

int Context::define_string(std::string string)
{
    strings.push_back(ContextStrings(string_counter, string));
    return string_counter++;
}

void Context::print_string(std::ostream &stream) const
{
    for (const auto &declaration : strings)
    {
        declaration.PrintString(stream);
    }
}

}