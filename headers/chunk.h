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
    OP_SET_POINTER_INVERSE,
};
enum class ValueType {
    BOOL,
    NUMBER,
    POINTER,
    STRING,
    NIL
};

typedef uint8_t byte;


#include "cstring"


struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        const char* string;
        const Value* pointer;

    } as;
    inline explicit Value(double value):type(ValueType::NUMBER), as({.number = value}){};
    inline explicit Value(bool value):type(ValueType::BOOL), as({.boolean = value}){};
    inline explicit Value(const Value* value):type(ValueType::POINTER), as({.pointer = value}){};
    inline explicit Value(const char* value):type(ValueType::STRING), as({.string = value}){};
    inline Value():type(ValueType::NIL), as({.boolean = false}){};

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

    int write(byte val, int line);
    int addConstant(Value const_val);
};


#endif //CHUNK_H
