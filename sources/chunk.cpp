#include <cstdio>
#include <cassert>
#include "../headers/chunk.h"

void Chunk::write(byte val, int line) {
    lines.push_back(line);
    code.push_back(val);
}


int Chunk::addConstant(Value const_val) {
    constants.push_back(const_val);
    return constants.size() - 1;
}

void Value::printValue(){
    switch (type) {
        case ValueType::NUMBER:
            printf("%g", as.number); break;
        case ValueType::BOOL:
            printf(as.boolean ? "true" : "false"); break;
        case ValueType::POINTER:
            assert(false);
    }
}

bool Value::operator==(const Value &other) const {
    if(type !=  other.type) return false;
    switch (type) {
        case ValueType::BOOL: return as.boolean == other.as.boolean;
        case ValueType::POINTER: return as.pointer == other.as.pointer;
        case ValueType::NUMBER: return as.number == other.as.number;
    }
    return false;
}
