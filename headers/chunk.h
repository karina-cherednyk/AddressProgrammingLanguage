#ifndef CHUNK_H
#define CHUNK_H


#include <cstdint>
#include <vector>
#include <string>
#include <map>

template <typename T>
bool has(const std::map<std::string ,T>& map, std::string val){
    return map.find(val) != map.end();
}


enum OpCode {
    OP_RETURN,
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_LESS,
    OP_EQUAL,
    OP_GREATER,
    OP_TRUE,
    OP_FALSE,
    OP_PRINT,
    OP_POP,
    OP_SET_POINTER,
    OP_GET_POINTER,
    OP_SET_POINTER_INVERSE,
    OP_GOTO
};
enum class ValueType {
    NUMBER,
    POINTER,
    STRING,
    BOXED
};

typedef uint8_t byte;


#include "cstring"


struct Value {
    ValueType type;
    union {
        double number;
        const char* string;
        Value* pointTo;

    } val;
    inline explicit Value(double value): type(ValueType::NUMBER), val({.number = value}){};
    inline explicit Value(Value* value): type(ValueType::POINTER), val({.pointTo = value}){};
    inline explicit Value(const char* value): type(ValueType::STRING), val({.string = value}){};
    inline static Value Boxed(Value* pointee){
        Value p(pointee);
        p.type = ValueType::BOXED;
        return p;
    }
    inline explicit Value():type{ValueType::POINTER}, val({.pointTo = nullptr}) {};
    bool operator== (const Value& other) const;
    void printValue() const;
};

//typedef double Value;


const char* addString(const char* start, int length);
const char* addNumString(double value);
void freeStrings();

struct Chunk {
    std::vector<byte> code;
    std::vector<int> lines;
    std::vector<Value> constants;

    void write(byte val, int line);
    int addConstant(Value const_val);
    inline size_t count(){ return  code.size(); }

};


#endif //CHUNK_H
