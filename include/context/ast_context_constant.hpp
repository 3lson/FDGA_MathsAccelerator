#pragma once

#include "ast_context_types.hpp"
#include <iostream>
#include <cstdint>
#include <cstring>

//Implements the upper and lower bit handling for float and double data types

namespace ast{

class ContextConstant {
private:
    Type type_;
    uint32_t low_bits;
    uint32_t high_bits;

    static uint32_t FloatToBits(float value){
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        return bits;
    }

    static void DoubleToBits(double value, uint32_t &low, uint32_t &high){
        uint64_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        low = static_cast<uint32_t>(bits & 0xFFFFFFFF);
        high = static_cast<uint32_t>((bits >> 32) & 0xFFFFFFFF);
    }

public:
    explicit ContextConstant(float value)
        : type_(Type::_FLOAT), low_bits(FloatToBits(value)), high_bits(0) {}

    explicit ContextConstant(double value)
        : type_(Type::_DOUBLE) {
        DoubleToBits(value, low_bits, high_bits);
    }
    //Add getters
    uint32_t get_low_bits() const { return low_bits; }
    uint32_t get_high_bits() const { return high_bits; }

    void PrintConst(std::ostream &stream) const;

    Type GetType() const;
};
}//namespace ast
