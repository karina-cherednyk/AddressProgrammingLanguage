#include <cstdio>
#include <cassert>
#include <iostream>
#include "../headers/chunk.h"

void Chunk::write(byte val, int line) {
    lines.push_back(line);
    code.push_back(val);
}
void Chunk::write(Chunk& chunk) {
    for(int i=0; i< chunk.count(); i++) {
        lines.push_back(chunk.lines[i]);
        code.push_back(chunk.code[i]);
        if(chunk.code[i] == OP_CONSTANT)
        {
            i++;
            constants.push_back(chunk.constants[chunk.code[i]]);
            code.push_back(constants.size() - 1);
        }
    }
}

int Chunk::addConstant(Value const_val) {
    constants.push_back(const_val);
    return constants.size() - 1;
}

void Value::printValue() const{
    std::cout << std::string(*this);
}
 Value::operator std::string() const{
    switch (type) {
        case ValueType::NUMBER:
            return std::to_string(val.number);
        case ValueType::BOXED:
            return std::string(*val.pointTo);
        case ValueType::STRING:
            return val.string;
        case ValueType::BOOL:
            return  (val.boolean ? "true" : "false");
        case ValueType::POINTER:
            return ("Pointer to :\t") + std::string(*val.pointTo);
    }
     return "";
}

bool Value::operator==(const Value &other) const {
    if(type !=  other.type) return false;
    switch (type) {
        case ValueType::BOXED: return val.pointTo == other.val.pointTo;
        case ValueType::POINTER: return val.pointTo == other.val.pointTo;
        case ValueType::NUMBER: return val.number == other.val.number;
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

const char* addString(const char* start){
    return addString(start, strlen(start));
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
