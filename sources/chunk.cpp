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

void Value::printValue() const{
    switch (type) {
        case ValueType::NUMBER:
            printf("%g", as.number); break;
        case ValueType::MAP_POINTER:
            as.pointTo->printValue();
        case ValueType::POINTER:
            printf("Pointer to :\n\t"); as.pointTo->printValue();
    }
}

bool Value::operator==(const Value &other) const {
    if(type !=  other.type) return false;
    switch (type) {
        case ValueType::MAP_POINTER: return as.pointTo == other.as.pointTo;
        case ValueType::POINTER: return as.pointTo == other.as.pointTo;
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
