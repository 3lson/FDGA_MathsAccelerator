#pragma once

#include <unordered_map>

namespace ast{

class ContextStrings{
private:
    int label_;
    std::string value_;

public:
    ContextStrings() {}
    ContextStrings(int label, std::string value) : label_(label), value_(value) {};

    void PrintString(std::ostream &stream) const;
};

const std::unordered_map<char, char> escape_sequences_map = {
    //Can add more if have time
    {'n', '\n'},   // Newline
    {'t', '\t'},   // Horizontal tab
    {'\\', '\\'},  // Backslash
    {'\'', '\''},  // Single quote
    {'0', '\0'},   // Null character
};


}//namespace ast