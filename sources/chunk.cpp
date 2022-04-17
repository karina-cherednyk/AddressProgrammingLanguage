#include <cstdio>
#include <cassert>
#include "../headers/chunk.h"

int Chunk::write(byte val, int line) {
    lines.push_back(line);
    code.push_back(val);
    return code.size() - 1;
}


int Chunk::addConstant(Value const_val) {
    constants.push_back(const_val);
    return constants.size() - 1;
}

void Value::printValue() const{
    switch (type) {
        case ValueType::STRING:
            printf("%s", as.string); break;
        case ValueType::NUMBER:
            printf("%g", as.number); break;
        case ValueType::BOOL:
            printf(as.boolean ? "true" : "false"); break;
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

std::vector<const char*> strings;
const char* addString(const char* start, int length){
    char* newStr = new char[length + 1];
    strncpy(newStr, start, length);
    newStr[length] = '\0';
    strings.push_back(newStr);
    return newStr;
}
#define DOUBLE_MAX_LEN  30
const char* addNumString(double value){
    char* newStr = new char[DOUBLE_MAX_LEN];
    sprintf(newStr, "%g", value);
    strings.push_back(newStr);
    return newStr;
}

void freeStrings(){
    for( auto strIt = strings.begin(); strIt !=strings.end(); strIt++){
        delete[] *strIt;
    }
}
