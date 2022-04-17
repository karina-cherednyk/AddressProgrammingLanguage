#ifndef CHUNK_H
#define CHUNK_H


#include <cstdint>
#include <vector>

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
};
enum class ValueType {
    NUMBER,
    POINTER,
    STRING,
    MAP_POINTER
};

typedef uint8_t byte;


#include "cstring"


struct Value {
    ValueType type;
    union {
        double number;
        const char* string;
        Value* pointTo;

    } as;
    inline explicit Value(double value):type(ValueType::NUMBER), as({.number = value}){};
    inline explicit Value(Value* value):type(ValueType::POINTER), as({.pointTo = value}){};
    inline explicit Value(const char* value):type(ValueType::STRING), as({.string = value}){};
    inline Value(): type(ValueType::MAP_POINTER), as({.pointTo = nullptr}){};
    inline static Value MapPointer(Value* pointee){
        Value p;
        p.as.pointTo = pointee;
        return p;
    }
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
};


#endif //CHUNK_H
